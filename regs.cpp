#include "regs.h"
#include <string>

void registers_to_lua_table(lua_State *L, int stackIndex, REGISTERS *regs) {
    stackIndex--;
    lua_pushlstring(L, (const char*)&regs->rax, 8);
    lua_setfield(L, stackIndex, "rax");
    lua_pushlstring(L, (const char*)&regs->rbx, 8);
    lua_setfield(L, stackIndex, "rbx");
    lua_pushlstring(L, (const char*)&regs->rcx, 8);
    lua_setfield(L, stackIndex, "rcx");
    lua_pushlstring(L, (const char*)&regs->rdx, 8);
    lua_setfield(L, stackIndex, "rdx");
    lua_pushlstring(L, (const char*)&regs->rsi, 8);
    lua_setfield(L, stackIndex, "rsi");
    lua_pushlstring(L, (const char*)&regs->rdi, 8);
    lua_setfield(L, stackIndex, "rdi");
    lua_pushlstring(L, (const char*)&regs->rbp, 8);
    lua_setfield(L, stackIndex, "rbp");
    lua_pushlstring(L, (const char*)&regs->rsp, 8);
    lua_setfield(L, stackIndex, "rsp");
    lua_pushlstring(L, (const char*)&regs->r8, 8);
    lua_setfield(L, stackIndex, "r8");
    lua_pushlstring(L, (const char*)&regs->r9, 8);
    lua_setfield(L, stackIndex, "r9");
    lua_pushlstring(L, (const char*)&regs->r10, 8);
    lua_setfield(L, stackIndex, "r10");
    lua_pushlstring(L, (const char*)&regs->r11, 8);
    lua_setfield(L, stackIndex, "r11");
    lua_pushlstring(L, (const char*)&regs->r12, 8);
    lua_setfield(L, stackIndex, "r12");
    lua_pushlstring(L, (const char*)&regs->r13, 8);
    lua_setfield(L, stackIndex, "r13");
    lua_pushlstring(L, (const char*)&regs->r14, 8);
    lua_setfield(L, stackIndex, "r14");
    lua_pushlstring(L, (const char*)&regs->r15, 8);
    lua_setfield(L, stackIndex, "r15");
    lua_pushlstring(L, (const char*)&regs->rflags, 8);
    lua_setfield(L, stackIndex, "rflags");

    lua_pushlstring(L, (const char*)&regs->xmm0, 16);
    lua_setfield(L, stackIndex, "xmm0");
    lua_pushlstring(L, (const char*)&regs->xmm1, 16);
    lua_setfield(L, stackIndex, "xmm1");
    lua_pushlstring(L, (const char*)&regs->xmm2, 16);
    lua_setfield(L, stackIndex, "xmm2");
    lua_pushlstring(L, (const char*)&regs->xmm3, 16);
    lua_setfield(L, stackIndex, "xmm3");
    lua_pushlstring(L, (const char*)&regs->xmm4, 16);
    lua_setfield(L, stackIndex, "xmm4");
    lua_pushlstring(L, (const char*)&regs->xmm5, 16);
    lua_setfield(L, stackIndex, "xmm5");
    lua_pushlstring(L, (const char*)&regs->xmm6, 16);
    lua_setfield(L, stackIndex, "xmm6");
    lua_pushlstring(L, (const char*)&regs->xmm7, 16);
    lua_setfield(L, stackIndex, "xmm7");
    lua_pushlstring(L, (const char*)&regs->xmm8, 16);
    lua_setfield(L, stackIndex, "xmm8");
    lua_pushlstring(L, (const char*)&regs->xmm9, 16);
    lua_setfield(L, stackIndex, "xmm9");
    lua_pushlstring(L, (const char*)&regs->xmm10, 16);
    lua_setfield(L, stackIndex, "xmm10");
    lua_pushlstring(L, (const char*)&regs->xmm11, 16);
    lua_setfield(L, stackIndex, "xmm11");
    lua_pushlstring(L, (const char*)&regs->xmm12, 16);
    lua_setfield(L, stackIndex, "xmm12");
    lua_pushlstring(L, (const char*)&regs->xmm13, 16);
    lua_setfield(L, stackIndex, "xmm13");
    lua_pushlstring(L, (const char*)&regs->xmm14, 16);
    lua_setfield(L, stackIndex, "xmm14");
    lua_pushlstring(L, (const char*)&regs->xmm15, 16);
    lua_setfield(L, stackIndex, "xmm15");
}

void registers_from_lua_table(lua_State *L, int stackIndex, REGISTERS *regs) {
    lua_getfield(L, stackIndex, "rax");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->rax, luaL_checkstring(L, -1), 8);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "rbx");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->rbx, luaL_checkstring(L, -1), 8);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "rcx");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->rcx, luaL_checkstring(L, -1), 8);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "rdx");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->rdx, luaL_checkstring(L, -1), 8);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "rsi");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->rsi, luaL_checkstring(L, -1), 8);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "rdi");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->rdi, luaL_checkstring(L, -1), 8);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "rbp");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->rbp, luaL_checkstring(L, -1), 8);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "r8");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->r8, luaL_checkstring(L, -1), 8);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "r9");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->r9, luaL_checkstring(L, -1), 8);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "r10");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->r10, luaL_checkstring(L, -1), 8);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "r11");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->r11, luaL_checkstring(L, -1), 8);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "r12");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->r12, luaL_checkstring(L, -1), 8);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "r13");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->r13, luaL_checkstring(L, -1), 8);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "r14");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->r14, luaL_checkstring(L, -1), 8);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "r15");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->r15, luaL_checkstring(L, -1), 8);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "rflags");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->rflags, luaL_checkstring(L, -1), 8);
    }
    lua_pop(L, 1);

    // xmm
    lua_getfield(L, stackIndex, "xmm0");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->xmm0, luaL_checkstring(L, -1), 16);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "xmm1");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->xmm1, luaL_checkstring(L, -1), 16);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "xmm2");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->xmm2, luaL_checkstring(L, -1), 16);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "xmm3");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->xmm3, luaL_checkstring(L, -1), 16);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "xmm4");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->xmm4, luaL_checkstring(L, -1), 16);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "xmm5");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->xmm5, luaL_checkstring(L, -1), 16);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "xmm6");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->xmm6, luaL_checkstring(L, -1), 16);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "xmm7");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->xmm7, luaL_checkstring(L, -1), 16);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "xmm8");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->xmm8, luaL_checkstring(L, -1), 16);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "xmm9");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->xmm9, luaL_checkstring(L, -1), 16);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "xmm10");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->xmm10, luaL_checkstring(L, -1), 16);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "xmm11");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->xmm11, luaL_checkstring(L, -1), 16);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "xmm12");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->xmm12, luaL_checkstring(L, -1), 16);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "xmm13");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->xmm13, luaL_checkstring(L, -1), 16);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "xmm14");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->xmm14, luaL_checkstring(L, -1), 16);
    }
    lua_pop(L, 1);
    lua_getfield(L, stackIndex, "xmm15");
    if (!lua_isnil(L, -1)) {
        memcpy(&regs->xmm15, luaL_checkstring(L, -1), 16);
    }
    lua_pop(L, 1);
}
