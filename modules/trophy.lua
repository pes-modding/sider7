--[[
==================================
trophy.lua - version 1.3

programming by: juce
using save data files by: saintric
==================================

Trophy server module is used to add trophy celebrations
to tournaments that do not have them in the game.

This works in 2 steps:

#1. We force the game to use scenes from tournaments
that do have trophy presentations. So without even adding
any content, you can have some existing trophy be shown
during English Super Cup - in this module, we use
American Cup.

#2. Then we can replace the content of the cup files with
custom made trophy. Those files are put into "content/trophy-server"
folder insider sider. The trophy files should still have the old names
but the trophy inside can be changed, of course. This way
it becomes possible to have proper trophies for all leagues, cups
and supercups.

All game modes are supported: League, Cup, Master League.

--]]

local m = { version = "1.3" }

local content_root = ".\\content\\trophy-server"
local tcontent = nil
local remap = {}
local info = ""

function load_map(filename)
    local map = {}
    for line in io.lines(filename) do
        -- strip comment
        line = string.gsub(line, "#.*", "")
        tid, tid2, path = string.match(line, "%s*(%d+)%s*,%s*(%d+)%s*,%s*[\"]?([^\"]*)")
        tid = tonumber(tid)
        tid2 = tonumber(tid2)
        if tid and tid2 and path then
            map[tid] = { tid2, path }
            log(string.format("tid: %d ==> tid: %d, content path: %s", tid, tid2, path))
        end
    end
    return map
end

function m.trophy_rewrite(ctx, tournament_id)
    tcontent = nil
    local entry = remap[tournament_id]
    if entry then
        local tid, relpath = unpack(entry)
        if tid and relpath then
            tcontent = content_root .. "\\" .. relpath .. "\\"
            log(string.format("This tournament is: %d. Remapping trophy scenes to: %d", tournament_id, tid))
            log(string.format("Using content from: %s", tcontent))
            return tid
        end
    end
end

function m.make_key(ctx, filename)
    --log(string.format("wants: %s", filename))
    if tcontent then
        return tcontent .. filename
    end
end

function m.get_filepath(ctx, filename, key)
    if tcontent then
        return key
    end
end

function m.overlay_on(ctx)
    return string.format("version %s\nControls: [0] - to reload map\n%s", m.version, info)
end

function m.key_down(ctx, vkey)
    if vkey == 0x30 then
        log("Reloading map.txt ...")
        remap = load_map(content_root .. "\\map.txt")
        info = info .. "\nmap reloaded"
    end
end

function m.init(ctx)
    if content_root:sub(1,1) == "." then
        content_root = ctx.sider_dir .. content_root
    end
    remap = load_map(content_root .. "\\map.txt")
    ctx.register("trophy_rewrite", m.trophy_rewrite)
    ctx.register("livecpk_make_key", m.make_key)
    ctx.register("livecpk_get_filepath", m.get_filepath)
    --ctx.register("overlay_on", m.overlay_on)
    ctx.register("key_down", m.key_down)
    log("trophy server: version " .. m.version)
end

return m
