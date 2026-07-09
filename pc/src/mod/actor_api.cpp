#include "pc_mod_actor_api.h"
#include "pc_mod_actor_internal.h"
#include "pc_mod_lua_internal.h"
#include "lualib.h"
#include <cstdio>
#include <cstring>

extern "C" {
#include "m_play.h"
#include "m_actor.h"
#include "m_lib.h"
#include "m_name_table.h"
#include "m_npc.h"
#include "m_player_lib.h"
#include "sys_math.h"
}

GAME_PLAY* pc_mod_try_get_play(void) {
    if (!gamePT || gamePT->exec != (void (*)(GAME*))play_main) {
        return nullptr;
    }
    return (GAME_PLAY*)gamePT;
}

int pc_mod_actor_is_live(ACTOR* actor, GAME_PLAY* play) {
    int part;
    ACTOR* it;

    if (!actor || !play) {
        return 0;
    }

    part = actor->part;
    if (part < 0 || part >= ACTOR_PART_NUM) {
        return 0;
    }

    for (it = play->actor_info.list[part].actor; it != nullptr; it = it->next_actor) {
        if (it == actor) {
            return 1;
        }
    }

    return 0;
}

void pc_mod_push_actor(lua_State* L, ACTOR* actor) {
    PcModActorHandle* handle;

    if (!actor) {
        lua_pushnil(L);
        return;
    }

    handle = (PcModActorHandle*)lua_newuserdata(L, sizeof(PcModActorHandle));
    handle->actor = actor;
    luaL_getmetatable(L, PC_MOD_ACTOR_MT);
    lua_setmetatable(L, -2);
}

ACTOR* pc_mod_check_actor(lua_State* L, int idx) {
    PcModActorHandle* handle = (PcModActorHandle*)luaL_checkudata(L, idx, PC_MOD_ACTOR_MT);
    return handle->actor;
}

int pc_mod_parse_part(lua_State* L, int idx) {
    static const char* kPartNames[ACTOR_PART_NUM] = {
        "fg", "item", "unused", "player", "npc", "bg", "effect", "control",
    };

    if (lua_type(L, idx) == LUA_TNUMBER) {
        int part = (int)lua_tointeger(L, idx);
        if (part < 0 || part >= ACTOR_PART_NUM) {
            luaL_argerror(L, idx, "actor part out of range");
        }
        return part;
    }

    size_t len = 0;
    const char* name = luaL_checklstring(L, idx, &len);
    for (int i = 0; i < ACTOR_PART_NUM; i++) {
        if (strcmp(name, kPartNames[i]) == 0) {
            return i;
        }
    }

    luaL_argerror(L, idx, "unknown actor part");
    return 0;
}

const char* pc_mod_part_name(int part) {
    static const char* kPartNames[ACTOR_PART_NUM] = {
        "fg", "item", "unused", "player", "npc", "bg", "effect", "control",
    };

    if (part < 0 || part >= ACTOR_PART_NUM) {
        return "unknown";
    }
    return kPartNames[part];
}

static int l_actor_get_player(lua_State* L) {
    GAME_PLAY* play = pc_mod_try_get_play();
    if (!play) {
        lua_pushnil(L);
        return 1;
    }

    pc_mod_push_actor(L, (ACTOR*)get_player_actor_withoutCheck(play));
    return 1;
}

static int l_actor_is_valid(lua_State* L) {
    ACTOR* actor = pc_mod_check_actor(L, 1);
    GAME_PLAY* play = pc_mod_try_get_play();
    lua_pushboolean(L, pc_mod_actor_is_live(actor, play));
    return 1;
}

static int l_actor_get_profile(lua_State* L) {
    ACTOR* actor = pc_mod_check_actor(L, 1);
    lua_pushinteger(L, actor->id);
    return 1;
}

static int l_actor_get_part(lua_State* L) {
    ACTOR* actor = pc_mod_check_actor(L, 1);
    lua_pushstring(L, pc_mod_part_name(actor->part));
    return 1;
}

static int l_actor_get_npc_id(lua_State* L) {
    ACTOR* actor = pc_mod_check_actor(L, 1);
    lua_pushinteger(L, actor->npc_id);
    return 1;
}

static int l_actor_get_position(lua_State* L) {
    ACTOR* actor = pc_mod_check_actor(L, 1);
    lua_pushnumber(L, actor->world.position.x);
    lua_pushnumber(L, actor->world.position.y);
    lua_pushnumber(L, actor->world.position.z);
    return 3;
}

static int l_actor_set_position(lua_State* L) {
    ACTOR* actor = pc_mod_check_actor(L, 1);
    actor->world.position.x = luaL_checknumber(L, 2);
    actor->world.position.y = luaL_checknumber(L, 3);
    actor->world.position.z = luaL_checknumber(L, 4);
    return 0;
}

static int l_actor_get_rotation(lua_State* L) {
    ACTOR* actor = pc_mod_check_actor(L, 1);
    lua_pushnumber(L, SHORT2RAD_ANGLE2(actor->world.angle.x));
    lua_pushnumber(L, SHORT2RAD_ANGLE2(actor->world.angle.y));
    lua_pushnumber(L, SHORT2RAD_ANGLE2(actor->world.angle.z));
    return 3;
}

