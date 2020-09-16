-- Let it snow

local m = {}

function m.set_conditions(ctx, options)
    log("let it snow!")
    options.weather = 2  -- snow
    options.weather_effects = 2  -- falling now
    options.season = 1  -- winter
    options.timeofday = 1  -- night
    return options
end

function m.init(ctx)
   ctx.register("set_conditions", m.set_conditions)
end

return m
