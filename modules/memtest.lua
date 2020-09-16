-- demo and tests for memory module

local hex = memory.hex

local tests = {
    { "f",  '\x66\xe6\xf6\x42', 123.45, 0.00001 },
    { "d",  '\xcd\xcc\xcc\xcc\xcc\xdc\x5e\x40', 123.45, 0.00000001 },
    { "i64",'\x10\x20\x30\x40\x87\xd6\x12\x30', 0x3012d68740302010LL, 0, },
    { "i64",'\xfd\xff\xff\xff\xff\xff\xff\xff', -3LL, 0, },
    { "i",  '\x87\xd6\x12\x00', 1234567, 0, },
    { "s",  '\x39\x30', 12345, 0, },
    { "u64",'\x10\x20\x30\x40\x87\xd6\x12\xcf', 0xcf12d68740302010ULL, 0, },
    { "u64",'\x00\x01\x00\x00\x00\x00\x00\x00', 0x100ULL, 0, },
    { "ui", '\x00\x5e\xd0\xb2', 3000000000, 0, },
    { "us", '\x69\xa5', 42345, 0, },
}

function init(ctx)
    for _,t in ipairs(tests) do
        local fmt, s, v, err = t[1], t[2], t[3], t[4]
        local packed = memory.pack(fmt, v)
        local unpacked = memory.unpack(fmt, packed);
        log(string.format("%s vs %s", hex(s), hex(packed)))
        assert(s == packed)
        log(string.format("%s vs %s", v, unpacked))
        assert(err >= math.abs(tonumber(v - unpacked)))
    end

    local s = memory.read(0x14000b00eULL, 12)
    log(string.format("%s bytes at address %s: %s", #s, hex(0x14000b00eULL), hex(s)))
    local n1 = memory.unpack("i64", s)
    local n2 = memory.unpack("u64", s)
    log(string.format("first 8 bytes as signed int: %s (%s)", hex(n1), n1))
    log(string.format("first 8 bytes as unsigned int: %s (%s)", hex(n2), n2))
end

return { init = init }
