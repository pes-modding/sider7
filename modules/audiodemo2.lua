local m = {}

local sound
local volume

local function pause_sound()
    if sound then
        -- remember volume of this sound, so that we can resume playback later at the same volume
        volume = sound:get_volume()
        -- pause with 2.5 second fade-out
        sound:fade_to(0, 2.5)
        sound:pause()
    end
end

local function resume_sound()
    if sound then
        -- resume play with 2.5 second fade-in
        sound:fade_to(volume, 2.5)
        sound:play()
    end
end

function m.key_up(ctx, vkey)
    if vkey == 0x35 then
        if sound == nil or sound:get_state() == "finished" then
            -- not playing: Let's start a new sound
            sound = audio.new(ctx.sider_dir .. "content\\audio-demo\\uefa.mp3")
            sound:set_volume(0.7)
            sound:play()
        elseif sound:get_state() == "playing" then
            pause_sound()
        elseif sound:get_state() == "paused" then
            resume_sound()
        end
    end
end

function m.overlay_on(ctx)
    if sound ~= nil then
        return string.format("sound %s (volume: %0.2f): %s", sound:get_filename(), sound:get_volume(), sound:get_state())
    else
        return "no sound. Press [5] to play/pause"
    end
end

function m.init(ctx)
    ctx.register("key_up", m.key_up)
    ctx.register("overlay_on", m.overlay_on)
end

return m
