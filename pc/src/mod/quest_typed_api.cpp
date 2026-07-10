#include "pc_mod_quest_internal.h"
#include "pc_mod_save_internal.h"
#include "pc_mod_item_internal.h"
#include "pc_mod_lua_internal.h"
#include "lualib.h"

extern "C" {
#include "m_common_data.h"
#include "m_private.h"
#include "m_npc.h"
#include "m_quest.h"
#include "m_name_table.h"
}

static Private_c* pc_mod_quest_private(lua_State* L, int player_no) {
    Private_c* priv;

    if (player_no < 0 || player_no >= PLAYER_NUM) {
        luaL_error(L, "invalid quest handle");
    }

    priv = Save_GetPointer(private_data[player_no]);
    if (!priv || priv->exists == 0) {
        luaL_error(L, "invalid quest handle");
    }

    return priv;
}

static Animal_c* pc_mod_quest_animal(lua_State* L, int animal_index) {
    Animal_c* animal;

    if (animal_index < 0 || animal_index >= ANIMAL_NUM_MAX) {
        luaL_error(L, "invalid quest handle");
    }

    animal = Save_GetPointer(animals[animal_index]);
    if (!animal || mNpc_CheckFreeAnimalInfo(animal)) {
        luaL_error(L, "invalid quest handle");
    }

    return animal;
}

static void pc_mod_push_time_limit(lua_State* L, const mQst_base_c* base) {
    if (!base->time_limit_enabled) {
        lua_pushnil(L);
        return;
    }

    lua_createtable(L, 0, 6);
    lua_pushinteger(L, base->time_limit.year);
    lua_setfield(L, -2, "year");
    lua_pushinteger(L, base->time_limit.month);
    lua_setfield(L, -2, "month");
    lua_pushinteger(L, base->time_limit.day);
    lua_setfield(L, -2, "day");
    lua_pushinteger(L, base->time_limit.hour);
    lua_setfield(L, -2, "hour");
    lua_pushinteger(L, base->time_limit.min);
    lua_setfield(L, -2, "minute");
    lua_pushinteger(L, base->time_limit.sec);
    lua_setfield(L, -2, "second");
}

static void pc_mod_push_npc_id(lua_State* L, AnmPersonalID_c* pid) {
    if (mNpc_CheckFreeAnimalPersonalID(pid)) {
        lua_pushnil(L);
        return;
    }
    lua_pushinteger(L, pid->npc_id);
}

static const char* pc_mod_quest_type_name(u32 quest_type) {
    switch (quest_type) {
        case mQst_QUEST_TYPE_DELIVERY:
            return "delivery";
        case mQst_QUEST_TYPE_ERRAND:
            return "errand";
        case mQst_QUEST_TYPE_CONTEST:
            return "contest";
        default:
            return "none";
    }
}

/* --- Delivery --- */

void pc_mod_push_delivery_quest(lua_State* L, int player_no, int slot_index) {
    PcModDeliveryQuestHandle* handle =
        (PcModDeliveryQuestHandle*)lua_newuserdata(L, sizeof(PcModDeliveryQuestHandle));
    handle->player_no = player_no;
    handle->slot_index = slot_index;
    luaL_getmetatable(L, PC_MOD_DELIVERY_QUEST_MT);
    lua_setmetatable(L, -2);
}

void pc_mod_push_deliveries(lua_State* L, int player_no) {
    int i;

    lua_createtable(L, mPr_DELIVERY_QUEST_NUM, 0);
    for (i = 0; i < mPr_DELIVERY_QUEST_NUM; i++) {
        pc_mod_push_delivery_quest(L, player_no, i);
        lua_rawseti(L, -2, i + 1);
    }
}

static mQst_delivery_c* pc_mod_require_delivery(lua_State* L, int idx) {
    PcModDeliveryQuestHandle* handle = (PcModDeliveryQuestHandle*)luaL_checkudata(L, idx, PC_MOD_DELIVERY_QUEST_MT);
    Private_c* priv = pc_mod_quest_private(L, handle->player_no);

    if (handle->slot_index < 0 || handle->slot_index >= mPr_DELIVERY_QUEST_NUM) {
        luaL_error(L, "invalid DeliveryQuest handle");
    }

    return &priv->deliveries[handle->slot_index];
}

static int l_delivery_prop_index(lua_State* L) {
    PcModDeliveryQuestHandle* handle = (PcModDeliveryQuestHandle*)luaL_checkudata(L, 1, PC_MOD_DELIVERY_QUEST_MT);
    lua_pushinteger(L, handle->slot_index + 1);
    return 1;
}

