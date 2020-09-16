-- Time accelerator: for non-exhibition matches

local m = {}

function m.set_match_time(ctx, minutes)
    if ctx.tournament_id ~= 65535 then
        -- some cup or league (not an exhibition match)
        local new_minutes = 1
        log(string.format("Accelerating match time: %d --> %d minutes",
            minutes, new_minutes))
        return new_minutes
    end
end

function m.init(ctx)
   ctx.register("set_match_time", m.set_match_time)
end

return m
