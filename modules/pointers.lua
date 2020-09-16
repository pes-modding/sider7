local m = {}

function m.overlay_on(ctx)
    if ctx.mis then
        -- "ctx.mis" refers to MATCH_INFO structure
        -- which has some interesting stuff

        -- cast to a char pointer
        local p = ffi.cast("char*", ctx.mis)

        -- now we can do pointer arithmetic and memory access
        local home_name = memory.read(p + 0x13c, 0x46)
        local away_name = memory.read(p + 0x690 + 0x13c, 0x46)

        -- strip trailing zeros from names: for better display
        home_name = string.match(home_name, '[^%z]*')
        away_name = string.match(away_name, '[^%z]*')

        -- show in overlay
        return string.format('Match: %s vs %s', home_name, away_name)
    end
end

function m.init(ctx)
    if not ffi then
        error('LuaJIT FFI is disabled. Enable it with "luajit.ext.enabled = 1" in sider.ini')
    end
    ctx.register("overlay_on", m.overlay_on)
end

return m

