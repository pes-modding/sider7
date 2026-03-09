--[[
=========================

overlay_colors_test module v1.1
Requires: sider.dll 7.1.0+ with dynamic overlay properties support

Demonstrates usage of dynamic overlay properties:
- text_color: Customize the text color ("RRGGBBAA" hex string format)
- background_color: Customize the background color ("RRGGBBAA" hex string format)
- image_alpha: Override the image transparency (0.0 to 1.0)
- location: Position the overlay at top or bottom

The overlay_on handler can now return a 3rd value (options table) with:
{
    text_color = "RRGGBBAA",           -- e.g., "FF0000FF" for red text
    background_color = "RRGGBBAA",     -- e.g., "00FF0080" for semi-transparent green
    image_alpha = 0.5,                 -- e.g., 0.5 for 50% opaque image
    location = "top" | "bottom",       -- Location on screen
    image_width = pixels or 0-1,       -- Image dimensions (existing)
    image_height = pixels or 0-1,      -- Image dimensions (existing)
}

Color format: "RRGGBBAA" (hexadecimal string, same as sider.ini format)
- RR = Red (00-FF)
- GG = Green (00-FF)
- BB = Blue (00-FF)
- AA = Alpha/Transparency (00=fully transparent, FF=fully opaque)

Image alpha: 0.0 = fully transparent, 1.0 = fully opaque (default from sider.ini)

Examples:
- Red text with full opacity:      "FF0000FF"
- Green background semi-transparent: "00FF0080"
- Blue with 50% transparency:      "0000FF80"
- Black text fully opaque:         "000000FF"
- White text fully opaque:         "FFFFFFFF"

=========================
--]]

local m = {}
local version = "1.0"

-- Test state
local test_index = 0
local test_configs = {
    {
        name = "Default colors",
        text_color = nil, -- Use sider.ini default
        bg_color = nil,   -- Use sider.ini default
        location = nil,   -- Use sider.ini default
    },
    {
        name = "Red text, dark blue background",
        text_color = "FF0000FF", -- Red text
        bg_color = "000080C0",   -- Dark blue semi-transparent
        location = "top",
    },
    {
        name = "Yellow text, transparent background",
        text_color = "FFFF00FF", -- Yellow text
        bg_color = "204020A0",   -- Very transparent green
        location = "top",
    },
    {
        name = "White text, black background",
        text_color = "FFFFFFFF", -- White text
        bg_color = "000000C0",   -- Black with 75% opacity
        location = "bottom",
    },
    {
        name = "Cyan text bottom, semi-transparent image",
        text_color = "00FFFFFF", -- Cyan text
        bg_color = "402040AA",   -- Purple semi-transparent
        image_alpha = 0.5,        -- Image at 50% opacity
        location = "bottom",
    },
}

function m.overlay_on(ctx)
    local config = test_configs[test_index + 1]

    local text = string.format([[Test %d of %d: %s
Press [0] to cycle through color presets
Current:
- Text: %s
- BG: %s
- Location: %s
]],
        test_index + 1,
        #test_configs,
        config.name,
        config.text_color or "(default)",
        config.bg_color or "(default)",
        config.location or "(default)")

    -- Build the options table
    local options = {}
    if config.text_color then
        options.text_color = config.text_color
    end
    if config.bg_color then
        options.background_color = config.bg_color
    end
    if config.image_alpha then
        options.image_alpha = config.image_alpha
    end
    if config.location then
        options.location = config.location
    end

    return text, nil, options
end

function m.key_down(ctx, vkey)
    if vkey == 0x30 then -- [0] key
        test_index = (test_index + 1) % #test_configs
    end
end

function m.init(ctx)
    -- register for events
    ctx.register("overlay_on", m.overlay_on)
    ctx.register("key_down", m.key_down)
end

return m
