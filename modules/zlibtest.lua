-- zlib test module

local m = {}

local function hex_dump(data)
    -- helper function to log data in hex/ascii format: 16 bytes per line
    local offs = 1
    while offs <= #data do
        local bytes = data:sub(offs,offs+15)
        local hex_part = string.gsub(bytes, ".", function(c)
            return string.format("%02x ", string.byte(c))
        end)
        local ascii_part = string.gsub(bytes, ".", function(c)
            if string.byte(c)>=32 and string.byte(c)<127 then
                return c
            else
                return '.'
            end
        end)
        local padding = #bytes < 16 and string.rep('   ', 16-#bytes) or ''
        log(string.format("%08x: %s%s | %s", offs-1, hex_part, padding, ascii_part))
        offs = offs + 16
    end
end

function m.init(ctx)
    local data = "hello, world!"
    local compressed = zlib.compress(data)
    log(string.format("compressed: %s", memory.hex(compressed)))
    local uncompressed = zlib.uncompress(compressed)
    log(string.format("uncompressed: %s", uncompressed))
    assert(data == uncompressed)

    -- test: bogus compressed data
    local result,err = zlib.uncompress("plaintext")
    log(string.format("expected problem:: result: %s, error: %s", result, err))

    -- test: uncompressed buffer too small
    local result,err = zlib.uncompress(compressed, 4)
    log(string.format("expected problem:: result: %s, error: %s", result, err))

    -- testing Konami WESYS packing/unpacking
    local unpacked_data =
        "Hello, this is Unpacked Data!\n" ..
        "We have some useless text here, because well, because text\n" ..
        "compresses really well. But it could have been binary data too.\n" ..
        "Whatever it is that you want to pack :)"

    local packed = zlib.pack(unpacked_data)
    log("packed:")
    hex_dump(packed)
    local unpacked = zlib.unpack(packed)
    log("unpacked:")
    log(unpacked)
    assert(unpacked == unpacked_data)
end

return m
