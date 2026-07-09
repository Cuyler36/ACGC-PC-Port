#include "pc_mod_private_api.h"
#include "pc_mod_private_internal.h"
#include "pc_mod_save_internal.h"
#include "pc_mod_item_internal.h"
#include "pc_mod_item_slot_internal.h"
#include "pc_mod_lua_internal.h"
#include "lualib.h"

extern "C" {
#include "m_common_data.h"
#include "m_personal_id.h"
#include "m_private.h"
}

Private_c* pc_mod_private_data(PcModPrivateHandle* handle) {
    if (!handle || handle->player_no < 0 || handle->player_no >= PLAYER_NUM) {
        return nullptr;
    }
    return Save_GetPointer(private_data[handle->player_no]);
}

static int pc_mod_private_is_live(int player_no) {
    Private_c* priv;

    if (player_no < 0 || player_no >= PLAYER_NUM) {
        return 0;
    }

    priv = Save_GetPointer(private_data[player_no]);
    return priv->exists != 0;
}

void pc_mod_push_private(lua_State* L, int player_no) {
    PcModPrivateHandle* handle;

    if (!pc_mod_private_is_live(player_no)) {
        lua_pushnil(L);
        return;
    }

    handle = (PcModPrivateHandle*)lua_newuserdata(L, sizeof(PcModPrivateHandle));
    handle->player_no = player_no;
    luaL_getmetatable(L, PC_MOD_PRIVATE_MT);
    lua_setmetatable(L, -2);
}

PcModPrivateHandle* pc_mod_check_private(lua_State* L, int idx) {
    return (PcModPrivateHandle*)luaL_checkudata(L, idx, PC_MOD_PRIVATE_MT);
}

static Private_c* pc_mod_require_private(lua_State* L, int idx) {
    PcModPrivateHandle* handle = pc_mod_check_private(L, idx);
    Private_c* priv = pc_mod_private_data(handle);

    if (!priv || priv->exists == 0) {
        luaL_error(L, "invalid Private handle");
    }

    return priv;
}

static int l_private_from_index(lua_State* L) {
    int player_no = pc_mod_check_player_index(L, 1, 0);
    pc_mod_push_private(L, player_no);
    return 1;
}

static int l_private_current(lua_State* L) {
    pc_mod_push_private(L, (int)Common_Get(player_no));
    return 1;
}

static int l_private_is_ready(lua_State* L) {
    lua_pushboolean(L, pc_mod_save_is_ready());
    return 1;
}

static int l_private_prop_index(lua_State* L) {
    PcModPrivateHandle* handle = pc_mod_check_private(L, 1);
    lua_pushinteger(L, handle->player_no);
    return 1;
}

static int l_private_prop_valid(lua_State* L) {
    PcModPrivateHandle* handle = pc_mod_check_private(L, 1);
    lua_pushboolean(L, pc_mod_private_is_live(handle->player_no));
    return 1;
}

static int l_private_prop_wallet_get(lua_State* L) {
    Private_c* priv = pc_mod_require_private(L, 1);
    lua_pushinteger(L, (lua_Integer)priv->inventory.wallet);
    return 1;
}

static int l_private_prop_wallet_set(lua_State* L) {
    Private_c* priv = pc_mod_require_private(L, 1);
    lua_Integer amount = luaL_checkinteger(L, 3);

    if (amount < 0) {
        amount = 0;
    } else if (amount > mPr_WALLET_MAX) {
        amount = mPr_WALLET_MAX;
    }

    priv->inventory.wallet = (u32)amount;
    return 0;
}

static int l_private_prop_name(lua_State* L) {
    PcModPrivateHandle* handle = pc_mod_check_private(L, 1);
    u8 name[PLAYER_NAME_LEN];

    mPr_GetPlayerName(name, handle->player_no);
    pc_mod_push_game_string(L, name, PLAYER_NAME_LEN);
    return 1;
}

static int l_private_prop_loan(lua_State* L) {
    Private_c* priv = pc_mod_require_private(L, 1);
    lua_pushinteger(L, (lua_Integer)priv->inventory.loan);
    return 1;
}

static int l_private_prop_pockets(lua_State* L) {
    PcModPrivateHandle* handle = pc_mod_check_private(L, 1);
    pc_mod_require_private(L, 1);
    pc_mod_push_pockets(L, handle->player_no);
    return 1;
}

static int l_private_prop_equipped_get(lua_State* L) {
    Private_c* priv = pc_mod_require_private(L, 1);
    pc_mod_push_item(L, priv->equipment);
    return 1;
}

static int l_private_prop_equipped_set(lua_State* L) {
    Private_c* priv = pc_mod_require_private(L, 1);
    mActor_name_t item;

    if (pc_mod_read_item_from_value(L, 3, &item) == LUA_OK) {
        priv->equipment = item;
    }

    return 0;
}

static const PcModPropertyDef private_properties[] = {
    { "Index",    l_private_prop_index,          nullptr },
    { "Valid",    l_private_prop_valid,          nullptr },
    { "Wallet",   l_private_prop_wallet_get,     l_private_prop_wallet_set },
    { "Name",     l_private_prop_name,           nullptr },
    { "Loan",     l_private_prop_loan,           nullptr },
    { "Pockets",  l_private_prop_pockets,        nullptr },
    { "Equipped", l_private_prop_equipped_get,   l_private_prop_equipped_set },
    { nullptr,    nullptr,                       nullptr },
};

static const luaL_Reg private_module[] = {
    { "fromIndex", l_private_from_index },
    { "current",   l_private_current },
    { "isReady",   l_private_is_ready },
    { nullptr, nullptr },
};

extern "C" void pc_mod_register_private_api(lua_State* L) {
    luaL_newmetatable(L, PC_MOD_PRIVATE_MT);
    pc_mod_register_userdata_properties(L, private_properties);
    pc_mod_lock_metatable(L);
    lua_pop(L, 1);

    luaL_register(L, "Private", private_module);
    lua_pop(L, 1);
}
