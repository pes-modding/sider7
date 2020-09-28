local m = {}

local cr_sound

function m.data_ready(ctx, filename)
    if string.match(filename, "goal\\cut_data\\goal_celebrate_0279_append02.fdc") then
        log("Ronaldo celebration loaded: " .. filename)
        -- Play C.Ronaldo "suuu!" sound, when he scores and does his unique celebration
        -- cr_suuu_d6.mp3 has 6 seconds of silence, followed by "suuu!"
        cr_sound = audio.new(ctx.sider_dir .. "content\\audio-demo\\cr_suuu_d6.mp3")
        cr_sound:set_volume(0.4)
        cr_sound:play()
        cr_sound:when_done(function()
            cr_sound = nil
        end)
    end
    if cr_sound then
        local another_celeb = string.match(filename, "goal\\cut_data\\goal_celebrate_(%d+)")
        if another_celeb and tonumber(another_celeb) ~= 279 then
            log("Another celebration loaded: " .. filename)
            -- cancel SUUU sound
            cr_sound:finish()
        end
    end
end

function m.init(ctx)
    ctx.register("livecpk_data_ready", m.data_ready)
end

return m
