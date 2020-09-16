-- Set match settings

local m = {}

-- difficulty: nil - game choice, 0 - beginner, 1 - amateur, ... 6 - Legend
m.difficulty = 6

-- extra time: nil - game choice, 0 - off, 1 - on
m.extra_time = 0

-- penalty shootout: nil - game choice, 0 - off, 1 - on
m.penalties = 1

function m.set_match_settings(ctx, options)
    if ctx.tournament_id == 65535 then
        -- for exhibition matches
        options.difficulty = m.difficulty
        options.extra_time = m.extra_time
        options.penalties = m.penalties
    else
        -- for non-exhibition matches: enforce number of subs
        options.substitutions = 5
    end
    return options
end

function m.init(ctx)
    ctx.register("set_match_settings", m.set_match_settings)
end

return m
