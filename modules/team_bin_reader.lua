-- Log some team names from Team.bin

local m = {}

local team_bin_chunks = {}

function print_teams(team_bin)
    local block = 1468 -- block size for one team
    local teamName_offs = 234  -- zero-based offset ... very lua-unlike :)
    local teamName_len = 70

    local base = 0
    while base < #team_bin do
        local sub_data = string.sub( team_bin, base + teamName_offs + 1, base + teamName_offs + teamName_len )
        local name = string.match( sub_data, "[^%z]+")  -- trim trailing zeros
        log("Team name (raw): " .. name)  -- seems to be working

        -- next
        base = base + block
    end
end

function m.read(ctx, filename, addr, len, total_size, offset)
    if filename == "common\\etc\\pesdb\\Team.bin" then
        -- addr is actually a pointer to data in memory, so if we want
        -- to use this data later, we need to make a copy of it now:
        local bytes = memory.read(addr, len)

        -- accumulate data chunks in a table, in case the file is large and gets loaded in mulitiple reads
        team_bin_chunks[#team_bin_chunks + 1] = bytes

        if offset + len >= total_size then
            -- got everything: now combine all chunks into one binary str
            local team_bin = table.concat(team_bin_chunks)

            -- do something useful with the data :)
            print_teams(team_bin)

            -- release memory held by chunks table
            team_bin_chunks = {}
        end
    end
end

function m.init(ctx)
    ctx.register("livecpk_read", m.read)
end

return m
