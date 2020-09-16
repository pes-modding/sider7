local m = {}

local cr_sound

function m.data_ready(ctx, filename)
    if filename == "common\\demo\\fixdemo\\goal\\cut_data\\goal_hugSet_starJump2018_base.fdc" then
        log("game loaded: " .. filename)
        -- Play C.Ronaldo "suuu!" sound, when he scores
        -- cr_suuu_d5.mp3 has 5 seconds of silence, followed by "suuu!"
        cr_sound = audio.new(ctx.sider_dir .. "content\\audio-demo\\cr_suuu_d5.mp3")
        cr_sound:set_volume(0.4)
        cr_sound:play()
    end
end

function m.init(ctx)
    ctx.register("livecpk_data_ready", m.data_ready)
end

return m
