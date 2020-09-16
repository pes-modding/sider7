--[[
testing large allocation
This demo module requires LuaJIT ffi module to be enabled in sider.ini with:

luajit.ext.enabled = 1

--]]

local m = {}

local t = {}

function m.init(ctx)
    local num_bytes = 1024*1024*1024
    local ctype = string.format("char[%s]", num_bytes)
    for i=1,8 do
        local buf = ffi.new(ctype)
        ffi.fill(buf, num_bytes, 0x40+i)
        local chunk = ffi.string(buf, num_bytes)
        collectgarbage("collect")
        log(string.format("successfully allocated chunk %s of size: %s", i, #chunk))
        t[#t+1] = chunk
    end
    log(string.format("Lua memory usage now: %0.3f KB", collectgarbage("count")))
    log(t[1]:sub(1,10))
    log(t[2]:sub(1,10))
    log(t[3]:sub(1,10))
end

return m
