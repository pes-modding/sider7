--[[
=========================

camera module
Game research by: nesa24
Requires: sider.dll 6.0.1+

=========================
--]]

local m = {}
m.version = "3.1"
local hex = memory.hex
local settings

local RESTORE_KEY = 0x38
local PREV_PROP_KEY = 0x39
local NEXT_PROP_KEY = 0x30
local PREV_VALUE_KEY = 0xbd
local NEXT_VALUE_KEY = 0xbb

local delta = 0
local frame_count = 0

local overlay_curr = 1
local overlay_states = {
    { ui = "dynamic wide camera angle: %0.2f", prop = "dynwide_camera_angle", decr = -0.1, incr = 0.1 },
    { ui = "fanview camera zoom: %0.2f", prop = "fanview_camera_zoom", decr = -0.1, incr = 0.1 },
    { ui = "fanview camera height: %0.2f", prop = "fanview_camera_height", decr = -0.01, incr = 0.01 },
    { ui = "fanview camera angle: %0.2f", prop = "fanview_camera_angle", decr = -0.1, incr = 0.1 },
    { ui = "replays: %s", prop = "replays",
        nextf = function(v)
            return (v == "on") and "off" or "on"
        end,
        prevf = function(v)
            return (v == "on") and "off" or "on"
        end,
    },
}
local ui_lines = {}

local bases = {
    dynwide_angle = nil,
    camera = nil,
    replays = nil,
}
local game_info = {
    dynwide_camera_angle  = { base = "dynwide_angle", offs = 0, format = "f", len = 4, def = 0.2 },
    fanview_camera_zoom   = { base = "camera", offs = 0x08, format = "f", len = 4, def = 25.60 },
    fanview_camera_height = { base = "camera", offs = 0x0c, format = "f", len = 4, def = 0.43 },
    fanview_camera_angle  = { base = "camera", offs = 0x24, format = "f", len = 4, def = 1 },
    replays = { base = "replays", offs = 0x04, format = "", len = 1, def = 'on', value_map = {on='\x04', off='\x07'} },
}

local function load_ini(ctx, filename)
    local t = {}
    -- initialize with defaults
    for prop,info in pairs(game_info) do
        t[prop] = info.def
    end
    -- now try to read ini-file, if present
    local f,err = io.open(ctx.sider_dir .. "\\" .. filename)
    if not f then
        return t
    end
    f:close()
    for line in io.lines(ctx.sider_dir .. "\\" .. filename) do
        local name, value = string.match(line, "^([%w_]+)%s*=%s*([-%w%d.]+)")
        if name and value then
            value = tonumber(value) or value
            t[name] = value
            log(string.format("Using setting: %s = %s", name, value))
        end
    end
    return t
end

local function save_ini(ctx, filename)
    local f,err = io.open(ctx.sider_dir .. "\\" .. filename, "wt")
    if not f then
        log(string.format("PROBLEM saving settings: %s", tostring(err)))
        return
    end
    f:write(string.format("# Camera settings. Written by camera.lua " .. m.version .. "\n\n"))
    f:write(string.format("dynwide_camera_angle = %0.2f\n", settings.dynwide_camera_angle or game_info.dynwide_camera_angle.def))
    f:write(string.format("fanview_camera_zoom = %0.2f\n", settings.fanview_camera_zoom or game_info.fanview_camera_zoom.def))
    f:write(string.format("fanview_camera_height = %0.2f\n", settings.fanview_camera_height or game_info.fanview_camera_height.def))
    f:write(string.format("fanview_camera_angle = %0.2f\n", settings.fanview_camera_angle or game_info.fanview_camera_angle.def))
    f:write(string.format("replays = %s\n", settings.replays or game_info.replays.def))
    f:write("\n")
    f:close()
end

