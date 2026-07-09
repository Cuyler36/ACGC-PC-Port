#include "pc_mod_item_slot_internal.h"
#include "pc_mod_item_internal.h"
#include "pc_mod_lua_internal.h"
#include "lualib.h"
#include <cstring>

extern "C" {
#include "m_common_data.h"
#include "m_private.h"
#include "m_name_table.h"
}

static int pc_mod_pockets_player_no(lua_State* L, int table_arg) {
    lua_rawgeti(L, table_arg, 1);
    PcModItemSlotHandle* handle = (PcModItemSlotHandle*)luaL_checkudata(L, -1, PC_MOD_ITEM_SLOT_MT);
    int player_no = handle->player_no;
    lua_pop(L, 1);
    return player_no;
}
static Private_c* pc_mod_item_slot_private(lua_State* L, PcModItemSlotHandle* handle) {
    Private_c* priv;

    if (handle->player_no < 0 || handle->player_no >= PLAYER_NUM) {
        luaL_error(L, "invalid ItemSlot handle");
    }

    priv = Save_GetPointer(private_data[handle->player_no]);
    if (!priv || priv->exists == 0) {
        luaL_error(L, "invalid ItemSlot handle");
    }

    if (handle->slot_index < 0 || handle->slot_index >= mPr_POCKETS_SLOT_COUNT) {
        luaL_error(L, "invalid ItemSlot handle");
    }

    return priv;
}

static const char* pc_mod_item_condition_name(u32 cond) {
    switch (cond) {
        case mPr_ITEM_COND_PRESENT:
            return "present";
        case mPr_ITEM_COND_QUEST:
            return "quest";
        default:
            return "normal";
    }
}

static u32 pc_mod_read_item_condition(lua_State* L, int arg) {
    size_t len = 0;
    const char* name = luaL_checklstring(L, arg, &len);

    if (strcmp(name, "present") == 0) {
        return mPr_ITEM_COND_PRESENT;
    }
    if (strcmp(name, "quest") == 0) {
        return mPr_ITEM_COND_QUEST;
    }
    if (strcmp(name, "normal") == 0) {
        return mPr_ITEM_COND_NORMAL;
    }

    luaL_argerror(L, arg, "item condition must be 'normal', 'present', or 'quest'");
    return mPr_ITEM_COND_NORMAL;
}

int pc_mod_read_item_from_value(lua_State* L, int arg, mActor_name_t* item) {
    PcModItemHandle* item_handle = pc_mod_try_check_item(L, arg);
    if (item_handle != nullptr) {
        *item = item_handle->item_id;
        return LUA_OK;
    }

    if (lua_isnumber(L, arg)) {
        lua_Integer value = luaL_checkinteger(L, arg);
        if (value < 0 || value > 0xFFFF) {
            luaL_argerror(L, arg, "item id out of range");
        }

        *item = (mActor_name_t)value;
        return LUA_OK;
    }

    luaL_argerror(L, arg, "expected Item or item id");
    return -1;
}

void pc_mod_push_item_slot(lua_State* L, int player_no, int slot_index) {
    PcModItemSlotHandle* handle = (PcModItemSlotHandle*)lua_newuserdata(L, sizeof(PcModItemSlotHandle));
    handle->player_no = player_no;
    handle->slot_index = slot_index;
    luaL_getmetatable(L, PC_MOD_ITEM_SLOT_MT);
    lua_setmetatable(L, -2);
}

int pc_mod_check_pocket_slot(lua_State* L, int arg) {
    int slot = (int)luaL_checkinteger(L, arg);

    if (slot < 1 || slot > mPr_POCKETS_SLOT_COUNT) {
        luaL_argerror(L, arg, "pocket slot out of range (1-15)");
    }

    return slot - 1;
}

void pc_mod_push_pockets(lua_State* L, int player_no) {
    lua_createtable(L, mPr_POCKETS_SLOT_COUNT, 0);

    for (int i = 0; i < mPr_POCKETS_SLOT_COUNT; i++) {
        pc_mod_push_item_slot(L, player_no, i);
        lua_rawseti(L, -2, i + 1);
    }

    luaL_getmetatable(L, PC_MOD_POCKETS_MT);
    lua_setmetatable(L, -2);
}

