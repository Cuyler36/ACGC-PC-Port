#include "pc_mod_animal_api.h"
#include "pc_mod_animal_internal.h"
#include "pc_mod_save_internal.h"
#include "pc_mod_quest_internal.h"
#include "pc_mod_lua_internal.h"
#include "mod/pc_mod_actor_internal.h"
#include "lualib.h"

extern "C" {
#include "m_common_data.h"
#include "m_npc.h"
#include "m_npc_schedule.h"
#include "m_npc_walk.h"
#include "m_actor.h"
#include "m_field_info.h"
#include "m_name_table.h"
#include "ac_set_npc_manager.h"
#include "m_player_lib.h"
}

static void pc_mod_clear_npclist_entry(int animal_index) {
    mNpc_NpcList_c* list;

    if (animal_index < 0 || animal_index >= ANIMAL_NUM_MAX) {
        return;
    }

    list = Common_GetPointer(npclist[animal_index]);
    list->name = EMPTY_NO;
    list->field_name = RSV_NO;
    list->appear_flag = FALSE;
    list->house_data.type = 0xFF;
    list->house_data.palette = 0xFF;
    list->house_data.wall_id = 0xFF;
    list->house_data.floor_id = 0xFF;
    list->house_data.main_layer_id = 203;
    list->reward_furniture = EMPTY_NO;
    mQst_ClearQuestInfo(&list->quest_info);
}

