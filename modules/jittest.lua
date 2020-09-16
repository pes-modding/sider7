-- JIT test: print the status of the LuaJIT Just-In-Time compilter

local m = {}

function m.init(ctx)
    log(string.format("jit: %s", jit))
    if jit then
        local info = {}
        for i,v in ipairs({jit.status()}) do
            info[#info + 1] = tostring(v)
        end
        log(string.format("status: %s", table.concat(info, " ")))
    end
end

return m
