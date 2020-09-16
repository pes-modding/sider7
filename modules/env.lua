-- print out the globals and context

local m = {}

function m.init(ctx)
    log("==> globals:")
    for k,v in pairs(_G) do
        log(string.format("%s: %s", k, v))
    end

    log("==> context:")
    for k,v in pairs(ctx) do
        log(string.format("%s: %s", k, v))
    end

    log("==> os:")
    if os then
        for k,v in pairs(os) do
            log(string.format("%s: %s", k, v))
        end
    end
end

return m
