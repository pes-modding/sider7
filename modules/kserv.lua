-- kserv.lua
-- Experimental kitserver with GDB

local m = {}

m.version = "1.4"

local kroot = ".\\content\\kit-server\\"
local kmap

local home_kits
local away_kits
local home_gk_kits
local away_gk_kits

local home_next_kit
local away_next_kit
local home_next_gk_kit
local away_next_gk_kit

local is_gk_mode = false
local modes = { "PLAYERS", "KEEPERS" }

-- final selections
local p_home
local p_away
local g_home
local g_away

local patterns = {
    ["\\(k%d+p%d+)%.ftex$"] = "KitFile",
    ["\\(k%d+p%d+_srm)%.ftex$"] = "KitFile_srm",
    ["\\(k%d+p%d+_c)%.ftex$"] = "ChestNumbersFile",
    ["\\(k%d+p%d+_l)%.ftex$"] = "LegNumbersFile",
    ["\\(k%d+p%d+_b)%.ftex$"] = "BackNumbersFile",
    ["\\(k%d+p%d+_n)%.ftex$"] = "NameFontFile",
    ["\\(k%d+g%d+)%.ftex$"] = "KitFile",
    ["\\(k%d+g%d+_srm)%.ftex$"] = "KitFile_srm",
    ["\\(k%d+g%d+_c)%.ftex$"] = "ChestNumbersFile",
    ["\\(k%d+g%d+_l)%.ftex$"] = "LegNumbersFile",
    ["\\(k%d+g%d+_b)%.ftex$"] = "BackNumbersFile",
    ["\\(k%d+g%d+_n)%.ftex$"] = "NameFontFile",
}
local ks_player_formats = {
    KitFile = "k%dp%d",
    KitFile_srm = "k%dp%d_srm",
    ChestNumbersFile = "k%dp%d_c",
    LegNumbersFile = "k%dp%d_l",
    BackNumbersFile = "k%dp%d_b",
    NameFontFile = "k%dp%d_n",
}
local ks_gk_formats = {
    KitFile = "k%dg%d",
    KitFile_srm = "k%dg%d_srm",
    ChestNumbersFile = "k%dg%d_c",
    LegNumbersFile = "k%dg%d_l",
    BackNumbersFile = "k%dg%d_b",
    NameFontFile = "k%dg%d_n",
}
local filename_keys = {
    "KitFile", "KitFile_srm", "ChestNumbersFile",
    "LegNumbersFile", "BackNumbersFile", "NameFontFile",
}

local kfile_remap = {}

local function file_exists(filename)
    local f = io.open(filename)
    if f then
        f:close()
        return true
    end
end

