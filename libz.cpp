#define UNICODE

#include <zlib.h>

#include "common.h"
#include "libz.h"
#include "sider.h"

struct PACK_HEADER {
    BYTE magic[3];
    BYTE wesys[5];
    DWORD compressed_size;
    DWORD uncompressed_size;
};

struct PACK {
    PACK_HEADER header;
    BYTE data[1];
};

static int libz_compress(lua_State *L)
{
    size_t len = 0;
    const char *data = luaL_checklstring(L, 1, &len);
    if (!data || len == 0) {
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_pushstring(L, "data cannot be of zero-length");
        return 2;
    }

    uLongf dest_len;
    BYTE* dest;

    dest_len = len*3; // big buffer just in case
    dest = (BYTE*)malloc(dest_len);
    if (!dest) {
        lua_pop(L, lua_gettop(L));
        lua_pushnil(L);
        lua_pushstring(L, "memory allocation error");
        return 2;
    }

    int retval = compress(dest, &dest_len, (const Bytef*)data, len);
    if (retval != Z_OK) {
        lua_pop(L, 1);
        lua_pushnil(L);
        lua_pushfstring(L, "compression failed with error code: %d", retval);
        free(dest);
        return 2;
    }

    lua_pop(L, 1);
    lua_pushlstring(L, (char*)dest, dest_len);
    free(dest);
    return 1;
}

static int libz_uncompress(lua_State *L)
{
    size_t len = 0;
    int uncompressed_len = 0;
    const char *data = luaL_checklstring(L, 1, &len);
    if (lua_gettop(L) > 1) {
        uncompressed_len = luaL_checkint(L, 2);
    }
    if (!data || len == 0) {
        lua_pop(L, lua_gettop(L));
        lua_pushnil(L);
        lua_pushstring(L, "compressed data cannot be of zero-length");
        return 2;
    }

    uLongf dest_len;
    BYTE* dest;

    dest_len = (uncompressed_len > 0) ? uncompressed_len : len*3;
    dest = (BYTE*)malloc(dest_len);
    if (!dest) {
        lua_pop(L, lua_gettop(L));
        lua_pushnil(L);
        lua_pushstring(L, "memory allocation error");
        return 2;
    }

    int retval = uncompress(dest, &dest_len, (const Bytef*)data, len);
    if (retval != Z_OK) {
        lua_pop(L, lua_gettop(L));
        lua_pushnil(L);
        lua_pushfstring(L, "decompression failed with error code: %d", retval);
        free(dest);
        return 2;
    }

    lua_pop(L, lua_gettop(L));
    lua_pushlstring(L, (char*)dest, dest_len);
    free(dest);
    return 1;
}

static int libz_unpack(lua_State *L)
{
    size_t len = 0;
    int uncompressed_len = 0;
    const char *data = luaL_checklstring(L, 1, &len);
    if (!data || len == 0) {
        lua_pop(L, lua_gettop(L));
        lua_pushnil(L);
        lua_pushstring(L, "packed data cannot be of zero-length");
        return 2;
    }

    if (len < 16) {
        // keep passed data on stack
        lua_pushstring(L, "packed data not in Konami WESYS format");
        return 2;
    }

    PACK *pack = (PACK*)data;
    const Bytef* src = (const Bytef*)&pack->data;
    if (memcmp(pack->header.wesys, "WESYS", 5)!=0) {
        // keep passed data on stack
        lua_pushstring(L, "packed data not in Konami WESYS format");
        return 2;
    }

    uLongf dest_len = pack->header.uncompressed_size;
    BYTE* dest;

    dest = (BYTE*)malloc(dest_len);
    if (!dest) {
        lua_pop(L, lua_gettop(L));
        lua_pushnil(L);
        lua_pushstring(L, "memory allocation error");
        return 2;
    }

    int retval = uncompress(dest, &dest_len, src, len);
    if (retval != Z_OK) {
        lua_pop(L, lua_gettop(L));
        lua_pushnil(L);
        lua_pushfstring(L, "decompression failed with error code: %d", retval);
        free(dest);
        return 2;
    }

    lua_pop(L, lua_gettop(L));
    lua_pushlstring(L, (char*)dest, dest_len);
    free(dest);
    return 1;
}

static int libz_pack(lua_State *L)
{
    size_t len = 0;
    const char *data = luaL_checklstring(L, 1, &len);
    if (!data || len == 0) {
        lua_pop(L, lua_gettop(L));
        lua_pushnil(L);
        lua_pushstring(L, "data cannot be of zero-length");
        return 2;
    }

    char magic[] = "\x00\x01\x01"; // default magic
    if (lua_isstring(L, 2)) {
        size_t mlen = 0;
        const char *mdata = luaL_checklstring(L, 1, &mlen);
        if (mlen != 3) {
            lua_pop(L, lua_gettop(L));
            lua_pushnil(L);
            lua_pushstring(L, "magic must be exactly 3 bytes long");
            return 2;
        }
        memcpy(magic, mdata, 3);
    }

    uLongf dest_len;
    BYTE* dest;

    dest_len = len*3; // big buffer just in case
    dest = (BYTE*)malloc(dest_len + sizeof(PACK_HEADER));
    if (!dest) {
        lua_pop(L, lua_gettop(L));
        lua_pushnil(L);
        lua_pushfstring(L, "memory allocation error");
        return 2;
    }
    PACK *pack = (PACK*)dest;
    memcpy(pack->header.magic, magic, 3);
    memcpy(pack->header.wesys, "WESYS", 5);

    int retval = compress((Bytef*)&pack->data, &dest_len, (const Bytef*)data, len);
    if (retval != Z_OK) {
        lua_pop(L, lua_gettop(L));
        lua_pushnil(L);
        lua_pushfstring(L, "compression failed with error code: %d", retval);
        free(dest);
        return 2;
    }

    pack->header.compressed_size = dest_len;
    pack->header.uncompressed_size = len;

    lua_pop(L, lua_gettop(L));
    lua_pushlstring(L, (char*)dest, dest_len + sizeof(PACK_HEADER));
    free(dest);
    return 1;
}

void init_z_lib(lua_State *L)
{
    lua_newtable(L);
    lua_pushstring(L, "compress");
    lua_pushcclosure(L, libz_compress, 0);
    lua_settable(L, -3);
    lua_pushstring(L, "uncompress");
    lua_pushcclosure(L, libz_uncompress, 0);
    lua_settable(L, -3);
    lua_pushstring(L, "pack");
    lua_pushcclosure(L, libz_pack, 0);
    lua_settable(L, -3);
    lua_pushstring(L, "unpack");
    lua_pushcclosure(L, libz_unpack, 0);
    lua_settable(L, -3);
}