static void pc_mod_despawn_animal_actors(GAME_PLAY* play, mActor_name_t npc_id, int animal_index) {
    ACTOR* actorx;
    mActor_name_t house_id;

    if (play == NULL || ITEM_NAME_GET_TYPE(npc_id) != NAME_TYPE_NPC) {
        return;
    }

    actorx = Actor_info_fgName_search(&play->actor_info, npc_id, ACTOR_PART_NPC);
    if (actorx == NULL && animal_index >= 0 && animal_index < ANIMAL_NUM_MAX) {
        mActor_name_t list_name = Common_Get(npclist[animal_index]).name;
        if (list_name != EMPTY_NO) {
            actorx = Actor_info_fgName_search(&play->actor_info, list_name, ACTOR_PART_NPC);
        }
    }

    if (actorx != NULL) {
        Actor_delete(actorx);
    }

    house_id = NPC_ID_TO_NPC_HOUSE_ID(npc_id);
    actorx = Actor_info_fgName_search(&play->actor_info, house_id, ACTOR_PART_ITEM);
    if (actorx != NULL) {
        Actor_delete(actorx);
    }
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

static int pc_mod_animal_handle_is_valid(PcModAnimalHandle* handle) {
    return handle != nullptr && handle->animal_index >= 0 && pc_mod_animal_is_live(handle->animal_index);
}

static void pc_mod_invalidate_animal_handle(PcModAnimalHandle* handle) {
    if (handle != nullptr) {
        handle->animal_index = PC_MOD_ANIMAL_INDEX_INVALID;
    }
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

    if (!pc_mod_animal_handle_is_valid(handle)) {
        luaL_error(L, "invalid Animal handle");
    }

    return pc_mod_animal_data(handle);
}

static void pc_mod_sync_animal_to_town(int animal_index) {
    mNpc_SyncNpcListEntry(animal_index);
    mNPS_SyncNpcSchedule(animal_index);
    mNpcW_RegisterAnimal(animal_index);

    GAME_PLAY* play = pc_mod_try_get_play();
    if (play != NULL) {
        aSNMgr_RefreshAnimalInManager(play, animal_index);
    }

    if (mFI_CheckFieldData()) {
        mFI_SetFGUpData();
    }
}

static int l_animal_from_index(lua_State* L) {
    int animal_index = pc_mod_check_animal_index(L, 1);
    pc_mod_push_animal(L, animal_index);
    return 1;
}

static int l_animal_new(lua_State* L) {
    mActor_name_t npc_id = (mActor_name_t)luaL_checkinteger(L, 1);
    int optional_bypass_validation = lua_toboolean(L, 2);
    int item_type = ITEM_NAME_GET_TYPE(npc_id);
    int idx;

    if (!pc_mod_save_is_ready()) {
        luaL_error(L, "save is not ready");
    }

    if (item_type != NAME_TYPE_NPC) {
        luaL_error(L, "invalid NPC ID: %04X", npc_id);
    }
    
    if (!optional_bypass_validation) {
        Animal_c* animal;
        int i;

        if (mNpc_GET_IDX(npc_id) >= mNpc_GetNpcNumMax()) {
            luaL_error(L, "NPC ID out of range: %04X", npc_id);
        }

        animal = Save_Get(animals);
        for (i = 0; i < ANIMAL_NUM_MAX; i++) {
            if (!mNpc_CheckFreeAnimalInfo(animal) && animal->id.npc_id == npc_id) {
                luaL_error(L, "NPC ID already in use: %04X", npc_id);
            }

            animal++;
        }

        if (mNpc_GetFreeAnimalInfo(Save_Get(animals), ANIMAL_NUM_MAX) == -1) {
            luaL_error(L, "No free animal slots available");
        }
    }

    idx = mNpc_AddNpc(npc_id);
    if (idx == -1) {
        luaL_error(L, "failed to add NPC: %04X", npc_id);
    }

    if (mNpc_TryBuildHouseForAnimal(idx) == -1 && !optional_bypass_validation) {
        mNpc_RemoveNpcByIdx(idx, FALSE); // Rollback
        mNpc_ClearHaveAppeared(npc_id); // Rollback appeared
        luaL_error(L, "failed to build house for NPC #%d", idx);
    }

    pc_mod_sync_animal_to_town(idx);
    pc_mod_push_animal(L, idx);
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
    lua_pushboolean(L, pc_mod_animal_handle_is_valid(handle));
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

    lua_pushinteger(L, animal->home_info.block_x);
    lua_setfield(L, -2, "bx");
    lua_pushinteger(L, animal->home_info.block_z);
    lua_setfield(L, -2, "bz");

    lua_pushinteger(L, animal->home_info.ut_x);
    lua_setfield(L, -2, "ux");
    lua_pushinteger(L, animal->home_info.ut_z);
    lua_setfield(L, -2, "uz");

    return 1;
}

static int l_animal_prop_contest(lua_State* L) {
    PcModAnimalHandle* handle = pc_mod_check_animal(L, 1);
    pc_mod_require_animal(L, 1);
    pc_mod_push_contest_quest(L, handle->animal_index);
    return 1;
}

static int l_animal_npc_delete(lua_State* L) {
    PcModAnimalHandle* handle = pc_mod_check_animal(L, 1);
    int animal_index = handle->animal_index;
    Animal_c* animal;
    mNpc_walk_c* walk;
    mActor_name_t npc_id;
    GAME_PLAY* play;
    int i;

    if (!pc_mod_animal_handle_is_valid(handle)) {
        luaL_error(L, "invalid Animal handle");
    }

    if (mNpc_GetAnimalNum() <= 1) {
        luaL_error(L, "cannot delete the last NPC");
    }

    animal = Save_GetPointer(animals[animal_index]);
    if (mNpc_CheckFreeAnimalInfo(animal)) {
        luaL_error(L, "NPC #%d is not live", animal_index);
    }

    npc_id = animal->id.npc_id;
    play = pc_mod_try_get_play();

    mNPS_reset_schedule_area(&animal->id);
    Common_Get(npc_schedule[animal_index]).id = NULL;

    // Clear walk info if the villager is currently outside
    walk = Common_GetPointer(npc_walk);
    for (i = 0; i < mNpcW_MAX; i++) {
        if (mNpc_CheckCmpAnimalPersonalID(&walk->info[i].id, &animal->id)) {
            mNpcW_ClearNpcWalkInfo(&walk->info[i], 1);
            walk->used_idx_bitfield &= ~(1u << animal_index);
            break;
        }
    }

    if (play != NULL) {
        aSNMgr_UnregisterAnimalInManager(play, animal_index);
        pc_mod_despawn_animal_actors(play, npc_id, animal_index);
    }

    if (mNpc_RemoveNpcByIdx(animal_index, TRUE) == -1) {
        luaL_error(L, "failed to delete NPC #%d", animal_index);
    }

    pc_mod_clear_npclist_entry(animal_index);

    if (mFI_CheckFieldData()) {
        mFI_SetFGUpData();
        if (play != NULL) {
            PLAYER_ACTOR* player = GET_PLAYER_ACTOR(play);
            if (player != NULL) {
                mFI_SetBearActor(play, player->actor_class.world.position, TRUE);
            }
        }
    }

    pc_mod_invalidate_animal_handle(handle);
    return 0;
}

static const PcModPropertyDef animal_properties[] = {
    { "Index",     l_animal_prop_index,      nullptr },
    { "Valid",     l_animal_prop_valid,      nullptr },
    { "NpcId",     l_animal_prop_npc_id,     nullptr },
    { "Name",      l_animal_prop_name,       nullptr },
    { "Home",      l_animal_prop_home,       nullptr },
    { "Contest",   l_animal_prop_contest,    nullptr },
    { nullptr,     nullptr,                  nullptr },
};

static const luaL_Reg animal_npc_methods[] = {
    { "Delete", l_animal_npc_delete },
    { nullptr, nullptr },
};

static const luaL_Reg animal_module[] = {
    { "new",       l_animal_new },
    { "fromIndex", l_animal_from_index },
    { "count",     l_animal_count },
    { "iter",      l_animal_iter },
    { nullptr, nullptr },
};

extern "C" void pc_mod_register_animal_api(lua_State* L) {
    luaL_newmetatable(L, PC_MOD_ANIMAL_MT);
    pc_mod_register_userdata(L, animal_properties, animal_npc_methods);
    pc_mod_lock_metatable(L);
    lua_pop(L, 1);

    luaL_newmetatable(L, PC_MOD_ANIMAL_ITER_MT);
    pc_mod_lock_metatable(L);
    lua_pop(L, 1);

    luaL_register(L, "Animal", animal_module);
    pc_mod_set_module_iter(L, "Animal", PC_MOD_ANIMAL_MODULE_MT, l_animal_iter);
    lua_pop(L, 1);
}
