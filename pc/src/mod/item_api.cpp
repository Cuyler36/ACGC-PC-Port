#include "pc_mod_item_api.h"
#include "pc_mod_item_internal.h"
#include "pc_mod_item_slot_internal.h"
#include "pc_mod_save_internal.h"
#include "pc_mod_lua_internal.h"
#include "lualib.h"

extern "C" {
#include "m_item_name.h"
#include "m_name_table.h"
#include "m_msg.h"
#include "m_string.h"
#include "m_font.h"
}

static const char* pc_mod_item_kind_name(int name_type) {
    switch (name_type) {
        case NAME_TYPE_ITEM0:  return "item0";
        case NAME_TYPE_FTR0:
        case NAME_TYPE_FTR1:   return "furniture";
        case NAME_TYPE_ITEM1:  return "item1";
        case NAME_TYPE_WARP:   return "warp";
        case NAME_TYPE_STRUCT: return "struct";
        case NAME_TYPE_ITEM2:  return "item2";
        case NAME_TYPE_ACTOR:  return "actor";
        case NAME_TYPE_PROPS:  return "props";
        case NAME_TYPE_SPNPC:  return "spnpc";
        case NAME_TYPE_NPC:    return "npc";
        default:               return "unknown";
    }
}

static const char* pc_mod_item1_category_name(int category) {
    switch (category) {
        case ITEM1_CAT_PAPER:       return "paper";
        case ITEM1_CAT_MONEY:       return "money";
        case ITEM1_CAT_TOOL:        return "tool";
        case ITEM1_CAT_FISH:        return "fish";
        case ITEM1_CAT_CLOTH:       return "cloth";
        case ITEM1_CAT_ETC:         return "etc";
        case ITEM1_CAT_CARPET:      return "carpet";
        case ITEM1_CAT_WALL:        return "wall";
        case ITEM1_CAT_FRUIT:       return "fruit";
        case ITEM1_CAT_PLANT:       return "plant";
        case ITEM1_CAT_MINIDISK:    return "minidisk";
        case ITEM1_CAT_DUMMY:        return "dummy";
        case ITEM1_CAT_TICKET:      return "ticket";
        case ITEM1_CAT_INSECT:      return "insect";
        case ITEM1_CAT_HUKUBUKURO:   return "hukubukuro";
        case ITEM1_CAT_KABU:        return "kabu";
        default:                    return nullptr;
    }
}

void pc_mod_push_item(lua_State* L, mActor_name_t item_id) {
    PcModItemHandle* handle = (PcModItemHandle*)lua_newuserdata(L, sizeof(PcModItemHandle));
    handle->item_id = item_id;
    luaL_getmetatable(L, PC_MOD_ITEM_MT);
    lua_setmetatable(L, -2);
}

PcModItemHandle* pc_mod_check_item(lua_State* L, int idx) {
    return (PcModItemHandle*)luaL_checkudata(L, idx, PC_MOD_ITEM_MT);
}

PcModItemHandle* pc_mod_try_check_item(lua_State* L, int idx) {
    if (lua_type(L, idx) != LUA_TUSERDATA) {
        return nullptr;
    }

    void* ud = lua_touserdata(L, idx);
    if (ud == nullptr || !lua_getmetatable(L, idx)) {
        return nullptr;
    }

    luaL_getmetatable(L, PC_MOD_ITEM_MT);
    int matches = lua_rawequal(L, -1, -2);
    lua_pop(L, 2);

    return matches ? (PcModItemHandle*)ud : nullptr;
}

mActor_name_t pc_mod_item_id(PcModItemHandle* handle) {
    return handle ? handle->item_id : EMPTY_NO;
}

static mActor_name_t pc_mod_read_item_id(lua_State* L, int arg) {
    lua_Integer value = luaL_checkinteger(L, arg);

    if (value < 0 || value > 0xFFFF) {
        luaL_argerror(L, arg, "item id out of range");
    }

    return (mActor_name_t)value;
}

static int l_item_from_id(lua_State* L) {
    pc_mod_push_item(L, pc_mod_read_item_id(L, 1));
    return 1;
}

static int l_item_empty(lua_State* L) {
    pc_mod_push_item(L, EMPTY_NO);
    return 1;
}

static int l_item_prop_id(lua_State* L) {
    PcModItemHandle* handle = pc_mod_check_item(L, 1);
    lua_pushinteger(L, handle->item_id);
    return 1;
}

