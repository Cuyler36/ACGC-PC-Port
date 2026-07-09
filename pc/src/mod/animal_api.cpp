#include "pc_mod_animal_api.h"
#include "pc_mod_animal_internal.h"
#include "pc_mod_save_internal.h"
#include "pc_mod_lua_internal.h"
#include "lualib.h"

extern "C" {
#include "m_common_data.h"
#include "m_npc.h"
}

Animal_c* pc_mod_animal_data(PcModAnimalHandle* handle) {
    if (!handle || handle->animal_index < 0 || handle->animal_index >= ANIMAL_NUM_MAX) {
        return nullptr;
    }
    return Save_GetPointer(animals[handle->animal_index]);
}

static int pc_mod_animal_is_live(int animal_index) {
    Animal_c* animal;

    if (animal_index < 0 || animal_index >= ANIMAL_NUM_MAX) {
        return 0;
    }

    animal = Save_GetPointer(animals[animal_index]);
    return !mNpc_CheckFreeAnimalInfo(animal);
}

void pc_mod_push_animal(lua_State* L, int animal_index) {
    PcModAnimalHandle* handle;

    if (!pc_mod_animal_is_live(animal_index)) {
        lua_pushnil(L);
        return;
    }

    handle = (PcModAnimalHandle*)lua_newuserdata(L, sizeof(PcModAnimalHandle));
    handle->animal_index = animal_index;
    luaL_getmetatable(L, PC_MOD_ANIMAL_MT);
    lua_setmetatable(L, -2);
}

PcModAnimalHandle* pc_mod_check_animal(lua_State* L, int idx) {
    return (PcModAnimalHandle*)luaL_checkudata(L, idx, PC_MOD_ANIMAL_MT);
}

static Animal_c* pc_mod_require_animal(lua_State* L, int idx) {
    PcModAnimalHandle* handle = pc_mod_check_animal(L, idx);
    Animal_c* animal = pc_mod_animal_data(handle);

    if (!animal || mNpc_CheckFreeAnimalInfo(animal)) {
        luaL_error(L, "invalid Animal handle");
    }

    return animal;
}

static int l_animal_from_index(lua_State* L) {
    int animal_index = pc_mod_check_animal_index(L, 1);
    pc_mod_push_animal(L, animal_index);
    return 1;
}

static int l_animal_count(lua_State* L) {
    int count = 0;

    for (int i = 0; i < ANIMAL_NUM_MAX; i++) {
        if (pc_mod_animal_is_live(i)) {
            count++;
        }
    }

    lua_pushinteger(L, count);
    return 1;
}

static int l_animal_iter_next(lua_State* L) {
    PcModAnimalIterState* state = (PcModAnimalIterState*)luaL_checkudata(L, 1, PC_MOD_ANIMAL_ITER_MT);

    while (state->next_index < ANIMAL_NUM_MAX) {
        int animal_index = state->next_index++;
        if (pc_mod_animal_is_live(animal_index)) {
            pc_mod_push_animal(L, animal_index);
            return 1;
        }
    }

    return 0;
}

static int l_animal_iter(lua_State* L) {
    PcModAnimalIterState* state;

    state = (PcModAnimalIterState*)lua_newuserdata(L, sizeof(PcModAnimalIterState));
    state->next_index = 0;
    luaL_getmetatable(L, PC_MOD_ANIMAL_ITER_MT);
    lua_setmetatable(L, -2);

    lua_pushcfunction(L, &l_animal_iter_next, PC_MOD_ANIMAL_ITER_MT);
    lua_pushvalue(L, -2);
    lua_pushnil(L);
    return 3;
}

static int l_animal_prop_index(lua_State* L) {
    PcModAnimalHandle* handle = pc_mod_check_animal(L, 1);
    lua_pushinteger(L, handle->animal_index);
    return 1;
}

static int l_animal_prop_valid(lua_State* L) {
    PcModAnimalHandle* handle = pc_mod_check_animal(L, 1);
    lua_pushboolean(L, pc_mod_animal_is_live(handle->animal_index));
    return 1;
}

static int l_animal_prop_npc_id(lua_State* L) {
    Animal_c* animal = pc_mod_require_animal(L, 1);
    lua_pushinteger(L, animal->id.npc_id);
    return 1;
}

static int l_animal_prop_name(lua_State* L) {
    Animal_c* animal = pc_mod_require_animal(L, 1);
    u8 name[ANIMAL_NAME_LEN];

    mNpc_GetNpcWorldNameAnm(name, &animal->id);
    pc_mod_push_game_string(L, name, ANIMAL_NAME_LEN);
    return 1;
}

static int l_animal_prop_home(lua_State* L) {
    Animal_c* animal = pc_mod_require_animal(L, 1);

    lua_newtable(L);
    
    // push block
    lua_pushinteger(L, animal->home_info.block_x);
    lua_setfield(L, -2, "bx");
    lua_pushinteger(L, animal->home_info.block_z);
    lua_setfield(L, -2, "bz");
    
    // push unit
    lua_pushinteger(L, animal->home_info.ut_x);
    lua_setfield(L, -2, "ux");
    lua_pushinteger(L, animal->home_info.ut_z);
    lua_setfield(L, -2, "uz");

    return 1;
}

static const PcModPropertyDef animal_properties[] = {
    { "Index",     l_animal_prop_index,      nullptr },
    { "Valid",     l_animal_prop_valid,      nullptr },
    { "NpcId",     l_animal_prop_npc_id,     nullptr },
    { "Name",      l_animal_prop_name,       nullptr },
    { "Home",      l_animal_prop_home,       nullptr },
    { nullptr,     nullptr,                  nullptr },
};

static const luaL_Reg animal_module[] = {
    { "fromIndex", l_animal_from_index },
    { "count",     l_animal_count },
    { "iter",      l_animal_iter },
    { nullptr, nullptr },
};

extern "C" void pc_mod_register_animal_api(lua_State* L) {
    luaL_newmetatable(L, PC_MOD_ANIMAL_MT);
    pc_mod_register_userdata_properties(L, animal_properties);
    pc_mod_lock_metatable(L);
    lua_pop(L, 1);

    luaL_newmetatable(L, PC_MOD_ANIMAL_ITER_MT);
    pc_mod_lock_metatable(L);
    lua_pop(L, 1);

    luaL_register(L, "Animal", animal_module);
    pc_mod_set_module_iter(L, "Animal", PC_MOD_ANIMAL_MODULE_MT, l_animal_iter);
    lua_pop(L, 1);
}