static int l_actor_set_rotation(lua_State* L) {
    ACTOR* actor = pc_mod_check_actor(L, 1);
    actor->world.angle.x = RAD2SHORT_ANGLE2(luaL_checknumber(L, 2));
    actor->world.angle.y = RAD2SHORT_ANGLE2(luaL_checknumber(L, 3));
    actor->world.angle.z = RAD2SHORT_ANGLE2(luaL_checknumber(L, 4));
    return 0;
}

static int l_actor_get_distance_to_player(lua_State* L) {
    ACTOR* actor = pc_mod_check_actor(L, 1);
    lua_pushnumber(L, actor->player_distance_xz);
    return 1;
}

static int l_actor_each(lua_State* L) {
    int part = pc_mod_parse_part(L, 1);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    GAME_PLAY* play = pc_mod_try_get_play();
    if (!play) {
        return 0;
    }

    for (ACTOR* actor = play->actor_info.list[part].actor; actor != nullptr; actor = actor->next_actor) {
        lua_pushvalue(L, 2);
        pc_mod_push_actor(L, actor);
        if (lua_pcall(L, 1, 1, 0) != LUA_OK) {
            fprintf(stderr, "[mod] actor.each err: %s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
            continue;
        }

        if (lua_isboolean(L, -1) && lua_toboolean(L, -1) == 0) {
            lua_pop(L, 1);
            break;
        }

        lua_pop(L, 1);
    }

    return 0;
}

typedef struct PcModActorIterState {
    ACTOR* next;
} PcModActorIterState;

static int l_actor_iter_next(lua_State* L) {
    PcModActorIterState* state = (PcModActorIterState*)luaL_checkudata(L, 1, PC_MOD_ACTOR_ITER_MT);

    if (state->next == nullptr) {
        return 0;
    }

    ACTOR* actor = state->next;
    state->next = actor->next_actor;
    pc_mod_push_actor(L, actor);
    return 1;
}

static int l_actor_iter(lua_State* L) {
    int part = pc_mod_parse_part(L, 1);
    GAME_PLAY* play = pc_mod_try_get_play();
    PcModActorIterState* state;

    state = (PcModActorIterState*)lua_newuserdata(L, sizeof(PcModActorIterState));
    state->next = (play != nullptr) ? play->actor_info.list[part].actor : nullptr;
    luaL_getmetatable(L, PC_MOD_ACTOR_ITER_MT);
    lua_setmetatable(L, -2);

    lua_pushcfunction(L, &l_actor_iter_next, PC_MOD_ACTOR_ITER_MT);
    lua_pushvalue(L, -2);
    lua_pushnil(L);
    return 3;
}

static void pc_mod_read_sxyz(lua_State* L, int table_index, const char* field, const s_xyz* defaults, s_xyz* out) {
    out->x = defaults->x;
    out->y = defaults->y;
    out->z = defaults->z;

    lua_getfield(L, table_index, field);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    lua_rawgeti(L, -1, 1);
    if (lua_isnumber(L, -1)) {
        out->x = (s16)RAD2SHORT_ANGLE2((f32)lua_tonumber(L, -1));
    }
    lua_pop(L, 1);

    lua_rawgeti(L, -1, 2);
    if (lua_isnumber(L, -1)) {
        out->y = (s16)RAD2SHORT_ANGLE2((f32)lua_tonumber(L, -1));
    }
    lua_pop(L, 1);

    lua_rawgeti(L, -1, 3);
    if (lua_isnumber(L, -1)) {
        out->z = (s16)RAD2SHORT_ANGLE2((f32)lua_tonumber(L, -1));
    }
    lua_pop(L, 2);
}

static void pc_mod_read_xyz(lua_State* L, int table_index, const char* field, const xyz_t* defaults, xyz_t* out) {
    out->x = defaults->x;
    out->y = defaults->y;
    out->z = defaults->z;

    lua_getfield(L, table_index, field);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    lua_rawgeti(L, -1, 1);
    if (lua_isnumber(L, -1)) {
        out->x = (f32)lua_tonumber(L, -1);
    }
    lua_pop(L, 1);

    lua_rawgeti(L, -1, 2);
    if (lua_isnumber(L, -1)) {
        out->y = (f32)lua_tonumber(L, -1);
    }
    lua_pop(L, 1);

    lua_rawgeti(L, -1, 3);
    if (lua_isnumber(L, -1)) {
        out->z = (f32)lua_tonumber(L, -1);
    }
    lua_pop(L, 2);
}

static void pc_mod_read_block_field(lua_State* L, int table_index, const char* field, s8 default_x, s8 default_z,
                                    s8* out_x, s8* out_z) {
    int value;

    *out_x = default_x;
    *out_z = default_z;

    lua_getfield(L, table_index, field);
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return;
    }

    lua_rawgeti(L, -1, 1);
    if (lua_isnumber(L, -1)) {
        value = (int)lua_tointeger(L, -1);
        if (value < -128 || value > 127) {
            luaL_error(L, "invalid block x");
        }
        *out_x = (s8)value;
    }
    lua_pop(L, 1);

    lua_rawgeti(L, -1, 2);
    if (lua_isnumber(L, -1)) {
        value = (int)lua_tointeger(L, -1);
        if (value < -128 || value > 127) {
            luaL_error(L, "invalid block z");
        }
        *out_z = (s8)value;
    }
    lua_pop(L, 2);
}

