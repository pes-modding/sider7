-- Event tracer
-- prints event and context info, reacting to various events

local m = {}

local function t2s(t)
    local parts = {}
    for k,v in pairs(t) do
        parts[#parts + 1] = string.format("%s=%s", k, v)
    end
    table.sort(parts) -- sort alphabetically
    return string.format("{%s}", table.concat(parts,", "))
end

-- utility function to log message with a timestamp
local function tlog(...)
    local msg = string.format(...)
    log(string.format("%s | %s", os.date("%Y-%m-%d %H:%M:%S"), msg))
end

function m.set_teams(ctx, home, away)
    tlog("teams: %d vs %d", home, away)
end

function m.set_match_time(ctx, num_minutes)
    tlog("match_time: %d", num_minutes)
end

function m.set_stadium_choice(ctx, stadium_choice)
    tlog("set_stadium_choice: %d", stadium_choice)
end

function m.set_stadium(ctx, options)
    tlog("set_stadium: %s", t2s(options))
end

function m.set_conditions(ctx, options)
    tlog("set_conditions: %s", t2s(options))
end

function m.set_match_settings(ctx, options)
    tlog("set_match_settings: %s", t2s(options))
end

function m.after_set_conditions(ctx)
    tlog("after_set_conditions")
    tlog("ctx: %s", t2s(ctx))
end

function m.get_ball_name(ctx, ball_name)
    tlog("get_ball_name: %s", ball_name)
end

function m.get_stadium_name(ctx, stadium_name)
    tlog("get_stadium_name: %s", stadium_name)
end

local opts = { image_width = 32, image_hmargin = 4, image_vmargin = 2 }
function m.overlay_on(ctx)
    local memory_used = collectgarbage("count")
    local text = string.format("ctx: %s\nLua memory used: %d KB", t2s(ctx), memory_used)
    local image_path = ctx.sider_dir .. "sider-icon.dds"
    return text, image_path, opts
end

function m.init(ctx)
    ctx.register("set_teams", m.set_teams)
    ctx.register("set_match_time", m.set_match_time)
    ctx.register("set_stadium_choice", m.set_stadium_choice)
    ctx.register("set_stadium", m.set_stadium)
    ctx.register("set_conditions", m.set_conditions)
    ctx.register("set_match_settings", m.set_match_settings)
    ctx.register("after_set_conditions", m.after_set_conditions)
    ctx.register("get_ball_name", m.get_ball_name)
    ctx.register("get_stadium_name", m.get_stadium_name)
    ctx.register("overlay_on", m.overlay_on)
end

return m
