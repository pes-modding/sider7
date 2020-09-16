--[[
=========================

inputdemo module
Requires: sider.dll 5.2.1

Demonstrates usage of: "overlay_on", "key_down" and "gamepad_input" events

"overlay_on" handler is called for every frame, when the overlay is displayed
and when the current module is in control of the overlay.

"key_down" handler is called, when user presses a key. Virtual Key Codes (or "vkeys")
can be determined by experiment, or can be looked up here:
https://docs.microsoft.com/en-us/windows/desktop/inputdev/virtual-key-codes

"gamepad_input" handler is called, when gamepad controlls are used. Notice that
separate events are generated for button presses/releases, and for movement of
sticks and gamepads. The handler receives two parameters: ctx and inputs
    ctx - sider context object
    inputs - a table of input events

=========================
--]]

local m = {}
local version = "1.0"

local text = ""
local lines = {}
local MAX_LINES = 20

function m.overlay_on(ctx)
    return string.format([[version %s
Press buttons on keyboard, move sticks or press buttons on gamepad
Last %s events:

%s]], version, MAX_LINES, text)
end

local function get_last(t, n)
    local new_t = {}
    local first = math.max(1, #t - n + 1)
    for i=first,first+n-1 do
        new_t[#new_t + 1] = t[i]
    end
    return new_t
end

function m.key_down(ctx, vkey)
    lines[#lines + 1] = string.format("Keyboard input event: vkey=0x%x", vkey)
    lines = get_last(lines, MAX_LINES)
    text = table.concat(lines, '\n')
end

function m.gamepad_input(ctx, inputs)
    for name,value in pairs(inputs) do
        lines[#lines + 1] = string.format("Gamepad input event: name=%s, value=%d", name, value)
    end
    lines = get_last(lines, MAX_LINES)
    text = table.concat(lines, '\n')
end

function m.init(ctx)
    -- register for events
    ctx.register("overlay_on", m.overlay_on)
    ctx.register("key_down", m.key_down)
    ctx.register("gamepad_input", m.gamepad_input)
end

return m
