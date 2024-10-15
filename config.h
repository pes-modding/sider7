#ifndef SIDER_CONFIG_H
#define SIDER_CONFIG_H

#define UNICODE

#include <windows.h>
#include <string>
#include <vector>

#include "common.h"

using namespace std;

#include "lua.hpp"
#include "lauxlib.h"
#include "lualib.h"

#define DEFAULT_OVERLAY_TEXT_COLOR 0xc080ff80
#define DEFAULT_OVERLAY_BACKGROUND_COLOR 0x80102010
#define DEFAULT_OVERLAY_IMAGE_ALPHA_MAX 1.0f
#define DEFAULT_OVERLAY_FONT L"Arial"
#define DEFAULT_OVERLAY_FONT_SIZE 0
#define DEFAULT_OVERLAY_LOCATION 0
#define DEFAULT_OVERLAY_VKEY_TOGGLE 0x20
#define DEFAULT_OVERLAY_VKEY_NEXT_MODULE 0x31
#define DEFAULT_OVERLAY_VKEY_PREV_MODULE 0xc0
#define DEFAULT_OVERLAY_TOGGLE_SOUND L"toggle.wav"
#define DEFAULT_OVERLAY_TOGGLE_SOUND_VOLUME 1.0f
#define DEFAULT_VKEY_RELOAD_1 0x10 //Shift
#define DEFAULT_VKEY_RELOAD_2 0x52 //R
#define DEFAULT_GAMEPAD_STICK_SENSITIVITY 0.6
#define DEFAULT_GAMEPAD_POLL_INTERVAL_MSEC 200
#define DEFAULT_GAMEPAD_OVERLAY_POLL_INTERVAL_MSEC 32
#define DEFAULT_CACHE_SIZE 32
#define DEFAULT_MATCH_STATS_ENABLED false
#define DEFAULT_GLOBAL_BLOCK_INPUT false
#define DEFAULT_HOOK_ALL_KEYBOARDS true
#define DEFAULT_CONTROLLER_INPUT_BLOCKING_ENABLED true

extern wchar_t sider_dir[MAX_PATH];

class config_t {
public:
    int _debug;
    int _priority_class;
    int _skip_checks;
    bool _livecpk_enabled;
    bool _lookup_cache_enabled;
    bool _lua_enabled;
    bool _jit_enabled;
    bool _luajit_extensions_enabled;
    bool _dummify_uniparam;
    vector<wstring> _lua_extra_globals;
    int _lua_gc_opt;
    int _dll_mapping_option;
    int _key_cache_ttl_sec;
    int _rewrite_cache_ttl_sec;
    int _cache_size;
    wstring _section_name;
    vector<wstring> _cpk_roots;
    wstring _start_game;
    vector<wstring> _exe_names;
    vector<wstring> _module_names;
    bool _close_sider_on_exit;
    bool _start_minimized;
    bool _free_side_select;
    bool _match_stats_enabled;
    bool _global_block_input;
    bool _overlay_enabled;
    bool _overlay_on_from_start;
    wstring _overlay_font;
    wstring _overlay_toggle_sound;
    float _overlay_toggle_sound_volume;
    DWORD _overlay_text_color;
    DWORD _overlay_background_color;
    float _overlay_image_alpha_max;
    int _overlay_location;
    int _overlay_font_size;
    int _overlay_vkey_toggle;
    int _overlay_vkey_next_module;
    int _overlay_vkey_prev_module;
    int _vkey_reload_1;
    int _vkey_reload_2;
    int _num_minutes;
    BYTE *_hp_at_read_file;
    BYTE *_hp_at_get_size;
    BYTE *_hp_at_extend_cpk;
    BYTE *_hp_at_mem_copy;
    BYTE *_hp_at_lookup_file;
    BYTE *_hp_at_set_team_id;
    BYTE *_hp_at_set_settings;
    BYTE *_hp_at_trophy_check;
    BYTE *_hp_at_trophy_check2;
    BYTE *_hp_at_context_reset;
    BYTE *_hp_at_trophy_table;
    BYTE *_hp_at_ball_name;
    BYTE *_hp_at_game_lite;
    BYTE *_hp_at_stadium_name;
    BYTE *_hp_at_def_stadium_name;
    BYTE *_hp_at_set_stadium_choice;
    BYTE *_hp_at_check_kit_choice;
    BYTE *_hp_at_get_uniparam;
    BYTE *_hp_at_data_ready;
    BYTE *_hp_at_call_to_move;
    BYTE *_hp_at_kit_status;
    BYTE *_hp_at_set_team_for_kits;
    BYTE *_hp_at_clear_team_for_kits;
    BYTE *_hp_at_uniparam_loaded;
    BYTE *_hp_at_copy_clock;
    BYTE *_hp_at_clear_sc;
    BYTE *_hp_at_xinput;
    BYTE *_hp_at_set_edit_team_id;

