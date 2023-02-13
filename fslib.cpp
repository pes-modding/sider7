#include <windows.h>
#include "lua.hpp"
#include "sider.h"
#include "fslib.h"
#include "fslib_lua.h"

void init_fslib(lua_State *L)
{
    int r = luaL_loadbuffer(L, fslib_lua, strlen(fslib_lua), "fs");
    if (r != 0) {
        const char *err = lua_tostring(L, -1);
        logu_("PROBLEM loading fs library: %s. "
              "Skipping it\n", err);
        lua_pop(L, 1);
        return;
    }

    // run the module
    if (lua_pcall(L, 0, 1, 0) != 0) {
        const char *err = lua_tostring(L, -1);
        logu_("PROBLEM initializing fs library: %s. "
              "Skipping it\n", err);
        lua_pop(L, 1);
        return;
    }
}

