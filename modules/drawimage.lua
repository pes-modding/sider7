local m = {}
local image_path

function m.render(ctx, screen_width, screen_height)
    sprite(image_path, 40, 48)
end

function m.init(ctx)
    image_path = ctx.sider_dir .. "sider-icon.dds"
    ctx.register("display_frame", m.render)
end

return m
