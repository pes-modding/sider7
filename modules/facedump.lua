--[[

Example of simple "content-extractor": saves all files that belong to chosen faces to disk.
This module automatically creates directory structure for files that are saved, which requires
LuaJIT ffi module to be available. To enable it make sure to have this setting to your sider.ini:

luajit.ext.enabled = 1

--]]

local m = {}
ffi.cdef[[
int CreateDirectoryA(char *path, uint64_t pattrs);
]]
local null_ptr = ffi.new("uint64_t")

local dumproot = ".\\facedump\\"

-- change this to whatever "real" faces you want to dump
local patterns = {
    "Asset\\model\\character\\face\\real\\40571\\", -- De Gea
    "common\\render\\symbol\\player\\40571%.dds",
    "Asset\\model\\character\\face\\real\\45023\\", -- Pogba
    "common\\render\\symbol\\player\\45023%.dds",
    "Asset\\model\\character\\face\\real\\57123\\", -- Salah
    "common\\render\\symbol\\player\\57123%.dds",
}

local function create_dirs_for(pathname)
    local path = ""
    for name in string.gmatch(pathname, "[^\\]+\\") do
        --log(string.format("name={%s}", name))
        path = path .. name
        if path ~= pathname then
            --log(string.format("path={%s}", path))
            local cstr = ffi.new("char[?]",#path+1)
            ffi.copy(cstr,path)
            ffi.C.CreateDirectoryA(cstr, null_ptr)
        end
    end
end

function m.data_ready(ctx, filename, addr, len, total_size, offset)
    for i,pattern in ipairs(patterns) do
        if string.match(filename, pattern) then
            -- try opening the file
            local full_filename = dumproot .. filename
            local f = io.open(full_filename, "wb")
            if not f then
                -- cannot open the file: perhaps directories do not
                -- exist yet. Create them
                create_dirs_for(full_filename)
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
            f:write(bytes)
            f:close()
            log(string.format("saved: %s", full_filename))
            break
        end
    end
end

function m.init(ctx)
    if dumproot:sub(1,1) == "." then
        dumproot = ctx.sider_dir .. dumproot
    end
    ctx.register("livecpk_data_ready", m.data_ready)
end

return m
