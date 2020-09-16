-- Module to prevent the game from connecting to network
-- Also, skips LiveUpdate before exhibition matches
-- by Hawke, zlac and juce

local strings = {
    "pes21-x64-gate.cs.konami.net",
}
local replacer = "0.0.0.0"
local content_root = ".\\content\\netblock\\"

local m = {}
m.version = "0.2"

function m.get_filepath(ctx, filename, key)
    --log(string.format("wants: %s", filename))
    if key then
        return content_root .. key
    end
end

function m.init(ctx)
    log("version " .. m.version)
    if content_root:sub(1,1) == "." then
        content_root = ctx.sider_dir .. content_root
    end

    ctx.register("livecpk_get_filepath", m.get_filepath)

    for _,s in ipairs(strings) do
        addr, info = memory.search_process(s .. "\x00")
        if not addr then
            log(string.format('warning: unable to find string: "%s" in memory', s))
        else
            log(string.format('string "%s" found at %s. Replacing with "%s"', s, memory.hex(addr), replacer))
            memory.write(addr, replacer .."\x00")
        end
    end
end

return m
