local m = {}
local img

function m.render(ctx, screen_width, screen_height)
    -- put image into bottom rigtht corner with some padding
    sprite(img, screen_width - img.width - 5, screen_height - img.height - 5)
end

function m.setup(ctx)
    img = image(ctx.sider_dir .. "sider-icon.dds")
    log(string.format("w:%d, h:%d, file:%s", img.width, img.height, img.filename))
end

function m.init(ctx)
    ctx.register("graphics_ready", m.setup)
    ctx.register("display_frame", m.render)
end

return m