local function apply_settings(ctx, log_it, save_it)
    for name,value in pairs(settings) do
        local entry = game_info[name]
        if entry then
            local base = bases[entry.base]
            if base then
                if entry.value_map then
                    value = entry.value_map[value]
                end
                local addr = base + entry.offs
                local old_value, new_value
                if entry.format ~= "" then
                    old_value = memory.unpack(entry.format, memory.read(addr, entry.len))
                    memory.write(addr, memory.pack(entry.format, value))
                    new_value = memory.unpack(entry.format, memory.read(addr, entry.len))
                    if log_it then
                        log(string.format("%s: changed at %s: %s --> %s",
                            name, hex(addr), old_value, new_value))
                    end
                else
                    old_value = memory.read(addr, entry.len)
                    memory.write(addr, value)
                    new_value = memory.read(addr, entry.len)
                    if log_it then
                        log(string.format("%s: changed at %s: %s --> %s",
                            name, hex(addr), hex(old_value), hex(new_value)))
                    end
                end
            end
        end
    end
    if (save_it) then
        save_ini(ctx, "camera.ini")
    end
end

function m.set_teams(ctx, home, away)
    apply_settings(ctx, true)
end

local function repeat_change(ctx, after_num_frames, change)
    if change ~= 0 then
        frame_count = frame_count + 1
        if frame_count >= after_num_frames then
            local s = overlay_states[overlay_curr]
            settings[s.prop] = settings[s.prop] + change
            apply_settings(ctx, false, false) -- apply
        end
    end
end

function m.overlay_on(ctx)
    -- repeat change from gamepad, if delta exists
    repeat_change(ctx, 30, delta)
    -- construct ui text
    for i,v in ipairs(overlay_states) do
        local s = overlay_states[i]
        local setting = string.format(s.ui, settings[s.prop])
        if i == overlay_curr then
            ui_lines[i] = string.format("\n---> %s <---", setting)
        else
            ui_lines[i] = string.format("\n     %s", setting)
        end
    end
    return string.format([[version %s
Keys: [9][0] - choose setting, [-][+] - modify value, [8] - restore defaults
Gamepad: RS up/down - choose setting, RS left/right - modify value
%s]], m.version, table.concat(ui_lines))
end

function m.key_down(ctx, vkey)
    if vkey == NEXT_PROP_KEY then
        if overlay_curr < #overlay_states then
            overlay_curr = overlay_curr + 1
        end
    elseif vkey == PREV_PROP_KEY then
        if overlay_curr > 1 then
            overlay_curr = overlay_curr - 1
        end
    elseif vkey == NEXT_VALUE_KEY then
        local s = overlay_states[overlay_curr]
        if s.incr ~= nil then
            settings[s.prop] = settings[s.prop] + s.incr
        elseif s.nextf ~= nil then
            settings[s.prop] = s.nextf(settings[s.prop])
        end
        apply_settings(ctx, false, true)
    elseif vkey == PREV_VALUE_KEY then
        local s = overlay_states[overlay_curr]
        if s.decr ~= nil then
            settings[s.prop] = settings[s.prop] + s.decr
        elseif s.prevf ~= nil then
            settings[s.prop] = s.prevf(settings[s.prop])
        end
        apply_settings(ctx, false, true)
    elseif vkey == RESTORE_KEY then
        for i,s in ipairs(overlay_states) do
            settings[s.prop] = game_info[s.prop].def
        end
        apply_settings(ctx, false, true)
    end
end

function m.gamepad_input(ctx, inputs)
    local v = inputs["RSy"]
    if v then
        if v == -1 and overlay_curr < #overlay_states then -- moving down
            overlay_curr = overlay_curr + 1
        elseif v == 1 and overlay_curr > 1 then -- moving up
            overlay_curr = overlay_curr - 1
        end
    end

    v = inputs["RSx"]
    if v then
        if v == -1 then -- moving left
            local s = overlay_states[overlay_curr]
            if s.decr ~= nil then
                settings[s.prop] = settings[s.prop] + s.decr
                -- set up the repeat change
                delta = s.decr
                frame_count = 0
            elseif s.prevf ~= nil then
                settings[s.prop] = s.prevf(settings[s.prop])
            end
            apply_settings(ctx, false, false) -- apply
        elseif v == 1 then -- moving right
            local s = overlay_states[overlay_curr]
            if s.decr ~= nil then
                settings[s.prop] = settings[s.prop] + s.incr
                -- set up the repeat change
                delta = s.incr
                frame_count = 0
            elseif s.nextf ~= nil then
                settings[s.prop] = s.nextf(settings[s.prop])
            end
            apply_settings(ctx, false, false) -- apply
        elseif v == 0 then -- stop change
            delta = 0
            apply_settings(ctx, false, true) -- apply and save
        end
    end