static int l_item_slot_prop_item_get(lua_State* L) {
    PcModItemSlotHandle* handle = (PcModItemSlotHandle*)luaL_checkudata(L, 1, PC_MOD_ITEM_SLOT_MT);
    Private_c* priv = pc_mod_item_slot_private(L, handle);
    pc_mod_push_item(L, priv->inventory.pockets[handle->slot_index]);
    return 1;
}

static int l_item_slot_prop_item_set(lua_State* L) {
    PcModItemSlotHandle* handle = (PcModItemSlotHandle*)luaL_checkudata(L, 1, PC_MOD_ITEM_SLOT_MT);
    Private_c* priv = pc_mod_item_slot_private(L, handle);
    mActor_name_t item;

    if (pc_mod_read_item_from_value(L, 3, &item) == LUA_OK) {
        u32 cond = mPr_GET_ITEM_COND(priv->inventory.item_conditions, handle->slot_index);

        mPr_SetPossessionItem(priv, handle->slot_index, item, cond);
    }

    return 0;
}

static int l_item_slot_prop_condition_get(lua_State* L) {
    PcModItemSlotHandle* handle = (PcModItemSlotHandle*)luaL_checkudata(L, 1, PC_MOD_ITEM_SLOT_MT);
    Private_c* priv = pc_mod_item_slot_private(L, handle);
    u32 cond = mPr_GET_ITEM_COND(priv->inventory.item_conditions, handle->slot_index);

    lua_pushstring(L, pc_mod_item_condition_name(cond));
    return 1;
}

static int l_item_slot_prop_condition_set(lua_State* L) {
    PcModItemSlotHandle* handle = (PcModItemSlotHandle*)luaL_checkudata(L, 1, PC_MOD_ITEM_SLOT_MT);
    Private_c* priv = pc_mod_item_slot_private(L, handle);
    u32 cond = pc_mod_read_item_condition(L, 3);
    mActor_name_t item = priv->inventory.pockets[handle->slot_index];

    mPr_SetPossessionItem(priv, handle->slot_index, item, cond);
    return 0;
}

static int l_pockets_newindex(lua_State* L) {
    Private_c* priv;
    int player_no;
    int slot;
    mActor_name_t item;

    luaL_checktype(L, 1, LUA_TTABLE);
    player_no = pc_mod_pockets_player_no(L, 1);

    if (player_no < 0 || player_no >= PLAYER_NUM) {
        luaL_error(L, "invalid Pockets");
    }

    priv = Save_GetPointer(private_data[player_no]);
    if (!priv || priv->exists == 0) {
        luaL_error(L, "invalid Pockets");
    }

    slot = pc_mod_check_pocket_slot(L, 2);
    if (pc_mod_read_item_from_value(L, 3, &item) == LUA_OK) {
        mPr_SetPossessionItem(priv, slot, item, mPr_ITEM_COND_NORMAL);
        pc_mod_push_item_slot(L, player_no, slot);
        lua_rawseti(L, 1, slot + 1);
    }

    return 0;
}

static const PcModPropertyDef item_slot_properties[] = {
    { "Item",      l_item_slot_prop_item_get,      l_item_slot_prop_item_set },
    { "Condition", l_item_slot_prop_condition_get, l_item_slot_prop_condition_set },
    { nullptr,     nullptr,                        nullptr },
};

extern "C" void pc_mod_register_item_slot_api(lua_State* L) {
    luaL_newmetatable(L, PC_MOD_ITEM_SLOT_MT);
    pc_mod_register_userdata(L, item_slot_properties, nullptr);
    pc_mod_lock_metatable(L);
    lua_pop(L, 1);

    luaL_newmetatable(L, PC_MOD_POCKETS_MT);
    lua_pushcfunction(L, l_pockets_newindex, "Pockets.__newindex");
    lua_setfield(L, -2, "__newindex");
    pc_mod_lock_metatable(L);
    lua_pop(L, 1);
}
