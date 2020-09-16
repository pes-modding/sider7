--[[
=========================

gameplay.lua version 1.0
Game research by: nesa24
Requires: sider.dll 5.0.1

=========================
--]]

local m = {}
local hex = memory.hex

local game_info = {
    gameplay_speed = { 0x1b93c08, "f", 4 },       --> default: 0.200
}

local patches = {
    disable_replays = { 
        search_for   = "\x41\xc6\x45\x08\x04",
        replace_with = "\x41\xc6\x45\x08\x06",
    }
}

local function load_ini(ctx, filename)
    local t = {}
    for line in io.lines(ctx.sider_dir .. "\\" .. filename) do
        local name, value = string.match(line, "^([%w_]+)%s*=%s*([-%d.]+)")
        if name and value then
            t[name] = tonumber(value)
            log(string.format("Using setting: %s = %s", name, value))
        end
    end
    return t
end

function m.init(ctx)
    local pi = memory.get_process_info()
    log(string.format("process base: %s", hex(pi.base)))

    local settings = load_ini(ctx, "gameplay.ini")

    -- apply settings
    for name,value in pairs(settings) do
        local entry = game_info[name]
        if entry then
            local rva, format, len = unpack(entry)
            local addr = pi.base + rva
            local old_value = memory.unpack(format, memory.read(addr, len))
            memory.write(addr, memory.pack(format, value))
            local new_value = memory.unpack(format, memory.read(addr, len))
            log(string.format("%s: changed at %s: %s --> %s",
                name, hex(addr), old_value, new_value))
        end
    end

    -- install patches
    for name,value in pairs(settings) do
        local entry = patches[name]
        if entry and tonumber(value)==1 then
            local addr, section = memory.search_process(entry.search_for)
            if addr then
                memory.write(addr, entry.replace_with)
                log(string.format("patch \"%s\" installed at %s (section: %s [%s - %s])",
                    name, hex(addr), section.name, hex(section.start), hex(section.finish)))
            end
        end
    end
end

return m
