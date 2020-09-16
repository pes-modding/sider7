#ifndef SIDER_KITINFO_H
#define SIDER_KITINFO_H

#include <windows.h>

#include <lua.hpp>

static void str_to_rgb(BYTE *dst, char *src);
static void set_word_bits(void *dst, int value, int bit_from, int bit_to);
static int get_word_bits(void *dst, int bit_from, int bit_to);

void set_kit_info_from_lua_table(lua_State *L, int index, BYTE *dst, BYTE *radar_color, BYTE *shirt_color);
void get_kit_info_to_lua_table(lua_State *L, int index, BYTE *src);

struct KIT_STATUS_INFO {
    BYTE **vtable;
    struct TASK_UNIFORM_IMPL *task_uniform_impl;
    DWORD home_team_id_encoded;
    DWORD away_team_id_encoded;
    BYTE home_player_kit_id;
    BYTE home_player_unknown[3];
    BYTE home_gk_kit_id;
    BYTE home_gk_unknown[3];
    BYTE away_player_kit_id;
    BYTE away_player_unknown[3];
    BYTE away_gk_kit_id;
    BYTE away_gk_unknown[3];
    DWORD home_league_id;
    DWORD away_league_id;
    DWORD unknown2[6];
    BYTE unknown3;
    BYTE is_edit_mode;
    BYTE unknown4[2];
};

struct TASK_UNIFORM_IMPL {
    BYTE **vtable;
    char className[0x10];
    BYTE unknown1[0x28];
    struct KIT_INTERMED_STRUCT *home;
    BYTE *p0;
    BYTE *p1;
    struct KIT_INTERMED_STRUCT *away;
    BYTE *p2;
    BYTE unknown2[8];
    DWORD change_flag;
    DWORD state; // 7-idle, 4-kit reloading
    BYTE unknown3[0x30];
    DWORD home_team_id_encoded;
    DWORD away_team_id_encoded;
    DWORD home_team_id_encoded_again;
    DWORD away_team_id_encoded_again;
    BYTE home_player_kit_id;
    BYTE home_player_unknown[3];
    BYTE home_gk_kit_id;
    BYTE home_gk_unknown[3];
    BYTE away_player_kit_id;
    BYTE away_player_unknown[3];
    BYTE away_gk_kit_id;
    BYTE away_gk_kit_unknown[3];
    BYTE home_player_kit_id_again;
    BYTE home_player_kit_id_again_unknown[3];
    BYTE home_gk_kit_id_again;
    BYTE home_gk_kit_id_again_unknown[3];
    BYTE away_player_kit_id_again;
    BYTE away_player_kit_id_again_unknown[3];
    BYTE away_gk_kit_id_again;
    BYTE away_gk_kit_id_again_unknown[3];
    BYTE unknown5[0x40];
    BYTE home_change_flag1;
    BYTE home_change_flag2;
    BYTE away_change_flag1;
    BYTE away_change_flag2;
};

struct KIT_INTERMED_STRUCT {
    BYTE **vtable;
    BYTE unknown[0x88];
    struct KIT_HELPER_STRUCT *kit_helper;
};

struct KIT_HELPER_STRUCT {
    BYTE **vtable;
    BYTE unknown1[0x54];
    BYTE is_goalkeeper;
    BYTE unknown2[3];
    BYTE unknown3[0x10];
    BYTE unknown4[0x34];
    BYTE unknown5;
    BYTE unknown6;
    BYTE change_flag;
    BYTE unknown7;
};

#endif