end

local function find_pattern(ctx, pattern, cache_id)
    if cache_id then
        -- check the cached location first: it might match
        local f = io.open(ctx.sider_dir .. "\\camera.cache", "rb")
        if f then
            local cache_data = f:read("*all")
            local hint = string.sub(cache_data, cache_id*8+1, cache_id*8+8)
            if hint and hint ~= "" then
                local addr = memory.unpack("i64", hint)
                if addr and addr ~= 0 then
                    local data = memory.read(addr, #pattern)
                    if pattern == data then
                        log(string.format("matched cache hint #%s", cache_id))
                        return addr
                    end
                end
            end
        end
    end
    -- no cache, or no match: search the process
    return memory.search_process(pattern)
end

local function write_cache(ctx, cache, cache_size)
    local f = io.open(ctx.sider_dir .. "\\camera.cache", "wb")
    for i=0,cache_size-1 do
        addr = cache[i] or 0
        f:write(memory.pack("i64", addr))
    end
    f:close()
end

function m.init(ctx)
    local cache = {}

    settings = load_ini(ctx, "camera.ini")

    -- find the base address of the block of camera settings
    --[[
0000000147E08772 | 0F 11 44 24 20                     | movups xmmword ptr ss:[rsp+20],xmm0    |
0000000147E08777 | 89 45 87                           | mov dword ptr ss:[rbp-79],eax          |
0000000147E0877A | 0F 11 4C 24 30                     | movups xmmword ptr ss:[rsp+30],xmm1    |
    --]]
    local loc = find_pattern(ctx, "\x0f\x11\x44\x24\x20\x89\x45\x87\x0f\x11\x4c\x24\x30", 0)
    if loc then
        cache[0] = loc
        local rel_offset = memory.unpack("i32", memory.read(loc - 4, 4))
        bases.camera = loc + rel_offset
        log(string.format("Fanview camera block base address: %s", hex(bases.camera)))
    else
        error("problem: unable to find code pattern for camera block")
    end
    --]]

    -- find replays opcode
    local loc = find_pattern(ctx, "\x41\xc6\x45\x08\x04", 1)
    if loc then
        cache[1] = loc
        bases.replays = loc
        log(string.format("Replays switch address: %s", hex(bases.replays)))
    else
        error("problem: unable to find code pattern for replays switch")
    end

    -- find angle reading instruction for Dynamic Wide camera.
    -- this code pattern immediately follows the instruction we need:
    --[[
000000014AD14892 | F3 41 0F 59 FA                     | mulss xmm7,xmm10                       |
000000014AD14897 | F3 45 0F 5E C2                     | divss xmm8,xmm10                       |
000000014AD1489C | F3 41 0F 5E F2                     | divss xmm6,xmm10                       |
000000014AD148A1 | F3 41 0F 5C C0                     | subss xmm0,xmm8                        |
    --]]
    local loc = find_pattern(ctx, "\xf3\x41\x0f\x59\xfa\xf3\x45\x0f\x5e\xc2\xf3\x41\x0f\x5e\xf2\xf3\x41\x0f\x5c\xc0", 2)
    if loc then
        cache[2] = loc
        local rel_offset = memory.unpack("i32", memory.read(loc - 4, 4))
        log(string.format("Dynamic Wide angle read at: %s", hex(loc - 9)))
        log(string.format("Dynamic Wide org angle address: %s", hex(loc + rel_offset)))
        -- codecave concept: modify instruction, change relative offset.
        -- We need to make the game to read a value from a different location,
        -- so that we can safely modify the value.
        rel_offset = rel_offset + 0x2c -- there's an unused memory slot there
        memory.write(loc - 4, memory.pack("i32", rel_offset))
        bases.dynwide_angle = loc + rel_offset
        log(string.format("Dynamic Wide new angle address: %s", hex(bases.dynwide_angle)))
    else
        error("problem: unable to find code pattern for dynamic wide camera angle read")
    end

    -- save found locations to disk
    write_cache(ctx, cache, 3)

    -- register for events
    ctx.register("set_teams", m.set_teams)
    ctx.register("overlay_on", m.overlay_on)
    ctx.register("key_down", m.key_down)
    ctx.register("gamepad_input", m.gamepad_input)
end

return m
