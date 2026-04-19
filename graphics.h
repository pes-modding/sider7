#ifndef _SIDER_GRAPHICS_H
#define _SIDER_GRAPHICS_H

#include "lua.hpp"
#include "lauxlib.h"
#include "lualib.h"

void gfx_init(lua_State *L);
int gfx_image(lua_State *L);
int gfx_sprite(lua_State *L);

#endif
