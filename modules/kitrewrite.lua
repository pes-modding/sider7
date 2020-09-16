-- simple kit textures rewrite example:
-- make the goalkeeper wear 2nd player kit

local m = {}

function m.rewrite(ctx, filename)
    --log(filename)
    local team_id = string.match(filename, "\\u(%d+)g1.*%.ftex")
    if team_id then
        return string.gsub(filename, "g1", "p2")
    end
end

function m.init(ctx)
    ctx.register("livecpk_rewrite", m.rewrite)
end

return m
