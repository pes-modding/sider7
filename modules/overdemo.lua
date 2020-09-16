--[[
=========================

overdemo module
Requires: sider.dll 5.1.0

Demonstrates usage of: "overlay_on" and "key_down" events

"overlay_on" handler is called for every frame, when the overlay is displayed
and when the current module is in control of the overlay.

"key_down" handler is called, when user presses a key. Virtual Key Codes (or "vkeys")
can be determined by experiment - for example, this module will display the
code of the last vkey that it received.

All keys can also be found here:
https://docs.microsoft.com/en-us/windows/desktop/inputdev/virtual-key-codes

=========================
--]]

local m = {}
local PREV_VALUE_KEY = 0xbd
local NEXT_VALUE_KEY = 0xbb

local text = ""
local lines_count = 0
local last_vkey

function m.overlay_on(ctx)
    local kstr = last_vkey and string.format("0x%02x", last_vkey) or "None"
    return string.format("Last vkey: %s | Press [+][-] buttons to add/remove text%s", kstr, text)
end

function m.key_down(ctx, vkey)
    last_vkey = vkey
    if vkey == PREV_VALUE_KEY then
        if lines_count > 0 then
            lines_count = lines_count - 1
            local t = {}
            for i=1,lines_count do t[i] = "\n" .. tostring(i) .. " some text" end
            text = table.concat(t)
        end
    elseif vkey == NEXT_VALUE_KEY then
        if lines_count < 100 then
            lines_count = lines_count + 1
            local t = {}
            for i=1,lines_count do t[i] = "\n" .. tostring(i) .. " some text" end
            text = table.concat(t)
        end
    end
end

function m.init(ctx)
    -- register for events
    ctx.register("overlay_on", m.overlay_on)
    ctx.register("key_down", m.key_down)
end

return m
