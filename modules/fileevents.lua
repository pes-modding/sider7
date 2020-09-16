-- Track reads and data-ready events for specific file

local m = {}
local hex = memory.hex

local track_file = "shaders\\dx11\\GrModelShaders_dx11.fsop"

function m.read(ctx, filename, addr, len, total_size, offset)
    if filename == track_file then
        log(string.format("livecpk_read: %s, buffer:%s, length:%s, total:%s, offset:%s",
            filename, addr, hex(len), hex(total_size), hex(offset)))
    end
end

function m.data_ready(ctx, filename, addr, len, total_size, offset)
    if filename == track_file then
        log(string.format("livecpk_data_ready: %s, buffer:%s, length:%s, total:%s, offset:%s",
            filename, addr, hex(len), hex(total_size), hex(offset)))
    end
end

function m.init(ctx)
    ctx.register("livecpk_read", m.read)
    ctx.register("livecpk_data_ready", m.data_ready)
end

return m
