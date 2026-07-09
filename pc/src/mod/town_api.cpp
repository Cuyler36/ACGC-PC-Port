#include "pc_mod_town_api.h"
#include "pc_mod_town_internal.h"
#include "pc_mod_save_internal.h"
#include "pc_mod_lua_internal.h"
#include "lualib.h"

extern "C" {
#include "m_common_data.h"
#include "m_land_h.h"
#include "m_melody.h"
}

void pc_mod_push_town(lua_State* L) {
    PcModTownHandle* handle;

    if (!pc_mod_save_is_ready()) {
        lua_pushnil(L);
        return;
    }

    handle = (PcModTownHandle*)lua_newuserdata(L, sizeof(PcModTownHandle));
    handle->present = 1;
    luaL_getmetatable(L, PC_MOD_TOWN_MT);
    lua_setmetatable(L, -2);
}

PcModTownHandle* pc_mod_check_town(lua_State* L, int idx) {
    return (PcModTownHandle*)luaL_checkudata(L, idx, PC_MOD_TOWN_MT);
}

static void pc_mod_require_town(lua_State* L, int idx) {
    pc_mod_check_town(L, idx);

    if (!pc_mod_save_is_ready()) {
        luaL_error(L, "invalid Town handle");
    }
}

static int l_town_current(lua_State* L) {
    pc_mod_push_town(L);
    return 1;
}

static int l_town_is_ready(lua_State* L) {
    lua_pushboolean(L, pc_mod_save_is_ready());
    return 1;
}

static int l_town_prop_ready(lua_State* L) {
    pc_mod_check_town(L, 1);
    lua_pushboolean(L, pc_mod_save_is_ready());
    return 1;
}

static int l_town_prop_name(lua_State* L) {
    pc_mod_require_town(L, 1);
    pc_mod_push_game_string(L, Save_Get(land_info).name, LAND_NAME_SIZE);
    return 1;
}

static int l_town_prop_id(lua_State* L) {
    pc_mod_require_town(L, 1);
    lua_pushinteger(L, Save_Get(land_info).id);
    return 1;
}

static int l_town_prop_villager_count(lua_State* L) {
    pc_mod_require_town(L, 1);
    lua_pushinteger(L, Save_Get(now_npc_max));
    return 1;
}

static int l_town_prop_fruit(lua_State* L) {
    pc_mod_require_town(L, 1);
    lua_pushinteger(L, Save_Get(fruit));
    return 1;
}

static int l_town_prop_melody_get(lua_State* L) {
    u8 melody[mMld_MELODY_LEN];

    pc_mod_require_town(L, 1);
    mMld_GetMelody(melody);

    lua_newtable(L);
    for (int i = 0; i < mMld_MELODY_LEN; i++) {
        lua_pushinteger(L, melody[i]);
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

static int l_town_prop_melody_set(lua_State* L) {
    u8 melody[mMld_MELODY_LEN];

    pc_mod_require_town(L, 1);
    luaL_checktype(L, 3, LUA_TTABLE);

    for (int i = 0; i < mMld_MELODY_LEN; i++) {
        lua_rawgeti(L, 3, i + 1);
        if (!lua_isnumber(L, -1)) {
            luaL_error(L, "Melody must be a table of 16 numbers");
        }

        lua_Integer note = luaL_checkinteger(L, -1);
        if (note < 0 || note > 0xF) {
            luaL_error(L, "Melody note %d out of range (0-15)", i + 1);
        }

        melody[i] = (u8)note;
        lua_pop(L, 1);
    }

    mMld_SetSaveMelody(melody);
    return 0;
}

static const PcModPropertyDef town_properties[] = {
    { "Ready",          l_town_prop_ready,          nullptr },
    { "Name",           l_town_prop_name,           nullptr },
    { "Id",             l_town_prop_id,             nullptr },
    { "VillagerCount",  l_town_prop_villager_count, nullptr },
    { "Fruit",          l_town_prop_fruit,          nullptr },
    { "Melody",         l_town_prop_melody_get,     l_town_prop_melody_set },
    { nullptr,          nullptr,                    nullptr },
};

static const luaL_Reg town_module[] = {
    { "current", l_town_current },
    { "isReady", l_town_is_ready },
    { nullptr, nullptr },
};

extern "C" void pc_mod_register_town_api(lua_State* L) {
    luaL_newmetatable(L, PC_MOD_TOWN_MT);
    pc_mod_register_userdata_properties(L, town_properties);
    pc_mod_lock_metatable(L);
    lua_pop(L, 1);

    luaL_register(L, "Town", town_module);
    lua_pop(L, 1);
}
