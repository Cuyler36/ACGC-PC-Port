#include "pc_mod_player_api.h"
#include "pc_mod_actor_internal.h"
#include "lualib.h"

extern "C" {
#include "m_player.h"
#include "m_player_lib.h"
}

static int l_player_get(lua_State* L) {
    GAME_PLAY* play = pc_mod_try_get_play();
    if (!play) {
        lua_pushnil(L);
        return 1;
    }

    pc_mod_push_actor(L, (ACTOR*)get_player_actor_withoutCheck(play));
    return 1;
}

static int l_player_get_item_kind(lua_State* L) {
    GAME_PLAY* play = pc_mod_try_get_play();
    PLAYER_ACTOR* player;

    if (!play) {
        lua_pushnil(L);
        return 1;
    }

    player = get_player_actor_withoutCheck(play);
    if (!player) {
        lua_pushnil(L);
        return 1;
    }

    lua_pushinteger(L, player->item_kind);
    return 1;
}

static const luaL_Reg playerlib[] = {
    { "get",           l_player_get },
    { "get_item_kind", l_player_get_item_kind },
    { nullptr, nullptr },
};

extern "C" void pc_mod_register_player_api(lua_State* L) {
    luaL_register(L, "player", playerlib);
    lua_pop(L, 1);
}
