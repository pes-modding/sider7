--[[
Example of simple "content-extractor": saves files that come from a chosen CPK.
This module automatically creates directory structure for files that are saved
--]]

local m = {}

local cpk_to_match = ".\\Data\\dt18_all.cpk"

local dumproot = ".\\cpk-dump\\"

function m.data_ready(ctx, filename, addr, len, total_size, offset, cpk_filename)
    --log("cpk_filename: " .. cpk_filename .. " filename: " .. filename)
    if cpk_filename == cpk_to_match and filename ~= cpk_filename then
        -- try opening the file
        local full_filename = dumproot .. filename
        log("full_filename: " .. full_filename)
        local f = io.open(full_filename, "r+b")
        if not f then
            f = io.open(full_filename, "wb")
        end
        if not f then
            -- cannot open the file: perhaps directories do not
            -- exist yet. Try to create them
            local dirname = string.match(full_filename, "(.*)[/\\].*")
            log("dirname: " .. dirname)
            local err = fs.make_dirs(dirname)
            if err then
                log("fs.make_dirs returned: " .. err)
            end
            -- try again
            f = assert(io.open(full_filename, "wb"),
                "PROBLEM: cannot open file for writing: " .. full_filename)
        end

        -- addr is actually a pointer to data in memory, so if we want
        -- to use this data, we need to make a copy of it:
        log("getting bytes...")
        local bytes,err = memory.read(addr, len)
        log(string.format("got bytes: %d (err=%s)", #bytes,  err))

        -- write to disk
        f:seek("set", offset)
        f:write(bytes)
        f:close()
        log(string.format("saved: %s", full_filename))
    end
end

function m.init(ctx)
    if dumproot:sub(1,1) == "." then
        dumproot = ctx.sider_dir .. dumproot
    end
    ctx.register("livecpk_data_ready", m.data_ready)
end

return m
