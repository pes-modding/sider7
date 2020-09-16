-- Dump a file to disk, after the game read it

local m = {}

function m.data_ready(ctx, filename, addr, len, total_size, offset)
    if filename == "shaders\\dx11\\GrModelShaders_dx11.fsop" then
        -- addr is actually a pointer to data in memory, so if we want
        -- to use this data later, we need to make a copy of it now:
        local bytes = memory.read(addr, len)

        local f = assert(io.open(ctx.sider_dir .. "GrModelShaders_dx11.fsop", "wb"))
        f:write(bytes)
        f:close()
    end
end

function m.init(ctx)
    ctx.register("livecpk_data_ready", m.data_ready)
end

return m