static int l_delivery_prop_type(lua_State* L) {
    mQst_delivery_c* delivery = pc_mod_require_delivery(L, 1);
    lua_pushstring(L, pc_mod_quest_type_name(delivery->base.quest_type));
    return 1;
}

static int l_delivery_prop_kind(lua_State* L) {
    mQst_delivery_c* delivery = pc_mod_require_delivery(L, 1);
    lua_pushinteger(L, delivery->base.quest_kind);
    return 1;
}

static int l_delivery_prop_progress(lua_State* L) {
    mQst_delivery_c* delivery = pc_mod_require_delivery(L, 1);
    lua_pushinteger(L, delivery->base.progress);
    return 1;
}

static int l_delivery_prop_time_limit_enabled(lua_State* L) {
    mQst_delivery_c* delivery = pc_mod_require_delivery(L, 1);
    lua_pushboolean(L, delivery->base.time_limit_enabled);
    return 1;
}

static int l_delivery_prop_time_limit(lua_State* L) {
    mQst_delivery_c* delivery = pc_mod_require_delivery(L, 1);
    pc_mod_push_time_limit(L, &delivery->base);
    return 1;
}

static int l_delivery_prop_is_active(lua_State* L) {
    mQst_delivery_c* delivery = pc_mod_require_delivery(L, 1);
    lua_pushboolean(L, mQst_CheckFreeQuest(&delivery->base) == FALSE);
    return 1;
}

static int l_delivery_prop_recipient(lua_State* L) {
    mQst_delivery_c* delivery = pc_mod_require_delivery(L, 1);
    pc_mod_push_npc_id(L, &delivery->recipient);
    return 1;
}

static int l_delivery_prop_sender(lua_State* L) {
    mQst_delivery_c* delivery = pc_mod_require_delivery(L, 1);
    pc_mod_push_npc_id(L, &delivery->sender);
    return 1;
}

static int l_delivery_clear(lua_State* L) {
    mQst_delivery_c* delivery = pc_mod_require_delivery(L, 1);
    mQst_ClearDelivery(delivery, 1);
    return 0;
}

static const PcModPropertyDef delivery_properties[] = {
    { "Index",            l_delivery_prop_index,              nullptr },
    { "Type",             l_delivery_prop_type,               nullptr },
    { "Kind",             l_delivery_prop_kind,               nullptr },
    { "Progress",         l_delivery_prop_progress,           nullptr },
    { "TimeLimitEnabled", l_delivery_prop_time_limit_enabled, nullptr },
    { "TimeLimit",        l_delivery_prop_time_limit,         nullptr },
    { "IsActive",         l_delivery_prop_is_active,          nullptr },
    { "RecipientNpcId",   l_delivery_prop_recipient,          nullptr },
    { "SenderNpcId",      l_delivery_prop_sender,             nullptr },
    { nullptr,            nullptr,                            nullptr },
};

static const luaL_Reg delivery_methods[] = {
    { "Clear", l_delivery_clear },
    { nullptr, nullptr },
};

/* --- Errand --- */

void pc_mod_push_errand_quest(lua_State* L, int player_no, int slot_index) {
    PcModErrandQuestHandle* handle = (PcModErrandQuestHandle*)lua_newuserdata(L, sizeof(PcModErrandQuestHandle));
    handle->player_no = player_no;
    handle->slot_index = slot_index;
    luaL_getmetatable(L, PC_MOD_ERRAND_QUEST_MT);
    lua_setmetatable(L, -2);
}

void pc_mod_push_errands(lua_State* L, int player_no) {
    int i;

    lua_createtable(L, mPr_ERRAND_QUEST_NUM, 0);
    for (i = 0; i < mPr_ERRAND_QUEST_NUM; i++) {
        pc_mod_push_errand_quest(L, player_no, i);
        lua_rawseti(L, -2, i + 1);
    }
}

static mQst_errand_c* pc_mod_require_errand(lua_State* L, int idx) {
    PcModErrandQuestHandle* handle = (PcModErrandQuestHandle*)luaL_checkudata(L, idx, PC_MOD_ERRAND_QUEST_MT);
    Private_c* priv = pc_mod_quest_private(L, handle->player_no);

    if (handle->slot_index < 0 || handle->slot_index >= mPr_ERRAND_QUEST_NUM) {
        luaL_error(L, "invalid ErrandQuest handle");
    }

    return &priv->errands[handle->slot_index];
}

