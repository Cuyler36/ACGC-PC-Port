#include "pc_mod_api.h"
#include "pc_mod_actor_api.h"
#include "pc_mod_player_api.h"
#include "pc_mod_item_api.h"
#include "pc_mod_save_api.h"
#include "lua.h"
#include "lualib.h"
#include <cstdio>

static const char kCallbackRoot[] = "pc_mod_callbacks";
static lua_State* g_mod_L;

static void push_event_table(lua_State* L, const char* event) {
    lua_getfield(L, LUA_REGISTRYINDEX, kCallbackRoot);

    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setfield(L, LUA_REGISTRYINDEX, kCallbackRoot);
    }

    lua_getfield(L, -1, event);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        lua_newtable(L);
        lua_pushvalue(L, -1);
        lua_setfield(L, -3, event);
    }

    lua_remove(L, -2);
}

static int l_bind_event(lua_State* L, const char* event) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    if (!g_mod_L) {
        luaL_error(L, "mod API not initialized");
    }

    lua_State* R = g_mod_L;
    push_event_table(R, event);
    lua_pushvalue(L, 1);
    lua_xmove(L, R, 1);
    lua_rawseti(R, -2, (int)lua_objlen(R, -2) + 1);
    lua_pop(R, 1);
    return 0;
}

#define BIND_FN(name) \
    static int l_bind_##name(lua_State* L) { return l_bind_event(L, #name); }

BIND_FN(load)
BIND_FN(init)
BIND_FN(beginframe)
BIND_FN(endframe)
BIND_FN(premove)
BIND_FN(postmove)
BIND_FN(predraw)
BIND_FN(postdraw)

static const luaL_Reg bindlib[] = {
    { "load",       l_bind_load },
    { "init",       l_bind_init },
    { "beginframe", l_bind_beginframe },
    { "endframe",   l_bind_endframe },
    { "premove",    l_bind_premove },
    { "postmove",   l_bind_postmove },
    { "predraw",    l_bind_predraw },
    { "postdraw",   l_bind_postdraw },
    { nullptr, nullptr },
};

extern "C" {

void pc_mod_register_api(lua_State* L) {
    g_mod_L = L;

    luaL_register(L, "bind", bindlib);
    lua_pop(L, 1);
    pc_mod_register_actor_api(L);
    pc_mod_register_player_api(L);
    pc_mod_register_item_api(L);
    pc_mod_register_save_api(L);
}

void pc_mod_dispatch(const char* event) {
    if (!g_mod_L) return;

    lua_State* L = g_mod_L;
    lua_settop(L, 0);
    push_event_table(L, event);

    int n = lua_objlen(L, -1);

    for (int i = 1; i <= n; i++) {
        lua_State* thread = lua_newthread(L);
        luaL_sandboxthread(thread);

        lua_rawgeti(L, -2, i);
        lua_xmove(L, thread, 1);

        if (lua_pcall(thread, 0, 0, 0) != LUA_OK) {
            fprintf(stderr, "[mod] bind.%s err: %s\n", event, lua_tostring(thread, -1));
            lua_pop(thread, 1);
        }
        lua_pop(L, 1); // thread
    }
    lua_pop(L, 1); // event table
}

void pc_mod_on_load(void)       { pc_mod_dispatch("load"); }
void pc_mod_on_init(void)       { pc_mod_dispatch("init"); }
void pc_mod_on_begin_frame(void){ pc_mod_dispatch("beginframe"); }
void pc_mod_on_end_frame(void)  { pc_mod_dispatch("endframe"); }
void pc_mod_on_pre_move(void)   { pc_mod_dispatch("premove"); }
void pc_mod_on_post_move(void)  { pc_mod_dispatch("postmove"); }
void pc_mod_on_pre_draw(void)   { pc_mod_dispatch("predraw"); }
void pc_mod_on_post_draw(void)  { pc_mod_dispatch("postdraw"); }

}
