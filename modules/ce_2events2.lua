local m = {}

local function log_registers(registers)
    log("---------------------------------------")
    local keys = {}
    for k,_ in pairs(registers) do
        keys[#keys + 1] = k
    end
    table.sort(keys)
    for _,k in ipairs(keys) do
        local v = registers[k]
        if k:sub(1,1) == "r" then
            v = memory.unpack("u64", v)
            log(string.format("register: %s = %s (HEX), %s (DEC)", k, memory.hex(v), v))
        else
            log(string.format("register: %s = %s (HEX str)", k, memory.hex(v)))
        end
    end
    log("---------------------------------------")
end

local ACTION_KEY = { 0x35, "[5]" }
local forcefully_end
function m.key_up(ctx, vkey)
    if vkey == ACTION_KEY[1] then
        forcefully_end = true
        log(string.format("triggering end of half: forcefully_end = true"))
    end
end

local old_al
function m.check_eoh(ctx, event_id, registers)
    local al = registers.rax:sub(1,1)
    if old_al ~= al then
        log(string.format("custom:check_eoh: (event_id:%s, r8b:%s, al:%s)", event_id, memory.hex(registers.r8:sub(1,1)), memory.hex(al)))
    end
    old_al = al

    --[[
    log(string.format("xmm0 = %s, xmm2 = %s, xmm3 = %s",
        memory.unpack("f", registers.xmm0:sub(1,4)),
        memory.unpack("f", registers.xmm2:sub(1,4)),
        memory.unpack("f", registers.xmm3:sub(1,4))
    ))
    --]]

    if forcefully_end then
        -- check if new half started
        local xmm3 = memory.unpack("f", registers.xmm3:sub(1,4))
        if xmm3 <= 100 then
            -- new half started: we need to reset our flag
            forcefully_end = nil
            log(string.format("custom:check_eoh: new half started: xmm3 = %s", xmm3))
            log(string.format("custom:check_eof: forcefully_end = nil"))
        else
            -- send AL = 1 to end the half
            log(string.format("custon:check_eoh: returning rax:%s", memory.hex(memory.pack("u64",1))))
            return { rax = memory.pack("u64", 1) }
        end
    end
end

function m.goal_scored(ctx, event_id, registers)
    log(string.format("custom:goal_scored: (event_id:%s, registers:%s)", event_id, registers))
    log_registers(registers)
end

function m.overlay_on(ctx)
    return string.format("%s - to end current period of play", ACTION_KEY[2])
end

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

local function setup_goal_scored(ctx, event_name)
    --[[
    0000000140478C3F | E8 7CA75E00                     | call pes2021.140A633C0                  | new goal scored / get player id
    0000000140478C44 | 8BD8                            | mov ebx,eax                             |
    0000000140478C46 | E8 65456000                     | call pes2021.140A7D1B0                  |
    0000000140478C4B | 48:8B90 38040000                | mov rdx,qword ptr ds:[rax+438]          |
    0000000140478C52 | 4C:8B42 08                      | mov r8,qword ptr ds:[rdx+8]             |
    0000000140478C56 | 48:8BCA                         | mov rcx,rdx                             |
    0000000140478C59 | 45:3870 19                      | cmp byte ptr ds:[r8+19],r14b            |
    0000000140478C5D | 75 19                           | jne pes2021.140478C78                   |
    --]]

    local addr = memory.search_process("\x48\x8b\x90\x38\x04\x00\x00\x4c\x8b\x42\x08\x48\x8b\xca\x45\x38\x70\x19\x75\x19")
    if addr == nil then
        error("code location not found")
    end
    log(string.format("code location: " .. memory.hex(addr)))

    -- get unique id for the custom event name
    local event_id = ctx.get_event_id(event_name)
    log(string.format("event_id: %d", event_id))

    -- put event trigger into codecave
    local codecave = memory.allocate_codecave(64)
    log("codecave allocated at: " .. memory.hex(codecave))
    memory.write(codecave,
        "\x53" ..                                                -- push rbx
        "\x51" ..                                                -- push rcx
        "\x48\xbb" .. memory.pack("u64", ctx.custom_evt_rbx) ..  -- mov rbx, <sider_custom_event_rbx_hk>
        "\x66\xb9" .. memory.pack("u16", event_id) ..            -- mov cx, <event_id>
        "\xff\xd3" ..                                            -- call rbx
        "\x48\x83\xc4\x10" ..                                    -- add rsp,10h
        "\xc3"                                                   -- ret
    )

    -- call the event trigger
    local offset = memory.unpack("i32", memory.read(addr - 4, 4))
    local target_addr = addr + offset

    --[[
    0000000140A7D1B0 | 48:83EC 38                      | sub rsp,38                              |
    0000000140A7D1B4 | 48:C74424 20 FEFFFFFF           | mov qword ptr ss:[rsp+20],FFFFFFFFFFFFF |
    0000000140A7D1BD | 48:8B05 3462C802                | mov rax,qword ptr ds:[1437033F8]        |
    0000000140A7D1C4 | 48:85C0                         | test rax,rax                            |
    0000000140A7D1C7 | 75 34                           | jne pes2021.140A7D1FD                   |
    0000000140A7D1C9 | C74424 40 19000000              | mov dword ptr ss:[rsp+40],19            |
    0000000140A7D1D1 | 45:33C0                         | xor r8d,r8d                             |
    0000000140A7D1D4 | BA 60040000                     | mov edx,460                             |
    0000000140A7D1D9 | 48:8D4C24 40                    | lea rcx,qword ptr ss:[rsp+40]           |
    0000000140A7D1DE | E8 7DE297FF                     | call pes2021.1403FB460                  |
    0000000140A7D1E3 | 48:894424 48                    | mov qword ptr ss:[rsp+48],rax           |
    0000000140A7D1E8 | 48:85C0                         | test rax,rax                            |
    0000000140A7D1EB | 74 09                           | je pes2021.140A7D1F6                    |
    0000000140A7D1ED | 48:8BC8                         | mov rcx,rax                             |
    0000000140A7D1F0 | E8 FBF9FFFF                     | call pes2021.140A7CBF0                  |
    0000000140A7D1F5 | 90                              | nop                                     |
    0000000140A7D1F6 | 48:8905 FB61C802                | mov qword ptr ds:[1437033F8],rax        |
    0000000140A7D1FD | 48:83C4 38                      | add rsp,38                              |
    0000000140A7D201 | C3                              | ret                                     |
    0000000140A7D202 | CC                              | int3                                    |
    0000000140A7D203 | CC                              | int3                                    |
    0000000140A7D204 | CC                              | int3                                    |
    0000000140A7D205 | CC                              | int3                                    |
    0000000140A7D206 | CC                              | int3                                    |
    0000000140A7D207 | CC                              | int3                                    |
    0000000140A7D208 | CC                              | int3                                    |
    0000000140A7D209 | CC                              | int3                                    |
    0000000140A7D20A | CC                              | int3                                    |
    0000000140A7D20B | CC                              | int3                                    |
    0000000140A7D20C | CC                              | int3                                    |
    0000000140A7D20D | CC                              | int3                                    |
    0000000140A7D20E | CC                              | int3                                    |
    0000000140A7D20F | CC                              | int3                                    |
    --]]
    memory.write(target_addr + 0x202 - 0x1b0,
        "\x48\xb8" .. memory.pack("u64", codecave) ..    -- mov rax, codecave
        "\xff\xd0" ..                                    -- call rax
        "\xeb\xa0")                                      -- jmp 0140A7D1B0

    -- adjust initial caller to hit our entry point at 0x140A7D202
    offset = offset + 0x202 - 0x1b0
    memory.write(addr - 4, memory.pack("i32", offset))
end

function m.init(ctx)
    -- check if custom event trigger is supported
    if not ctx.custom_evt_rbx then
        error("custom events are not supported. Upgrade your sider")
    end

    -- create custom events
    setup_end_of_half(ctx, "custom:check_end_of_half")
    setup_goal_scored(ctx, "custom:goal_scored")

    -- register for custom events
    ctx.register("custom:check_end_of_half", m.check_eoh)
    ctx.register("custom:goal_scored", m.goal_scored)

    ctx.register("overlay_on", m.overlay_on)
    ctx.register("key_up", m.key_up)
end

return m
