local m = {}

local function setup_end_of_half(ctx, event_name)
    -- find the place where want to hook
    --[[
    0000000140A6C54A | 45:84C0                         | test r8b,r8b                            |
    0000000140A6C54D | 75 13                           | jne pes2021.140A6C562                   |
    0000000140A6C54F | F3:0F1005 B147BE01              | movss xmm0,dword ptr ds:[142650D08]     |
    0000000140A6C557 | F3:0F58C2                       | addss xmm0,xmm2                         |
    0000000140A6C55B | 0F2FD8                          | comiss xmm3,xmm0                        |
    0000000140A6C55E | 0F93C0                          | setae al                                |
    0000000140A6C561 | C3                              | ret                                     |
    0000000140A6C562 | F3:0F1005 9A47BE01              | movss xmm0,dword ptr ds:[142650D04]     |
    0000000140A6C56A | F3:0F58C2                       | addss xmm0,xmm2                         |
    0000000140A6C56E | 0F2FD8                          | comiss xmm3,xmm0                        |
    0000000140A6C571 | 0F93C0                          | setae al                                |
    0000000140A6C574 | C3                              | ret                                     |
    --]]
    local addr = memory.search_process("\xf3\x0f\x58\xc2\x0f\x2f\xd8\x0f\x93\xc0\xc3\xf3\x0f\x10\x05")
    if addr == nil then
        error("code location not found")
    end
    addr = addr - 0x57 + 0x4a
    log("code location: " .. memory.hex(addr))

    -- determine addrs that hold the floats that we will copy later
    local addr_1 = addr + 0x57 - 0x4a + memory.unpack("i32", memory.read(addr + 0x57 - 0x4a - 4, 4))
    local addr_2 = addr + 0x6a - 0x4a + memory.unpack("i32", memory.read(addr + 0x6a - 0x4a - 4, 4))
    log("addr_1: " .. memory.hex(addr_1))
    log("addr_2: " .. memory.hex(addr_2))

    -- get unique id for the custom event name
    local event_id = ctx.get_event_id(event_name)
    log(string.format("event_id: %d", event_id))

    -- put event trigger into codecave
    local codecave = memory.allocate_codecave(128)
    log("codecave allocated at: " .. memory.hex(codecave))

    -- we will place the values in the codecave too: right after code
    local new_offset_1 = 0x3b
    local new_offset_2 = 0x23

    memory.write(codecave,
        "\x45\x84\xc0" ..                                          -- test r8b,r8b
        "\x75\x14" ..                                              -- jne extra_time
        "\xf3\x0f\x10\x05" .. memory.pack("i32", new_offset_1) ..  -- movss xmm0,dword ptr [addr_1]
        "\xf3\x0f\x58\xc2" ..                                      -- adss xmm0,xmm2
        "\x0f\x2f\xd8" ..                                          -- comiss xmm3,xmm0
        "\x0f\x93\xc0" ..                                          -- setae al
        "\xeb\x12"     ..                                          -- jmp trigger
                                                        -- extra_time:
        "\xf3\x0f\x10\x05" .. memory.pack("i32", new_offset_2) ..  -- movss xmm0,dword ptr [addr_2]
        "\xf3\x0f\x58\xc2" ..                                      -- adss xmm0,xmm2
        "\x0f\x2f\xd8" ..                                          -- comiss xmm3,xmm0
        "\x0f\x93\xc0" ..                                          -- setae al
                                                         -- trigger:
        "\x53" ..                                                -- push rbx
        "\x51" ..                                                -- push rcx
        "\x48\xbb" .. memory.pack("u64", ctx.custom_evt_rbx) ..  -- mov rbx,<sider_custom_event_rbx_hk>
        "\x66\xb9" .. memory.pack("u16", event_id) ..            -- mov cx,<event_id>
        "\xff\xd3" ..                                            -- call rbx
        "\x48\x83\xc4\x10" ..                                    -- add rsp,10h
        "\xc3"                                                   -- ret
    )
    -- copy 2 floats to codecave, so that we can access them easier
    -- (without far-addressing)
    memory.write(codecave + 0x48, memory.read(addr_1, 4))
    memory.write(codecave + 0x44, memory.read(addr_2, 4))

    -- connect event trigger with place in the game code
    -- 1. put an indirect jump instruction using that addr that just folllows
    -- 2. put the 8-byte codecave addr right after
    memory.write(addr, "\xff\x25\x00\x00\x00\x00")
    memory.write(addr + 6, memory.pack("u64", codecave))
end

function m.init(ctx)
    -- check if custom event trigger is supported
    if not ctx.custom_evt_rbx then
        error("custom events are not supported. Upgrade your sider")
    end

    -- install custom event trigger code
    setup_end_of_half(ctx, "custom:check_end_of_half")
end

return m