static int l_errand_prop_index(lua_State* L) {
    PcModErrandQuestHandle* handle = (PcModErrandQuestHandle*)luaL_checkudata(L, 1, PC_MOD_ERRAND_QUEST_MT);
    lua_pushinteger(L, handle->slot_index + 1);
    return 1;
}

static int l_errand_prop_type(lua_State* L) {
    mQst_errand_c* errand = pc_mod_require_errand(L, 1);
    lua_pushstring(L, pc_mod_quest_type_name(errand->base.quest_type));
    return 1;
}

static int l_errand_prop_kind(lua_State* L) {
    mQst_errand_c* errand = pc_mod_require_errand(L, 1);
    lua_pushinteger(L, errand->base.quest_kind);
    return 1;
}

static int l_errand_prop_progress(lua_State* L) {
    mQst_errand_c* errand = pc_mod_require_errand(L, 1);
    lua_pushinteger(L, errand->base.progress);
    return 1;
}

static int l_errand_prop_time_limit_enabled(lua_State* L) {
    mQst_errand_c* errand = pc_mod_require_errand(L, 1);
    lua_pushboolean(L, errand->base.time_limit_enabled);
    return 1;
}

static int l_errand_prop_time_limit(lua_State* L) {
    mQst_errand_c* errand = pc_mod_require_errand(L, 1);
    pc_mod_push_time_limit(L, &errand->base);
    return 1;
}

static int l_errand_prop_is_active(lua_State* L) {
    mQst_errand_c* errand = pc_mod_require_errand(L, 1);
    lua_pushboolean(L, mQst_CheckFreeQuest(&errand->base) == FALSE);
    return 1;
}

static int l_errand_prop_recipient(lua_State* L) {
    mQst_errand_c* errand = pc_mod_require_errand(L, 1);
    pc_mod_push_npc_id(L, &errand->recipient);
    return 1;
}

static int l_errand_prop_sender(lua_State* L) {
    mQst_errand_c* errand = pc_mod_require_errand(L, 1);
    pc_mod_push_npc_id(L, &errand->sender);
    return 1;
}

static int l_errand_prop_item(lua_State* L) {
    mQst_errand_c* errand = pc_mod_require_errand(L, 1);
    pc_mod_push_item(L, errand->item);
    return 1;
}

static int l_errand_prop_pockets_idx(lua_State* L) {
    mQst_errand_c* errand = pc_mod_require_errand(L, 1);
    if (errand->pockets_idx < 0) {
        lua_pushnil(L);
    } else {
        lua_pushinteger(L, errand->pockets_idx + 1);
    }
    return 1;
}

static int l_errand_prop_errand_type(lua_State* L) {
    mQst_errand_c* errand = pc_mod_require_errand(L, 1);
    lua_pushinteger(L, errand->errand_type);
    return 1;
}

static int l_errand_clear(lua_State* L) {
    mQst_errand_c* errand = pc_mod_require_errand(L, 1);
    mQst_ClearErrand(errand, 1);
    return 0;
}

static const PcModPropertyDef errand_properties[] = {
    { "Index",            l_errand_prop_index,              nullptr },
    { "Type",             l_errand_prop_type,               nullptr },
    { "Kind",             l_errand_prop_kind,               nullptr },
    { "Progress",         l_errand_prop_progress,           nullptr },
    { "TimeLimitEnabled", l_errand_prop_time_limit_enabled, nullptr },
    { "TimeLimit",        l_errand_prop_time_limit,         nullptr },
    { "IsActive",         l_errand_prop_is_active,          nullptr },
    { "RecipientNpcId",   l_errand_prop_recipient,          nullptr },
    { "SenderNpcId",      l_errand_prop_sender,             nullptr },
    { "Item",             l_errand_prop_item,               nullptr },
    { "PocketsIdx",       l_errand_prop_pockets_idx,        nullptr },
    { "ErrandType",       l_errand_prop_errand_type,        nullptr },
    { nullptr,            nullptr,                          nullptr },
};

static const luaL_Reg errand_methods[] = {
    { "Clear", l_errand_clear },
    { nullptr, nullptr },
};

/* --- Contest --- */

void pc_mod_push_contest_quest(lua_State* L, int animal_index) {
    PcModContestQuestHandle* handle = (PcModContestQuestHandle*)lua_newuserdata(L, sizeof(PcModContestQuestHandle));
    handle->animal_index = animal_index;
    luaL_getmetatable(L, PC_MOD_CONTEST_QUEST_MT);
    lua_setmetatable(L, -2);
}

static mQst_contest_c* pc_mod_require_contest(lua_State* L, int idx) {
    PcModContestQuestHandle* handle = (PcModContestQuestHandle*)luaL_checkudata(L, idx, PC_MOD_CONTEST_QUEST_MT);
    Animal_c* animal = pc_mod_quest_animal(L, handle->animal_index);
    return &animal->contest_quest;
}

