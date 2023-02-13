-- fsdemo.lua
-- Demonstrates usage of fs.find_files function

local m = {}

local function walk(path)
    for name, ftype in fs.find_files(path .. "\\*") do
        if name ~= "." and name ~= ".." then
            log(string.format("Found: %s\\%s (%s)", path, name, ftype))
            if ftype == "dir" then
                walk(path .. "\\" .. name)
            end
        end
    end
end

function m.init(ctx)
    -- list every file and every dir in BallServer content folder
    walk(ctx.sider_dir .. "content\\ball-server")
end

return m