    BYTE *_hp_at_set_min_time;
    BYTE *_hp_at_set_max_time;
    BYTE *_hp_at_call_set_minutes;
    BYTE *_hp_at_sider;

    BYTE *_hp_at_dxgi;

    bool _hook_set_team_id;
    bool _hook_set_settings;
    bool _hook_context_reset;
    bool _hook_trophy_check;
    bool _hook_trophy_table;
    bool _hook_all_keyboards;
    bool _controller_input_blocking_enabled;

    ~config_t() {}
    config_t(const wstring& section_name, const wchar_t* config_ini) :
                 _section_name(section_name),
                 _debug(0),
                 _skip_checks(0),
                 _priority_class(0),
                 _livecpk_enabled(false),
                 _lookup_cache_enabled(true),
                 _lua_enabled(true),
                 _jit_enabled(true),
                 _luajit_extensions_enabled(false),
                 _dummify_uniparam(true),
                 _lua_gc_opt(LUA_GCSTEP),
                 _close_sider_on_exit(false),
                 _start_minimized(false),
                 _free_side_select(false),
                 _match_stats_enabled(DEFAULT_MATCH_STATS_ENABLED),
                 _global_block_input(DEFAULT_GLOBAL_BLOCK_INPUT),
                 _start_game(L""),
                 _overlay_enabled(false),
                 _overlay_on_from_start(false),
                 _overlay_font(DEFAULT_OVERLAY_FONT),
                 _overlay_toggle_sound(DEFAULT_OVERLAY_TOGGLE_SOUND),
                 _overlay_toggle_sound_volume(DEFAULT_OVERLAY_TOGGLE_SOUND_VOLUME),
                 _overlay_text_color(DEFAULT_OVERLAY_TEXT_COLOR),
                 _overlay_background_color(DEFAULT_OVERLAY_BACKGROUND_COLOR),
                 _overlay_image_alpha_max(DEFAULT_OVERLAY_IMAGE_ALPHA_MAX),
                 _overlay_font_size(DEFAULT_OVERLAY_FONT_SIZE),
                 _overlay_location(DEFAULT_OVERLAY_LOCATION),
                 _overlay_vkey_toggle(DEFAULT_OVERLAY_VKEY_TOGGLE),
                 _overlay_vkey_next_module(DEFAULT_OVERLAY_VKEY_NEXT_MODULE),
                 _overlay_vkey_prev_module(DEFAULT_OVERLAY_VKEY_PREV_MODULE),
                 _vkey_reload_1(DEFAULT_VKEY_RELOAD_1),
                 _vkey_reload_2(DEFAULT_VKEY_RELOAD_2),
                 _key_cache_ttl_sec(10),
                 _rewrite_cache_ttl_sec(10),
                 _cache_size(DEFAULT_CACHE_SIZE),
                 _hp_at_read_file(NULL),
                 _hp_at_get_size(NULL),
                 _hp_at_extend_cpk(NULL),
                 _hp_at_mem_copy(NULL),
                 _hp_at_lookup_file(NULL),
                 _hp_at_set_team_id(NULL),
                 _hp_at_set_settings(NULL),
                 _hp_at_trophy_check(NULL),
                 _hp_at_trophy_check2(NULL),
                 _hp_at_context_reset(NULL),
                 _hp_at_set_stadium_choice(NULL),
                 _hp_at_check_kit_choice(NULL),
                 _hp_at_get_uniparam(NULL),
                 _hp_at_data_ready(NULL),
                 _hp_at_call_to_move(NULL),
                 _hp_at_kit_status(NULL),
                 _hp_at_set_team_for_kits(NULL),
                 _hp_at_clear_team_for_kits(NULL),
                 _hp_at_uniparam_loaded(NULL),
                 _hp_at_copy_clock(NULL),
                 _hp_at_clear_sc(NULL),
                 _hp_at_xinput(NULL),
                 _hp_at_set_edit_team_id(NULL),
                 _hp_at_set_min_time(NULL),
                 _hp_at_set_max_time(NULL),
                 _hp_at_call_set_minutes(NULL),
                 _hp_at_sider(NULL),
                 _hp_at_trophy_table(NULL),
                 _hp_at_ball_name(NULL),
                 _hp_at_game_lite(NULL),
                 _hp_at_stadium_name(NULL),
                 _hp_at_def_stadium_name(NULL),
                 _hp_at_dxgi(NULL),
                 _hook_set_team_id(true),
                 _hook_set_settings(true),
                 _hook_context_reset(true),
                 _hook_trophy_check(true),
                 _hook_trophy_table(true),
                 _hook_all_keyboards(DEFAULT_HOOK_ALL_KEYBOARDS),
                 _controller_input_blocking_enabled(DEFAULT_CONTROLLER_INPUT_BLOCKING_ENABLED),
                 _num_minutes(0)
    {
        wchar_t settings[32767];
        RtlZeroMemory(settings, sizeof(settings));
        if (GetPrivateProfileSection(_section_name.c_str(),
            settings, sizeof(settings)/sizeof(wchar_t), config_ini)==0) {
            // no ini-file, or no "[sider]" section
            return;
        }

        wchar_t* p = settings;
        while (*p) {
            wstring pair(p);
            wstring key(pair.substr(0, pair.find(L"=")));
            wstring value(pair.substr(pair.find(L"=")+1));
            string_strip_quotes(value);

            if (wcscmp(L"exe.name", key.c_str())==0) {
                _exe_names.push_back(value);
            }
            else if (wcscmp(L"lua.module", key.c_str())==0) {
                _module_names.push_back(value);
            }
            else if (wcscmp(L"overlay.font", key.c_str())==0) {
                _overlay_font = value;
            }
            else if (wcscmp(L"overlay.toggle.sound", key.c_str())==0) {
                _overlay_toggle_sound = value;
            }
            else if (wcscmp(L"start.game", key.c_str())==0) {
                _start_game = value;
            }
            else if (wcscmp(L"overlay.text-color", key.c_str())==0) {
                if (value.size() >= 8) {
                    DWORD c,v;
                    // red
                    if (swscanf(value.substr(0,2).c_str(), L"%x", &c)==1) { v = c; }
                    else if (swscanf(value.substr(0,2).c_str(), L"%X", &c)==1) { v = c; }
                    // green
                    if (swscanf(value.substr(2,2).c_str(), L"%x", &c)==1) { v = v | (c << 8); }
                    else if (swscanf(value.substr(2,2).c_str(), L"%X", &c)==1) { v = v | (c << 8); }
                    // blue
                    if (swscanf(value.substr(4,2).c_str(), L"%x", &c)==1) { v = v | (c << 16); }
                    else if (swscanf(value.substr(4,2).c_str(), L"%X", &c)==1) { v = v | (c << 16); }
                    // alpha
                    if (swscanf(value.substr(6,2).c_str(), L"%x", &c)==1) { v = v | (c << 24); }
                    else if (swscanf(value.substr(6,2).c_str(), L"%X", &c)==1) { v = v | (c << 24); }
                    _overlay_text_color = v;
                }
            }
            else if (wcscmp(L"overlay.background-color", key.c_str())==0) {
                if (value.size() >= 8) {
                    DWORD c,v;
                    // red
                    if (swscanf(value.substr(0,2).c_str(), L"%x", &c)==1) { v = c; }
                    else if (swscanf(value.substr(0,2).c_str(), L"%X", &c)==1) { v = c; }
                    // green
                    if (swscanf(value.substr(2,2).c_str(), L"%x", &c)==1) { v = v | (c << 8); }
                    else if (swscanf(value.substr(2,2).c_str(), L"%X", &c)==1) { v = v | (c << 8); }
                    // blue
                    if (swscanf(value.substr(4,2).c_str(), L"%x", &c)==1) { v = v | (c << 16); }
                    else if (swscanf(value.substr(4,2).c_str(), L"%X", &c)==1) { v = v | (c << 16); }
                    // alpha
                    if (swscanf(value.substr(6,2).c_str(), L"%x", &c)==1) { v = v | (c << 24); }
                    else if (swscanf(value.substr(6,2).c_str(), L"%X", &c)==1) { v = v | (c << 24); }
                    _overlay_background_color = v;
                }
            }
            else if (wcscmp(L"overlay.image-alpha-max", key.c_str())==0) {
                float alpha = 1.0f;
                if (swscanf(value.c_str(),L"%f",&alpha)==1) {
                    _overlay_image_alpha_max = min(1.0f, max(0.0f, alpha));
                }
            }
            else if (wcscmp(L"overlay.toggle.sound-volume", key.c_str())==0) {
                float volume;
                if (swscanf(value.c_str(),L"%f",&volume)==1) {
                    _overlay_toggle_sound_volume = min(1.0f, max(0.0f, volume));
                }
            }
            else if (wcscmp(L"overlay.location", key.c_str())==0) {
                _overlay_location = 0;
                if (value == L"bottom") {
                    _overlay_location = 1;
                }
            }
            else if (wcscmp(L"overlay.vkey.toggle", key.c_str())==0) {
                int v;
                if (swscanf(value.c_str(), L"%x", &v)==1) {
                    _overlay_vkey_toggle = v;
                }
            }
            else if (wcscmp(L"overlay.vkey.next-module", key.c_str())==0) {
                int v;
                if (swscanf(value.c_str(), L"%x", &v)==1) {
                    _overlay_vkey_next_module = v;
                }
            }
            else if (wcscmp(L"overlay.vkey.prev-module", key.c_str())==0) {
                int v;
                if (swscanf(value.c_str(), L"%x", &v)==1) {
                    _overlay_vkey_prev_module = v;
                }
            }
            else if (wcscmp(L"vkey.reload-1", key.c_str())==0) {
                int v;
                if (swscanf(value.c_str(), L"%x", &v)==1) {
                    _vkey_reload_1 = v;
                }
            }
            else if (wcscmp(L"vkey.reload-2", key.c_str())==0) {
                int v;
                if (swscanf(value.c_str(), L"%x", &v)==1) {
                    _vkey_reload_2 = v;
                }
            }
            else if (wcscmp(L"game.priority.class", key.c_str())==0) {
                int v;
                if (value == L"above_normal") {
                    _priority_class = 0x8000;
                }
                else if (value == L"below_normal") {
                    _priority_class = 0x4000;
                }
                else if (value == L"high") {
                    _priority_class = 0x80;
                }
                else if (value == L"idle") {
                    _priority_class = 0x40;
                }
                else if (value == L"normal") {
                    _priority_class = 0x20;
                }
                else if (value == L"background_begin") {
                    _priority_class = 0x00100000;
                }
                else if (value == L"background_end") {
                    _priority_class = 0x00200000;
                }
                else if (value == L"realtime") {
                    _priority_class = 0x100;
                }
                else if (swscanf(value.c_str(), L"%x", &v)==1) {
                    _priority_class = v;
                }
            }
            else if (wcscmp(L"lua.extra-globals", key.c_str())==0) {
                bool done(false);
                int start = 0, end = 0;
                while (!done) {
                    end = value.find(L",", start);
                    done = (end == string::npos);

                    wstring name((done) ?
                        value.substr(start) :
                        value.substr(start, end - start));
                    string_strip_quotes(name);
                    if (!name.empty()) {
                        _lua_extra_globals.push_back(name);
                    }
                    start = end + 1;
                }
            }
            else if (wcscmp(L"lua.gc.opt", key.c_str())==0) {
                _lua_gc_opt = LUA_GCSTEP;
                if (value == L"collect") {
                    _lua_gc_opt = LUA_GCCOLLECT;
                }
            }
            else if (wcscmp(L"cpk.root", key.c_str())==0) {
                if (value[value.size()-1] != L'\\') {
                    value += L'\\';
                }
                // handle relative roots
                if (value[0]==L'.') {
                    wstring rel(value);
                    value = sider_dir;
                    value += rel;
                }
                _cpk_roots.push_back(value);
            }

            p += wcslen(p) + 1;
        }

        _debug = GetPrivateProfileInt(_section_name.c_str(),
            L"debug", _debug,
            config_ini);

        _skip_checks = GetPrivateProfileInt(_section_name.c_str(),
            L"skip.checks", _skip_checks,
            config_ini);

        _close_sider_on_exit = GetPrivateProfileInt(_section_name.c_str(),
            L"close.on.exit", _close_sider_on_exit,
            config_ini);

        _start_minimized = GetPrivateProfileInt(_section_name.c_str(),
            L"start.minimized", _start_minimized,
            config_ini);

        _free_side_select = GetPrivateProfileInt(_section_name.c_str(),
            L"free.side.select", _free_side_select,
            config_ini);

        _match_stats_enabled = GetPrivateProfileInt(_section_name.c_str(),
            L"match-stats.enabled", _match_stats_enabled,
            config_ini);

        _global_block_input = GetPrivateProfileInt(_section_name.c_str(),
            L"overlay.block-input-when-on", _global_block_input,
            config_ini);

        _overlay_enabled = GetPrivateProfileInt(_section_name.c_str(),
            L"overlay.enabled", _overlay_enabled,
            config_ini);

        _overlay_on_from_start = GetPrivateProfileInt(_section_name.c_str(),
            L"overlay.on-from-start", _overlay_on_from_start,
            config_ini);

        _overlay_font_size = GetPrivateProfileInt(_section_name.c_str(),
            L"overlay.font-size", _overlay_font_size,
            config_ini);

        _livecpk_enabled = GetPrivateProfileInt(_section_name.c_str(),
            L"livecpk.enabled", _livecpk_enabled,
            config_ini);

        _lookup_cache_enabled = GetPrivateProfileInt(_section_name.c_str(),
            L"lookup-cache.enabled", _lookup_cache_enabled,
            config_ini);

        _lua_enabled = GetPrivateProfileInt(_section_name.c_str(),
            L"lua.enabled", _lua_enabled,
            config_ini);

        _jit_enabled = GetPrivateProfileInt(_section_name.c_str(),
            L"jit.enabled", _jit_enabled,
            config_ini);

        _luajit_extensions_enabled = GetPrivateProfileInt(_section_name.c_str(),
            L"luajit.ext.enabled", _luajit_extensions_enabled,
            config_ini);

        _dummify_uniparam = GetPrivateProfileInt(_section_name.c_str(),
            L"dummify.uniparam", _dummify_uniparam,
            config_ini);

        _key_cache_ttl_sec = GetPrivateProfileInt(_section_name.c_str(),
            L"key-cache.ttl-sec", _key_cache_ttl_sec,
            config_ini);

        _rewrite_cache_ttl_sec = GetPrivateProfileInt(_section_name.c_str(),
            L"rewrite-cache.ttl-sec", _rewrite_cache_ttl_sec,
            config_ini);

        _cache_size = GetPrivateProfileInt(_section_name.c_str(),
            L"cache.size", _cache_size,
            config_ini);
        if (_cache_size < 1) {
            _cache_size = DEFAULT_CACHE_SIZE;
        }
        else {
            // need to ensure power-of-2 size
            size_t v = 1;
            while ((v << 1) <= _cache_size) {
                v = v << 1;
            }
            _cache_size = v;
        }

        _num_minutes = GetPrivateProfileInt(_section_name.c_str(),
            L"match.minutes", _num_minutes,
            config_ini);

        _hook_set_team_id = GetPrivateProfileInt(_section_name.c_str(),
            L"hook.set-team-id", _hook_set_team_id,
            config_ini);

        _hook_set_settings = GetPrivateProfileInt(_section_name.c_str(),
            L"hook.set-settings", _hook_set_settings,
            config_ini);

        _hook_context_reset = GetPrivateProfileInt(_section_name.c_str(),
            L"hook.context-reset", _hook_context_reset,
            config_ini);

        _hook_trophy_table = GetPrivateProfileInt(_section_name.c_str(),
            L"hook.trophy-table", _hook_trophy_table,
            config_ini);

        _hook_trophy_check = GetPrivateProfileInt(_section_name.c_str(),
            L"hook.trophy-check", _hook_trophy_check,
            config_ini);

        _hook_all_keyboards = GetPrivateProfileInt(_section_name.c_str(),
            L"hook.all-keyboards", _hook_all_keyboards,
            config_ini);

        _controller_input_blocking_enabled = GetPrivateProfileInt(_section_name.c_str(),
            L"overlay.gamepad-input-blocking.enabled", _controller_input_blocking_enabled,
            config_ini);
    }
};

#endif