static void pc_mod_push_item_name(lua_State* L, mActor_name_t item_id) {
    u8 name[mIN_ITEM_NAME_LEN];

    mIN_copy_name_str(name, item_id);
    pc_mod_push_game_string(L, name, mMsg_Get_Length_String(name, mIN_ITEM_NAME_LEN));
}

static void pc_mod_push_item_article(lua_State* L, int article_type) {
    if (article_type == mIN_ARTICLE_NONE) {
        lua_pushstring(L, "");
        return;
    }

    u8 article[mMsg_FREE_STRING_LEN];

    mString_Load_ArticleFromRom(article, mMsg_FREE_STRING_LEN, article_type);
    pc_mod_push_game_string(L, article, mMsg_Get_Length_String(article, mMsg_FREE_STRING_LEN));
}

static void pc_mod_push_item_full_name(lua_State* L, mActor_name_t item_id) {
    u8 full_name[256];
    int article_type = mIN_get_item_article(item_id);
    int len = 0;

    if (article_type != mIN_ARTICLE_NONE) {
        mString_Load_ArticleFromRom(full_name, sizeof(full_name), article_type);
        len = mMsg_Get_Length_String(full_name, sizeof(full_name));
        full_name[len++] = CHAR_SPACE;
    }

    mIN_copy_name_str(full_name + len, item_id);
    len += mMsg_Get_Length_String(full_name + len, mIN_ITEM_NAME_LEN);
    pc_mod_push_game_string(L, full_name, len);
}

static int l_item_prop_name(lua_State* L) {
    PcModItemHandle* handle = pc_mod_check_item(L, 1);
    pc_mod_push_item_name(L, handle->item_id);
    return 1;
}

static int l_item_prop_is_empty(lua_State* L) {
    PcModItemHandle* handle = pc_mod_check_item(L, 1);
    lua_pushboolean(L, handle->item_id == EMPTY_NO);
    return 1;
}

static int l_item_prop_kind(lua_State* L) {
    PcModItemHandle* handle = pc_mod_check_item(L, 1);
    lua_pushstring(L, pc_mod_item_kind_name(ITEM_NAME_GET_TYPE(handle->item_id)));
    return 1;
}

static int l_item_prop_category(lua_State* L) {
    PcModItemHandle* handle = pc_mod_check_item(L, 1);

    if (ITEM_NAME_GET_TYPE(handle->item_id) != NAME_TYPE_ITEM1) {
        lua_pushnil(L);
        return 1;
    }

    const char* category = pc_mod_item1_category_name(ITEM_NAME_GET_CAT(handle->item_id));
    if (category == nullptr) {
        lua_pushnil(L);
        return 1;
    }

    lua_pushstring(L, category);
    return 1;
}

static int l_item_prop_article(lua_State* L) {
    PcModItemHandle* handle = pc_mod_check_item(L, 1);
    pc_mod_push_item_article(L, mIN_get_item_article(handle->item_id));
    return 1;
}

static int l_item_prop_full_name(lua_State* L) {
    PcModItemHandle* handle = pc_mod_check_item(L, 1);
    pc_mod_push_item_full_name(L, handle->item_id);
    return 1;
}

static int l_item_method_get_full_name(lua_State* L) {
    PcModItemHandle* handle = pc_mod_check_item(L, 1);
    pc_mod_push_item_full_name(L, handle->item_id);
    return 1;
}

static const PcModPropertyDef item_properties[] = {
    { "Id",        l_item_prop_id,        nullptr },
    { "Name",      l_item_prop_name,      nullptr },
    { "FullName",  l_item_prop_full_name, nullptr },
    { "IsEmpty",   l_item_prop_is_empty,  nullptr },
    { "Kind",      l_item_prop_kind,      nullptr },
    { "Category",  l_item_prop_category,  nullptr },
    { "Article",   l_item_prop_article,   nullptr },
    { nullptr,     nullptr,               nullptr },
};

static const luaL_Reg item_methods[] = {
    { "GetFullName", l_item_method_get_full_name },
    { nullptr, nullptr },
};

static const luaL_Reg item_module[] = {
    { "fromId", l_item_from_id },
    { "empty",  l_item_empty },
    { nullptr, nullptr },
};

extern "C" void pc_mod_register_item_api(lua_State* L) {
    luaL_newmetatable(L, PC_MOD_ITEM_MT);
    pc_mod_register_userdata(L, item_properties, item_methods);
    pc_mod_lock_metatable(L);
    lua_pop(L, 1);

    pc_mod_register_item_slot_api(L);

    luaL_register(L, "Item", item_module);
    lua_pop(L, 1);
}
