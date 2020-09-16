#include <windows.h>
#include "lua.hpp"
#include "sider.h"
#include "memlib.h"
#include "memlib_lua.h"

void init_memlib(lua_State *L)
{
    int r = luaL_loadbuffer(L, memlib_lua, strlen(memlib_lua), "memory");
    if (r != 0) {
        const char *err = lua_tostring(L, -1);
        logu_("PROBLEM loading memory library: %s. "
              "Skipping it\n", err);
        lua_pop(L, 1);
        return;
    }

    // run the module
    if (lua_pcall(L, 0, 1, 0) != 0) {
        const char *err = lua_tostring(L, -1);
        logu_("PROBLEM initializing memory library: %s. "
              "Skipping it\n", err);
        lua_pop(L, 1);
        return;
    }
}

