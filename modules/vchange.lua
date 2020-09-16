--[[
Example of value changed continuously by gamepad stick or keyboard
--]]

local m = {}

local some_value = 10.0
local delta = 0

-- frame_count is used for delaying the repeat
local frame_count = 0

function m.gamepad_input(ctx, inputs)
    for name,value in pairs(inputs) do
        if name == "RSx" then
            if value == -1 then
                -- moving left --> decrease value
                delta = -0.01
            elseif value == 1 then
                -- moving right --> increase value
                delta = 0.01
            else
                -- back to middle: stop changing
                delta = 0
            end
            some_value = some_value + delta
            frame_count = 0
        end
    end
end

local function repeat_change(num_frames)
    if delta ~= 0 then
        frame_count = frame_count + 1
        if frame_count >= num_frames then
            some_value = some_value + delta
        end
    end
end

function m.key_down(ctx, vkey)
    if vkey == 0x21 then -- PageUp
        some_value = some_value - 0.01
    elseif vkey == 0x22 then -- PageDown
        some_value = some_value + 0.01
    end
end

function m.overlay_on(ctx)
    repeat_change(30)
    return string.format("Value: %0.3f", some_value)
end

function m.init(ctx)
    ctx.register("key_down", m.key_down)
    ctx.register("gamepad_input", m.gamepad_input)
    ctx.register("overlay_on", m.overlay_on)
end

return m