static int pc_mod_read_integer_field(lua_State* L, int table_index, const char* field, int default_value, int min_value,
                                     int max_value, const char* error_label) {
    int value;

    lua_getfield(L, table_index, field);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return default_value;
    }

    value = (int)luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    if (value < min_value || value > max_value) {
        luaL_error(L, "%s", error_label);
    }

    return value;
}

static int pc_mod_read_optional_integer_field(lua_State* L, int table_index, const char* field, int min_value,
                                              int max_value, const char* error_label) {
    int value;

    lua_getfield(L, table_index, field);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return -1;
    }

    value = (int)luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    if (value < min_value || value > max_value) {
        luaL_error(L, "%s", error_label);
    }

    return value;
}

static int pc_mod_try_register_event_npc(lua_State* L, mActor_name_t event_id, mActor_name_t villager_id) {
    if (ITEM_NAME_GET_TYPE(event_id) != NAME_TYPE_SPNPC) {
        luaL_error(L, "npc_id must be a special NPC id when villager_id is set");
    }

    if (mNpc_GetSameEventNpc(event_id) != nullptr) {
        return 1;
    }

    if (!mNpc_RegistEventNpc(event_id, event_id, villager_id, EMPTY_NO)) {
        return 0;
    }

    return 1;
}

static int l_actor_make(lua_State* L) {
    GAME_PLAY* play = pc_mod_try_get_play();
    s16 profile;
    xyz_t position;
    s_xyz rotation = { 0, 0, 0 };
    s8 bx = -1;
    s8 bz = -1;
    int npc_id = EMPTY_NO;
    int villager_id = -1;
    int arg = -1;
    ACTOR* actor;

    if (!play) {
        lua_pushnil(L);
        return 1;
    }

    profile = (s16)luaL_checkinteger(L, 1);
    if (profile < 0 || profile >= mAc_PROFILE_NUM) {
        luaL_argerror(L, 1, "invalid actor profile");
    }

    position.x = (f32)luaL_checknumber(L, 2);
    position.y = (f32)luaL_checknumber(L, 3);
    position.z = (f32)luaL_checknumber(L, 4);

    if (!lua_isnoneornil(L, 5)) {
        luaL_checktype(L, 5, LUA_TTABLE);
        pc_mod_read_sxyz(L, 5, "rotation", &rotation, &rotation);
        pc_mod_read_block_field(L, 5, "block", -1, -1, &bx, &bz);
        npc_id = pc_mod_read_integer_field(L, 5, "npc_id", EMPTY_NO, 0, 0xFFFF, "invalid npc id");
        villager_id =
            pc_mod_read_optional_integer_field(L, 5, "villager_id", 0, 0xFFFF, "invalid villager id");
        arg = pc_mod_read_integer_field(L, 5, "arg", -1, SHT_MIN_S, SHT_MAX_S, "invalid actor argument");
    }

    if (villager_id >= 0) {
        if (!pc_mod_try_register_event_npc(L, (mActor_name_t)npc_id, (mActor_name_t)villager_id)) {
            lua_pushnil(L);
            return 1;
        }
    }

    actor = Actor_info_make_actor(&play->actor_info, (GAME*)play, profile, position.x, position.y, position.z,
                                  rotation.x, rotation.y, rotation.z, bx, bz, -1, (mActor_name_t)npc_id, (s16)arg, -1,
                                  -1);
    pc_mod_push_actor(L, actor);
    return 1;
}

static const luaL_Reg actorlib[] = {
    { "get_player",             l_actor_get_player },
    { "is_valid",               l_actor_is_valid },
    { "get_profile",            l_actor_get_profile },
    { "get_part",               l_actor_get_part },
    { "get_npc_id",             l_actor_get_npc_id },
    { "get_position",           l_actor_get_position },
    { "set_position",           l_actor_set_position },
    { "get_rotation",           l_actor_get_rotation },
    { "set_rotation",           l_actor_set_rotation },
    { "get_distance_to_player", l_actor_get_distance_to_player },
    { "each",                   l_actor_each },
    { "iter",                   l_actor_iter },
    { "make",                   l_actor_make },
    { nullptr, nullptr },
};

extern "C" void pc_mod_register_actor_api(lua_State* L) {
    luaL_newmetatable(L, PC_MOD_ACTOR_MT);
    pc_mod_lock_metatable(L);
    lua_pop(L, 1);

    luaL_newmetatable(L, PC_MOD_ACTOR_ITER_MT);
    pc_mod_lock_metatable(L);
    lua_pop(L, 1);

    luaL_register(L, "actor", actorlib);
    lua_pop(L, 1);
}
