-- Stadium switcher

local m = {}

function m.set_stadium(ctx, options)
    if ctx.tournament_id == 86 and ctx.match_info == 53 then
        -- Community Shield (English Super Cup)
        log("Community Shield: switching stadium to Anfield!")
        options.stadium = 4  -- Anfield
        return options
    end
end

function m.set_conditions(ctx, options)
    if ctx.tournament_id == 86 and ctx.match_info == 53 then
        -- Community Shield (English Super Cup)
        log("Community Shield: setting weather")
        options.weather = 2  -- snow
        options.weather_effects = 2  -- falling rain/snow
        options.timeofday = 1  -- night
        options.season = 1  -- winter
        return options
    end
end

function m.init(ctx)
   ctx.register("set_stadium", m.set_stadium)
   ctx.register("set_conditions", m.set_conditions)
end

return m
