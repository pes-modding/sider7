--[[
Simple example of using "match" library, to show statistics of current match.
IMPORTANT: the lib is still experimental, so it is disabled by default.
To enable, you need to have this line in your sider.ini:

match-stats.enabled = 1

--]]

local m = {}
local hex = memory.hex

function m.overlay_on(ctx)
    local stats = match.stats()
    if stats then
        return string.format("match stats (%s): %d-%d (PK: %d-%d), period:%d, clock: %02d:%02d %s",
            hex(stats.ptr),
            stats.home_score, stats.away_score,
            stats.pk_home_score, stats.pk_away_score,
            stats.period, stats.clock_minutes, stats.clock_seconds,
            stats.added_minutes and string.format("(+%d)", stats.added_minutes) or "")
    end
    return "no stats"
end

function m.init(ctx)
    ctx.register("overlay_on", m.overlay_on)
end

return m
