#include "pc_mod_msg_api.h"
#include "pc_mod_lua_internal.h"
#include "mod/pc_mod_actor_internal.h"

#include "m_msg.h"

static mMsg_Window_c* pc_mod_msg_window(void) {
    return mMsg_Get_base_window_p();
}

static int pc_mod_msg_is_open(mMsg_Window_c* msg_p) {
    return mMsg_Check_MainNormal(msg_p) || mMsg_Check_MainNormalContinue(msg_p);
}

static int pc_mod_msg_can_open(mMsg_Window_c* msg_p) {
    return mMsg_Check_main_hide(msg_p) || mMsg_Check_main_wait(msg_p);
}

static void pc_mod_msg_require_play(lua_State* L) {
    if (!pc_mod_try_get_play()) {
        luaL_error(L, "Message API requires active gameplay");
    }
}

static rgba_t pc_mod_msg_default_window_color(void) {
    rgba_t color;

    color.r = 235;
    color.g = 255;
    color.b = 235;
    color.a = 255;
    return color;
}

static int pc_mod_msg_api_open_msg_window_ex(int clear_flag, int msg_no_hint) {
    mMsg_Window_c* msg_p = pc_mod_msg_window();

    if (mMsg_Check_main_wait(msg_p)) {
        return mMsg_request_main_appear_wait_type2(msg_p, clear_flag) ? 1 : 0;
    }

    if (mMsg_Check_main_hide(msg_p)) {
        int msg_no = msg_no_hint;

        if (msg_no < 0) {
            msg_no = mMsg_Get_msg_num(msg_p);
        }

        if (msg_no < 0) {
            return 0;
        }

        rgba_t window_color = pc_mod_msg_default_window_color();
        return mMsg_request_main_appear(msg_p, NULL, FALSE, &window_color, msg_no, 5) ? 1 : 0;
    }

    return 0;
}

static int pc_mod_msg_api_open_msg_window(int clear_flag) {
    return pc_mod_msg_api_open_msg_window_ex(clear_flag, -1);
}

static int pc_mod_msg_api_close_msg_window(void) {
    mMsg_Window_c* msg_p = pc_mod_msg_window();

    if (!pc_mod_msg_is_open(msg_p)) {
        return 0;
    }

    return mMsg_request_main_disappear_wait_type1(msg_p) ? 1 : 0;
}

static int pc_mod_msg_api_set_msg_idx(int msg_idx) {
    return mMsg_ChangeMsgData(pc_mod_msg_window(), msg_idx) ? 1 : 0;
}

static int l_msg_is_open(lua_State* L) {
    if (!pc_mod_try_get_play()) {
        lua_pushboolean(L, 0);
        return 1;
    }

    lua_pushboolean(L, pc_mod_msg_is_open(pc_mod_msg_window()));
    return 1;
}

static int l_msg_can_open(lua_State* L) {
    if (!pc_mod_try_get_play()) {
        lua_pushboolean(L, 0);
        return 1;
    }

    lua_pushboolean(L, pc_mod_msg_can_open(pc_mod_msg_window()));
    return 1;
}

static int l_msg_msg_id(lua_State* L) {
    if (!pc_mod_try_get_play()) {
        lua_pushinteger(L, 0);
        return 1;
    }

    lua_pushinteger(L, mMsg_Get_msg_num(pc_mod_msg_window()));
    return 1;
}

static int l_msg_open(lua_State* L) {
    int clear_flag = lua_toboolean(L, 1);

    pc_mod_msg_require_play(L);
    lua_pushboolean(L, pc_mod_msg_api_open_msg_window(clear_flag));
    return 1;
}

static int l_msg_close(lua_State* L) {
    pc_mod_msg_require_play(L);
    lua_pushboolean(L, pc_mod_msg_api_close_msg_window());
    return 1;
}

static int l_msg_set_msg_id(lua_State* L) {
    int msg_idx = luaL_checkinteger(L, 1);

    pc_mod_msg_require_play(L);
    lua_pushboolean(L, pc_mod_msg_api_set_msg_idx(msg_idx));
    return 1;
}

static int l_msg_open_with_id(lua_State* L) {
    int msg_idx = luaL_checkinteger(L, 1);

    pc_mod_msg_require_play(L);

    if (!pc_mod_msg_api_set_msg_idx(msg_idx)) {
        lua_pushboolean(L, 0);
        return 1;
    }

    lua_pushboolean(L, pc_mod_msg_api_open_msg_window_ex(FALSE, msg_idx));
    return 1;
}

static const luaL_Reg msg_module[] = {
    { "IsOpen",      l_msg_is_open },
    { "CanOpen",     l_msg_can_open },
    { "GetMsgId",    l_msg_msg_id },
    { "Open",        l_msg_open },
    { "Close",       l_msg_close },
    { "SetMsgId",    l_msg_set_msg_id },
    { "OpenWithId",  l_msg_open_with_id },
    { nullptr,       nullptr },
};

extern "C" void pc_mod_register_msg_api(lua_State* L) {
    luaL_register(L, "Message", msg_module);
    lua_pop(L, 1);
}
