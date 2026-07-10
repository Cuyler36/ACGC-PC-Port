#include "pc_mod_player_api.h"
#include "pc_mod_actor_internal.h"
#include "lualib.h"
#include <cstring>
#include <cctype>

extern "C" {
#include "m_player.h"
#include "m_player_lib.h"
#include "c_keyframe.h"
}

static int s_pending_anim = -1;
static int s_pending_loop = 0;
static float s_pending_speed = 0.5f;
static float s_pending_morph = -5.0f;

/* Names align with mPlayer_ANIM_* enum order. */
static const char* const s_anim_names[mPlayer_ANIM_NUM] = {
    "wait1",           "walk1",           "axe1",            "run1",
    "push1",           "pull1",           "hold_wait1",      "pickup1",
    "lturn1",          "rturn1",          "get1",            "get_change1",
    "get_putaway1",    "open1",           "putaway1",        "trans_wait1",
    "transfer1",       "umb_open1",       "umbrella1",       "dash1",
    "run_slip1",       "get_pull1",       "get_m1",          "kamae_move_m1",
    "kamae_wait_m1",   "kokeru_a1",       "kokeru_getup_a1", "kokeru_getup_n1",
    "kokeru_n1",       "net1",            "net_swing1",      "axe_swing1",
    "kamae_slip_m1",   "kokeru1",         "kokeru_getup1",   "sitdown1",
    "sitdown_wait1",   "standup1",        "putaway_m1",      "bed_wait1",
    "inbed_l1",        "inbed_r1",        "intrain1",        "kagu_open_d1",
    "kagu_open_h1",    "kagu_open_k1",    "negaeri_l1",      "negaeri_r1",
    "outbed_l1",       "outbed_r1",       "outtrain1",       "kagu_close_d1",
    "kagu_close_h1",   "kagu_close_k1",   "kagu_wait_d1",    "kagu_wait_h1",
    "kagu_wait_k1",    "go_out_o1",       "go_out_s1",       "into_s1",
    "axe_hane1",       "axe_suka1",       "hold_wait_h1",    "hold_wait_o1",
    "get_t1",          "get_t2",          "putaway_t1",      "sao1",
    "sao_swing1",      "turi_hiki1",      "turi_wait1",      "not_get_t1",
    "menu_catch1",     "menu_change1",    "umb_close1",      "not_sao_swing1",
    "intrain_wait1",   "clear_table1",    "dig1",            "fill_up1",
    "not_dig1",        "clear_table_l1",  "pickup_l1",       "scoop1",
    "confirm1",        "dig_suka1",       "get_d1",          "putaway_d1",
    "dig_kabu1",       "fill_up_i1",      "send_mail1",      "get_f1",
    "get_pull_f1",     "get_putaway_f1",  "trans_wait_f1",   "transfer_f1",
    "shake1",          "tired1",          "wash1",           "wash2",
    "wash3",           "wash4",           "wash5",           "fukubiki1",
    "omairi1",         "saisen1",         "return_mail1",    "return_mail2",
    "return_mail3",    "eat1",            "gaaan1",          "gaaan2",
    "deru1",           "guratuku1",       "mogaku1",         "otiru1",
    "zassou1",         "knock1",          "biku1",           "hati1",
    "hati2",           "hati3",           "push_yuki1",      "deru2",
    "otiru2",          "itazura1",        "umb_rot1",        "pickup_wait1",
    "yatta1",          "yatta2",          "yatta3",          "kaza1",
    "mosquito1",       "mosquito2",       "ride1",           "ride2",
    "ridewait",        "getoff1",         "getoff2",         "utiwa_wait1",
    "utiwa_d1",        "axe_break1",      "axe_breakwait1",  "light_on1",
    "taisou1",         "taisou2_1",       "taisou2_2",       "taisou3",
    "taisou4_1",       "taisou4_2",       "taisou5_1",       "taisou5_2",
    "taisou6_1",       "taisou6_2",       "taisou7_1",       "taisou7_2",
    "omairi_us1",
};

static_assert(sizeof(s_anim_names) / sizeof(s_anim_names[0]) == (size_t)mPlayer_ANIM_NUM,
              "player animation name table must match mPlayer_ANIM_NUM");

static int pc_mod_anim_name_eq(const char* a, const char* b) {
    while (*a && *b) {
        char ca = (char)tolower((unsigned char)*a++);
        char cb = (char)tolower((unsigned char)*b++);
        if (ca != cb) {
            return 0;
        }
    }
    return *a == '\0' && *b == '\0';
}

static int pc_mod_resolve_anim_name(const char* name) {
    const char* p = name;
    int i;

    if (strncmp(p, "mPlayer_ANIM_", 13) == 0 || strncmp(p, "mplayer_anim_", 13) == 0) {
        p += 13;
    }

    for (i = 0; i < mPlayer_ANIM_NUM; i++) {
        if (pc_mod_anim_name_eq(p, s_anim_names[i])) {
            return i;
        }
    }

    return -1;
}

static int pc_mod_player_arg_offset(lua_State* L) {
    /* Support both player.PlayAnimation(...) and player:PlayAnimation(...). */
    if (lua_istable(L, 1) || lua_isuserdata(L, 1)) {
        return 1;
    }
    return 0;
}