static int l_contest_prop_animal_index(lua_State* L) {
    PcModContestQuestHandle* handle = (PcModContestQuestHandle*)luaL_checkudata(L, 1, PC_MOD_CONTEST_QUEST_MT);
    lua_pushinteger(L, handle->animal_index);
    return 1;
}

static int l_contest_prop_type(lua_State* L) {
    mQst_contest_c* contest = pc_mod_require_contest(L, 1);
    lua_pushstring(L, pc_mod_quest_type_name(contest->base.quest_type));
    return 1;
}

static int l_contest_prop_kind(lua_State* L) {
    mQst_contest_c* contest = pc_mod_require_contest(L, 1);
    lua_pushinteger(L, contest->base.quest_kind);
    return 1;
}

static int l_contest_prop_progress(lua_State* L) {
    mQst_contest_c* contest = pc_mod_require_contest(L, 1);
    lua_pushinteger(L, contest->base.progress);
    return 1;
}

static int l_contest_prop_time_limit_enabled(lua_State* L) {
    mQst_contest_c* contest = pc_mod_require_contest(L, 1);
    lua_pushboolean(L, contest->base.time_limit_enabled);
    return 1;
}

static int l_contest_prop_time_limit(lua_State* L) {
    mQst_contest_c* contest = pc_mod_require_contest(L, 1);
    pc_mod_push_time_limit(L, &contest->base);
    return 1;
}

static int l_contest_prop_is_active(lua_State* L) {
    mQst_contest_c* contest = pc_mod_require_contest(L, 1);
    lua_pushboolean(L, mQst_CheckFreeQuest(&contest->base) == FALSE);
    return 1;
}

static int l_contest_prop_requested_item(lua_State* L) {
    mQst_contest_c* contest = pc_mod_require_contest(L, 1);
    pc_mod_push_item(L, contest->requested_item);
    return 1;
}

static int l_contest_prop_player_name(lua_State* L) {
    mQst_contest_c* contest = pc_mod_require_contest(L, 1);

    if (mPr_NullCheckPersonalID(&contest->player_id)) {
        lua_pushnil(L);
        return 1;
    }

    pc_mod_push_game_string(L, contest->player_id.player_name, PLAYER_NAME_LEN);
    return 1;
}

static int l_contest_prop_contest_data_type(lua_State* L) {
    mQst_contest_c* contest = pc_mod_require_contest(L, 1);
    lua_pushinteger(L, contest->type);
    return 1;
}

static int l_contest_clear(lua_State* L) {
    mQst_contest_c* contest = pc_mod_require_contest(L, 1);
    mQst_ClearContest(contest);
    return 0;
}

static const PcModPropertyDef contest_properties[] = {
    { "AnimalIndex",      l_contest_prop_animal_index,       nullptr },
    { "Type",             l_contest_prop_type,               nullptr },
    { "Kind",             l_contest_prop_kind,               nullptr },
    { "Progress",         l_contest_prop_progress,           nullptr },
    { "TimeLimitEnabled", l_contest_prop_time_limit_enabled, nullptr },
    { "TimeLimit",        l_contest_prop_time_limit,         nullptr },
    { "IsActive",         l_contest_prop_is_active,          nullptr },
    { "RequestedItem",    l_contest_prop_requested_item,     nullptr },
    { "PlayerName",       l_contest_prop_player_name,        nullptr },
    { "ContestDataType",  l_contest_prop_contest_data_type,  nullptr },
    { nullptr,            nullptr,                           nullptr },
};

static const luaL_Reg contest_methods[] = {
    { "Clear", l_contest_clear },
    { nullptr, nullptr },
};

void pc_mod_register_quest_typed(lua_State* L) {
    luaL_newmetatable(L, PC_MOD_DELIVERY_QUEST_MT);
    pc_mod_register_userdata(L, delivery_properties, delivery_methods);
    pc_mod_lock_metatable(L);
    lua_pop(L, 1);

    luaL_newmetatable(L, PC_MOD_ERRAND_QUEST_MT);
    pc_mod_register_userdata(L, errand_properties, errand_methods);
    pc_mod_lock_metatable(L);
    lua_pop(L, 1);

    luaL_newmetatable(L, PC_MOD_CONTEST_QUEST_MT);
    pc_mod_register_userdata(L, contest_properties, contest_methods);
    pc_mod_lock_metatable(L);
    lua_pop(L, 1);
}