local function t2s(t)
    local parts = {}
    for k,v in pairs(t) do
        parts[#parts + 1] = string.format("%s=%s", k, v)
    end
    table.sort(parts) -- sort alphabetically
    return string.format("{%s}", table.concat(parts,", "))
end

local function table_copy(t)
    local new_t = {}
    for k,v in pairs(t) do
        new_t[k] = v
    end
    return new_t
end

local function load_map(filename)
    local map = {}
    for line in io.lines(filename) do
        -- strip comment
        line = string.gsub(line, "#.*", "")
        tid, path = string.match(line, "%s*(%d+)%s*,%s*[\"]?([^\"]*)")
        tid = tonumber(tid)
        if tid and path then
            map[tid] = path
            log(string.format("tid: %d ==> content path: %s", tid, path))
        end
    end
    return map
end

local function load_config(filename)
    local cfg = {}
    local key, value
    local f = io.open(filename)
    if not f then
        -- don't let io.lines raise an error, if config is missing
        -- instead, just ignore that kit
        return
    end
    for line in io.lines(filename) do
        -- strip comment
        line = string.gsub(line, ";.*", "")
        key, value = string.match(line, "%s*(%w+)%s*=%s*[\"]?([^\"]*)")
        value = tonumber(value) or value
        if key and value then
            cfg[key] = value
        end
    end
    return cfg
end

local function load_configs_for_team(team_id)
    local path = kmap[team_id]
    if not path then
        -- no kits for this team
        log(string.format("no kitserver kits for: %s", team_id))
        return nil, nil
    end
    log(string.format("looking for configs for: %s", path))
    -- players
    local pt = {}
    local f = io.open(kroot .. path .. "\\order.txt")
    if f then
        f:close()
        for line in io.lines(kroot .. path .. "\\order.txt") do
            line = string.gsub(string.gsub(line,"%s*$", ""), "^%s*", "")
            local filename = kroot .. path .. "\\" .. line .. "\\config.txt"
            local cfg = load_config(filename)
            if cfg then
                pt[#pt + 1] = { line, cfg }
            else
                log("WARNING: unable to load kit config from: " .. filename)
            end
        end
    end
    -- goalkeepers
    local gt = {}
    local f = io.open(kroot .. path .. "\\gk_order.txt")
    if f then
        f:close()
        for line in io.lines(kroot .. path .. "\\gk_order.txt") do
            line = string.gsub(string.gsub(line,"%s*$", ""), "^%s*", "")
            local filename = kroot .. path .. "\\" .. line .. "\\config.txt"
            local cfg = load_config(filename)
            if cfg then
                gt[#gt + 1] = { line, cfg }
            else
                log("WARNING: unable to load GK kit config from: " .. filename)
            end
        end
    end
    return pt, gt
end

local standard_kits = { "p1", "p2", "p3", "p4", "p5", "p6", "p7", "p8", "p9" }
local standard_gk_kits = { "g1" }

local function get_curr_kit(ctx, team_id, home_or_away)
    local kit_id = ctx.kits.get_current_kit_id(home_or_away)
    local kit_path = standard_kits[kit_id] or standard_kits[1]
    return { kit_path, ctx.kits.get(team_id, kit_id) }
end

local function get_curr_gk_kit(ctx, team_id)
    local kit_path = standard_gk_kits[1]
    return { kit_path, ctx.kits.get_gk(team_id) }
end

local function dump_kit_config(filename, t)
    -- utility method, for easy dumping of configs to disk
    -- (for debugging purposes)
    local f = assert(io.open(filename,"wt"))
    f:write("; Kit config dumped by kserv.lua\n\n")
    local keys = {}
    for k,v in pairs(t) do
        keys[#keys + 1] = k
    end
    table.sort(keys)
    for i,k in ipairs(keys) do
        f:write(string.format("%s=%s\n", k, t[k]))
    end
    f:close()
end

local function prep_home_team(ctx)
    -- see what kits are available
    home_kits, home_gk_kits = load_configs_for_team(ctx.home_team)
    home_next_kit = home_kits and #home_kits>0 and 0 or nil
    home_next_gk_kit = home_gk_kits and #home_gk_kits>0 and 0 or nil
end

local function prep_away_team(ctx)
    -- see what kits are available
    away_kits, away_gk_kits = load_configs_for_team(ctx.away_team)
    away_next_kit = away_kits and #away_kits>0 and 0 or nil
    away_next_gk_kit = away_gk_kits and #away_gk_kits>0 and 0 or nil
end

local function config_update_filenames(team_id, ord, kit_path, kit_cfg, formats)
    local path = kmap[team_id]
    if not path then
        return
    end
    for _,k in ipairs(filename_keys) do
        local attr = kit_cfg[k]
        if attr and attr ~= "" then
            local pathname = string.format("%s\\%s\\%s.ftex", path, kit_path, attr)
            log("checking: " .. kroot .. pathname)
            if file_exists(kroot .. pathname) then
                --[[
                rewrite filename in uniparam config to a "fake" file
                that uniquely maps to a specific kit for this team.
                Later, when the game requests this texture, we use
                "livecpk_make_key" and "livecpk_get_filepath" to feed
                the actual ftex from GDB
                --]]
                local fmt = formats[k]
                if fmt then
                    local fkey = string.format(fmt, team_id, ord)
                    kfile_remap[fkey] = pathname
                    kit_cfg[k] = fkey
                end
            end
        end
    end
end

local function update_kit_config(team_id, kit_ord, kit_path, cfg)
    -- insert _srm property, if not there
    -- (Standard configs do not have it, because the game
    -- just adds an "_srm" suffix to KitFile. But because
    -- we are remapping names, we need to account for that)
    if cfg.KitFile and not cfg.KitFile_srm then
        cfg.KitFile_srm = cfg.KitFile .. "_srm"
    end
    -- trick: mangle the filenames so that we can livecpk them later
    -- (only do that for files that actually exist in kitserver content)
    kit_ord = kit_ord or (kit_path == "p2" and 2 or 1)
    config_update_filenames(team_id, kit_ord, kit_path, cfg, ks_player_formats)
end

local function update_gk_kit_config(team_id, kit_ord, kit_path, cfg)
    -- insert _srm property, if not there
    -- (Standard configs do not have it, because the game
    -- just adds an "_srm" suffix to KitFile. But because
    -- we are remapping names, we need to account for that)
    if cfg.KitFile and not cfg.KitFile_srm then
        cfg.KitFile_srm = cfg.KitFile .. "_srm"
    end
    -- trick: mangle the filenames so that we can livecpk them later
    -- (only do that for files that actually exist in kitserver content)
    kit_ord = kit_ord or 1
    config_update_filenames(team_id, kit_ord, kit_path, cfg, ks_gk_formats)
end

function m.set_kits(ctx, home_info, away_info)
    is_gk_mode = false -- always start in Players mode
    log(string.format("home_info (team=%d): %s", ctx.home_team, t2s(home_info)))
    log(string.format("away_info (team=%d): %s", ctx.away_team, t2s(away_info)))

    --dump_kit_config(string.format("%s%d-%s-config.txt", ctx.sider_dir, ctx.home_team, home_info.kit_id), home_info)
    --dump_kit_config(string.format("%s%d-%s-config.txt", ctx.sider_dir, ctx.away_team, away_info.kit_id), away_info)

    prep_home_team(ctx)
    prep_away_team(ctx)

    -- load corresponding kits, if available in GDB
    local hi
    if home_kits and #home_kits > 0 then
        local kit_id = ctx.kits.get_current_kit_id(0)
        local ki = home_kits[kit_id+1]
        hi = ki and ki[2] or nil
        if hi then
            local kit_path = ki[1]
            log("loading home kit: "  .. kit_path)
            home_next_kit = kit_id+1
            update_kit_config(ctx.home_team, home_next_kit, kit_path, hi)
            log(string.format("home cfg returned (%s): %s", kit_path, t2s(hi)))
        end
        p_home = get_curr_kit(ctx, ctx.home_team, 0)
    end
    local ai
    if away_kits and #away_kits > 0 then
        local kit_id = ctx.kits.get_current_kit_id(1)
        local ki = away_kits[kit_id+1]
        ai = ki and ki[2] or nil
        if ai then
            local kit_path = ki[1]
            log("loading away kit: "  .. kit_path)
            away_next_kit = kit_id+1
            update_kit_config(ctx.away_team, away_next_kit, kit_path, ai)
            log(string.format("away cfg returned (%s): %s", kit_path, t2s(ai)))
        end
        p_away = get_curr_kit(ctx, ctx.away_team, 1)
    end

    -- set gk kits, if we have them
    if home_gk_kits and #home_gk_kits>0 then
        local kit_id = ctx.kits.get_current_kit_id(0)
        local ki = home_gk_kits[kit_id+1]
        local cfg = ki and ki[2] or nil
        if cfg then
            local kit_path = ki[1]
            log("loading home GK kit: "  .. kit_path)
            home_next_gk_kit = kit_id+1
            update_gk_kit_config(ctx.home_team, home_next_gk_kit, kit_path, cfg)
            ctx.kits.set_gk(ctx.home_team, cfg)
        end
        g_home = get_curr_gk_kit(ctx, ctx.home_team)
    end
    if away_gk_kits and #away_gk_kits>0 then
        local kit_id = ctx.kits.get_current_kit_id(1)
        local ki = away_gk_kits[kit_id+1]
        local cfg = ki and ki[2] or nil
        if cfg then
            local kit_path = ki[1]
            log("loading away GK kit: "  .. kit_path)
            away_next_gk_kit = kit_id+1
            update_gk_kit_config(ctx.away_team, away_next_gk_kit, kit_path, cfg)
            ctx.kits.set_gk(ctx.away_team, cfg)
        end
        g_away = get_curr_gk_kit(ctx, ctx.away_team)
    end
    --dump_kit_config(string.format("%s%d-%s-gk-config.txt", ctx.sider_dir, ctx.home_team, 0), g_home[2])
    --dump_kit_config(string.format("%s%d-%s-gk-config.txt", ctx.sider_dir, ctx.away_team, 0), g_away[2])
    return hi, ai
end

function m.make_key(ctx, filename)
    --log("wants: " .. filename)
    for patt,attr in pairs(patterns) do
        local fkey = string.match(filename, patt)
        if fkey then
            local key = kfile_remap[fkey]
            if key then
                log(string.format("fkey: {%s}, key: {%s}", fkey, key))
            end
            return key
        end
    end
    -- no key for this file
    return ""
end

function m.get_filepath(ctx, filename, key)
    if key and key ~= "" then
        return kroot .. key
    end
end

function m.key_down(ctx, vkey)
    if vkey == 0x30 then
        kmap = load_map(kroot .. "\\map.txt")
        log("Reloaded map from: " .. kroot .. "\\map.txt")

    elseif vkey == 0x39 then -- player/goalkeeper mode toggle
        if is_gk_mode then
            -- try to switch to players mode
            g_home = home_kits and home_gk_kits[home_next_gk_kit]
            g_away = away_kits and away_gk_kits[away_next_gk_kit]

            -- home: update cfg
            local curr = p_home
            if curr and curr[2] then
                local cfg = table_copy(curr[2])
                update_kit_config(ctx.home_team, home_next_kit, curr[1], cfg)
                -- trigger refresh
                local home_kit_id = ctx.kits.get_current_kit_id(0)
                ctx.kits.set(ctx.home_team, home_kit_id, cfg, 0)
                ctx.kits.refresh(0)
                is_gk_mode = false
            end
            -- away: update cfg
            local curr = p_away
            if curr and curr[2] then
                local cfg = table_copy(curr[2])
                update_kit_config(ctx.away_team, away_next_kit, curr[1], cfg)
                -- trigger refresh
                local away_kit_id = ctx.kits.get_current_kit_id(1)
                ctx.kits.set(ctx.away_team, away_kit_id, cfg, 1)
                ctx.kits.refresh(1)
                is_gk_mode = false
            end
        else
            -- try to switch to goalkeepers mode
            p_home = home_kits and home_kits[home_next_kit]
            p_away = away_kits and away_kits[away_next_kit]

            -- home: update cfg
            local curr = g_home
            if curr and curr[2] then
                -- we have a home GK kit
                local cfg = table_copy(curr[2])
                update_gk_kit_config(ctx.home_team, home_next_gk_kit, curr[1], cfg)
                -- update kit
                ctx.kits.set_gk(ctx.home_team, cfg)
                -- trigger refresh
                local home_kit_id = ctx.kits.get_current_kit_id(0)
                ctx.kits.set(ctx.home_team, home_kit_id, cfg)
                ctx.kits.refresh(0)
                is_gk_mode = true
            end
            -- away: update cfg
            local curr = g_away
            if curr and curr[2] then
                -- we have an away GK kit
                local cfg = table_copy(curr[2])
                update_gk_kit_config(ctx.away_team, away_next_gk_kit, curr[1], cfg)
                -- update kit
                ctx.kits.set_gk(ctx.away_team, cfg)
                -- trigger refresh
                local away_kit_id = ctx.kits.get_current_kit_id(1)
                ctx.kits.set(ctx.away_team, away_kit_id, cfg)
                ctx.kits.refresh(1)
                is_gk_mode = true
            end
        end

    elseif vkey == 0x36 then -- next home kit
        if not home_kits or not home_gk_kits then
            prep_home_team(ctx)
        end
        if not is_gk_mode then
            -- players
            if home_kits and #home_kits > 0 then
                -- advance the iter
                home_next_kit = (home_next_kit % #home_kits) + 1
                log("home_next_kit: " .. home_next_kit)
                -- update cfg
                local curr = home_kits[home_next_kit]
                local cfg = table_copy(curr[2])
                update_kit_config(ctx.home_team, home_next_kit, curr[1], cfg)
                -- trigger refresh
                local kit_id = ctx.kits.get_current_kit_id(0)
                ctx.kits.set(ctx.home_team, kit_id, cfg, 0)
                ctx.kits.refresh(0)
                p_home = curr
            end
        else
            -- goalkeepers
            if home_gk_kits and #home_gk_kits > 0 then
                -- advance the iter
                home_next_gk_kit = (home_next_gk_kit % #home_gk_kits) + 1
                log("home_next_gk_kit: " .. home_next_gk_kit)
                -- update cfg
                local curr = home_gk_kits[home_next_gk_kit]
                local cfg = table_copy(curr[2])
                update_gk_kit_config(ctx.home_team, home_next_gk_kit, curr[1], cfg)
                -- trigger refresh
                local kit_id = ctx.kits.get_current_kit_id(0)
                ctx.kits.set(ctx.home_team, kit_id, cfg)
                ctx.kits.refresh(0)
                g_home = curr
            end
        end
    elseif vkey == 0x37 then -- next away kit
        if not away_kits or not away_gk_kits then
            prep_away_team(ctx)
        end
        if not is_gk_mode then
            -- players
            if away_kits and #away_kits > 0 then
                -- advance the iter
                away_next_kit = (away_next_kit % #away_kits) + 1
                log("away_next_kit: " .. away_next_kit)
                -- update cfg
                local curr = away_kits[away_next_kit]
                local cfg = table_copy(curr[2])
                update_kit_config(ctx.away_team, away_next_kit, curr[1], cfg)
                -- trigger refresh
                local kit_id = ctx.kits.get_current_kit_id(1)
                ctx.kits.set(ctx.away_team, kit_id, cfg, 1)
                ctx.kits.refresh(1)
                p_away = curr
            end
        else
            -- goalkeepers
            if away_gk_kits and #away_gk_kits > 0 then
                -- advance the iter
                away_next_gk_kit = (away_next_gk_kit % #away_gk_kits) + 1
                log("away_next_gk_kit: " .. away_next_gk_kit)
                -- update cfg
                local curr = away_gk_kits[away_next_gk_kit]
                local cfg = table_copy(curr[2])
                update_gk_kit_config(ctx.away_team, away_next_gk_kit, curr[1], cfg)
                -- trigger refresh
                local kit_id = ctx.kits.get_current_kit_id(1)
                ctx.kits.set(ctx.away_team, kit_id, cfg)
                ctx.kits.refresh(1)
                g_away = curr
            end
        end
    end
end

function m.finalize_kits(ctx)
    log("finalizing kits ...")
    is_gk_mode = false
    if home_kits and #home_kits > 0 then
        local curr = g_home
        if curr then
            local cfg = table_copy(curr[2])
            update_gk_kit_config(ctx.home_team, home_next_gk_kit, curr[1], cfg)
            ctx.kits.set_gk(ctx.home_team, cfg)
        end
        local curr = p_home
        if curr then
            local kit_id = ctx.kits.get_current_kit_id(0)
            local cfg = table_copy(curr[2])
            update_kit_config(ctx.home_team, home_next_kit, curr[1], cfg)
            ctx.kits.set(ctx.home_team, kit_id, cfg, 0)
            ctx.kits.refresh(0)
        end
    end
    if away_kits and #away_kits > 0 then
        local curr = g_away
        if curr then
            local cfg = table_copy(curr[2])
            update_gk_kit_config(ctx.away_team, away_next_gk_kit, curr[1], cfg)
            ctx.kits.set_gk(ctx.away_team, cfg)
        end
        local curr = p_away
        if curr then
            local kit_id = ctx.kits.get_current_kit_id(1)
            local cfg = table_copy(curr[2])
            update_kit_config(ctx.away_team, away_next_kit, curr[1], cfg)
            ctx.kits.set(ctx.away_team, kit_id, cfg, 1)
            ctx.kits.refresh(1)
        end
    end
end

function m.overlay_on(ctx)
    return string.format("%s: [6] - switch home kit, [7] - switch away kit, [9] - PL/GK, [0] - reload map",
        modes[is_gk_mode and 2 or 1])
end

function m.init(ctx)
    if kroot:sub(1,1) == "." then
        kroot = ctx.sider_dir .. kroot
    end
    kmap = load_map(kroot .. "\\map.txt")
    ctx.register("overlay_on", m.overlay_on)
    ctx.register("key_down", m.key_down)
    ctx.register("set_kits", m.set_kits)
    ctx.register("after_set_conditions", m.finalize_kits)
    ctx.register("livecpk_make_key", m.make_key)
    ctx.register("livecpk_get_filepath", m.get_filepath)
end

return m
