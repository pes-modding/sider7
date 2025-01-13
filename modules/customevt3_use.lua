local m = {}

local ACTION_KEY = { 0x35, "[5]" }
local forcefully_end
local old_al

function m.key_up(ctx, vkey)
    if vkey == ACTION_KEY[1] then
        forcefully_end = true
        log(string.format("triggering end of half: forcefully_end = true"))
    end
end

function m.check_eoh(ctx, event_id, registers)
    local al = registers.rax:sub(1,1)
    if old_al ~= al then
        -- do not spam the log. print only when AL changes
        log(string.format("custom:check_eoh: (event_id:%s, r8b:%s, al:%s)", event_id, memory.hex(registers.r8:sub(1,1)), memory.hex(al)))
    end
    old_al = al

    if forcefully_end then
        local xmm3 = memory.unpack("f", registers.xmm3:sub(1,4))
        if xmm3 <= 100 then
            -- new period started
            log(string.format("custom:check_eoh: new time period started (xmm3: %s): forcefully_end = nil", xmm3))
            forcefully_end = nil
        else
            -- send AL = 1 to end the half
            log(string.format("custon:check_eoh: returning rax:%s (xmm3: %s)", memory.hex(memory.pack("u64",1)), xmm3))
            return { rax = memory.pack("u64", 1) }
        end
    end
end

function m.overlay_on(ctx)
    return string.format("%s - to request the end of current period of play", ACTION_KEY[2]) .. (forcefully_end and " | REQUESTED" or "")
end

function m.init(ctx)
    -- check if custom event trigger is supported
    if not ctx.custom_evt_rbx then
        error("custom events are not supported. Upgrade your sider")
    end
    -- we need match.stats too
    if not match.stats then
        error("this module requires match.stats to be enabled")
    end

    -- register for specific custom event
    ctx.register("custom:check_end_of_half", m.check_eoh)

    -- register for other helpful sider events
    ctx.register("overlay_on", m.overlay_on)
    ctx.register("key_up", m.key_up)
end

return m
