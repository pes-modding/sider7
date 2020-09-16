-- Dump specified files to disk, after the game read them
-- (if the contents are zlibbed in a "WESYS" container - unzlib it too)

local m = {}

local to_dump = {
    ["common\\character0\\model\\character\\uniform\\team\\UniColor.bin"] = "UniColor.bin",
    ["common\\etc\\pesdb\\Team4.bin"] = "Team4.bin",
}

function m.data_ready(ctx, filename, addr, len, total_size, offset)
    local localfile = to_dump[filename]
    if localfile then
        -- addr is actually a pointer to data in memory, so if we want
        -- to use this data later, we need to make a copy of it now:
        local bytes = memory.read(addr, len)

        -- unzlib if zlibbed
        bytes = zlib.unpack(bytes)

        local f = assert(io.open(ctx.sider_dir .. localfile, "wb"))
        f:write(bytes)
        f:close()
        log("saved to: " ..  ctx.sider_dir .. localfile)
    end
end

function m.init(ctx)
    ctx.register("livecpk_data_ready", m.data_ready)
end

return m