static int pc_mod_player_apply_animation(PLAYER_ACTOR* player, int anim_idx, int loop, float speed, float morph) {
    cKF_Animation_R_c* anim;
    int mode;

    if (!player || anim_idx < 0 || anim_idx >= mPlayer_ANIM_NUM) {
        return 0;
    }

    anim = mPlib_Get_Pointer_Animation(anim_idx);
    if (!anim) {
        return 0;
    }

    mode = loop ? cKF_FRAMECONTROL_REPEAT : cKF_FRAMECONTROL_STOP;

    player->animation0_idx = anim_idx;
    player->animation1_idx = anim_idx;
    if (player->part_table_idx != mPlayer_PART_TABLE_NORMAL) {
        player->part_table_idx = mPlayer_PART_TABLE_NORMAL;
        mPlib_DMA_player_Part_Table(player->part_table, mPlayer_PART_TABLE_NORMAL);
    }

    cKF_SkeletonInfo_R_init_standard_setframeandspeedandmorphandmode(&player->keyframe0, anim, NULL, 1.0f, speed, morph,
                                                                     mode);
    cKF_SkeletonInfo_R_init_standard_setframeandspeedandmorphandmode(&player->keyframe1, anim, NULL, 1.0f, speed, morph,
                                                                     mode);
    return 1;
}

void pc_mod_player_apply_pending_animation(void) {
    GAME_PLAY* play;
    PLAYER_ACTOR* player;

    if (s_pending_anim < 0) {
        return;
    }

    play = pc_mod_try_get_play();
    if (!play) {
        return;
    }

    player = get_player_actor_withoutCheck(play);
    if (!player) {
        return;
    }

    /* Apply only after the player actor has settled into wait for this frame.
       Setup runs during Game_play_move, so this must be called from post_move. */
    if (player->now_main_index != mPlayer_INDEX_WAIT) {
        return;
    }

    if (pc_mod_player_apply_animation(player, s_pending_anim, s_pending_loop, s_pending_speed, s_pending_morph)) {
        s_pending_anim = -1;
    }
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

static int l_player_play_animation(lua_State* L) {
    int off = pc_mod_player_arg_offset(L);
    int anim_arg = 1 + off;
    int opts_arg = 2 + off;
    GAME_PLAY* play;
    PLAYER_ACTOR* player;
    int anim_idx = -1;
    int loop = 0;
    float speed = 0.5f;
    float morph = -5.0f;

    play = pc_mod_try_get_play();
    if (!play) {
        luaL_error(L, "PlayAnimation requires active gameplay");
    }

    player = get_player_actor_withoutCheck(play);
    if (!player) {
        luaL_error(L, "PlayAnimation requires an active player");
    }

    if (lua_type(L, anim_arg) == LUA_TSTRING) {
        const char* name = luaL_checkstring(L, anim_arg);
        anim_idx = pc_mod_resolve_anim_name(name);
        if (anim_idx < 0) {
            luaL_argerror(L, anim_arg, "unknown player animation name");
        }
    } else {
        anim_idx = (int)luaL_checkinteger(L, anim_arg);
        if (anim_idx < 0 || anim_idx >= mPlayer_ANIM_NUM) {
            luaL_argerror(L, anim_arg, "animation index out of range");
        }
    }

    if (lua_istable(L, opts_arg)) {
        lua_getfield(L, opts_arg, "loop");
        if (!lua_isnil(L, -1)) {
            loop = lua_toboolean(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, opts_arg, "speed");
        if (lua_isnumber(L, -1)) {
            speed = (float)lua_tonumber(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, opts_arg, "morph");
        if (lua_isnumber(L, -1)) {
            morph = (float)lua_tonumber(L, -1);
        }
        lua_pop(L, 1);
    }

    s_pending_anim = anim_idx;
    s_pending_loop = loop;
    s_pending_speed = speed;
    s_pending_morph = morph;

    /* Use normal wait (not demo-wait) so stick input can reclaim control.
       Animation is applied in post_move after wait setup finishes. */
    player->request_main_wait_all_proc((GAME*)play, -5.0f, 0.0f, 0, mPlayer_REQUEST_PRIORITY_45);

    lua_pushboolean(L, 1);
    return 1;
}

static int l_player_get_animation_name(lua_State* L) {
    int off = pc_mod_player_arg_offset(L);
    int anim_idx = (int)luaL_checkinteger(L, 1 + off);

    if (anim_idx < 0 || anim_idx >= mPlayer_ANIM_NUM) {
        lua_pushnil(L);
        return 1;
    }

    lua_pushstring(L, s_anim_names[anim_idx]);
    return 1;
}

static int l_player_get_animation_count(lua_State* L) {
    (void)pc_mod_player_arg_offset(L);
    lua_pushinteger(L, mPlayer_ANIM_NUM);
    return 1;
}

static const luaL_Reg playerlib[] = {
    { "get",                l_player_get },
    { "get_item_kind",      l_player_get_item_kind },
    { "PlayAnimation",      l_player_play_animation },
    { "GetAnimationName",   l_player_get_animation_name },
    { "GetAnimationCount",  l_player_get_animation_count },
    { nullptr, nullptr },
};

extern "C" void pc_mod_register_player_api(lua_State* L) {
    luaL_register(L, "player", playerlib);
    lua_pop(L, 1);
}
