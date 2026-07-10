#include "pc_mod_scene_api.h"
#include "mod/pc_mod_actor_internal.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

extern "C" {
#include "m_common_data.h"
#include "m_name_table.h"
#include "m_play.h"
#include "m_player_lib.h"
#include "m_scene.h"
#include "m_scene_table.h"
}

typedef struct {
    const char* name;
    int scene_id;
    s16 spawn_x;
    s16 spawn_y;
    s16 spawn_z;
    u8 facing;
} PcModSceneInfo;

/* Canonical names match SCENE_* enum (without prefix), plus a few aliases. */
static const PcModSceneInfo g_scene_info[] = {
    { "test1",                SCENE_TEST1,                160, 0, 300, mSc_DIRECT_NORTH },
    { "test2",                SCENE_TEST2,                160, 0, 300, mSc_DIRECT_NORTH },
    { "test3",                SCENE_TEST3,                160, 0, 300, mSc_DIRECT_NORTH },
    { "water_test",           SCENE_WATER_TEST,           160, 0, 300, mSc_DIRECT_NORTH },
    { "footprint_test",       SCENE_FOOTPRINT_TEST,       160, 0, 300, mSc_DIRECT_NORTH },
    { "npc_test",             SCENE_NPC_TEST,             160, 0, 300, mSc_DIRECT_NORTH },
    { "npc_house",            SCENE_NPC_HOUSE,            160, 0, 300, mSc_DIRECT_NORTH },
    { "fg",                   SCENE_FG,                  1979, 200, 760, mSc_DIRECT_SOUTH },
    { "outdoors",             SCENE_FG,                  1979, 200, 760, mSc_DIRECT_SOUTH },
    { "town",                 SCENE_FG,                  1979, 200, 760, mSc_DIRECT_SOUTH },
    { "random_npc_test",      SCENE_RANDOM_NPC_TEST,      160, 0, 300, mSc_DIRECT_NORTH },
    { "shop0",                SCENE_SHOP0,                160, 0, 300, mSc_DIRECT_NORTH },
    { "nooks_cranny",         SCENE_SHOP0,                160, 0, 300, mSc_DIRECT_NORTH },
    { "bg_test_no_river",     SCENE_BG_TEST_NO_RIVER,     160, 0, 300, mSc_DIRECT_NORTH },
    { "bg_test_river",        SCENE_BG_TEST_RIVER,        160, 0, 300, mSc_DIRECT_NORTH },
    { "broker_shop",          SCENE_BROKER_SHOP,          160, 0, 300, mSc_DIRECT_NORTH },
    { "redd",                 SCENE_BROKER_SHOP,          160, 0, 300, mSc_DIRECT_NORTH },
    { "field_tool_inside",    SCENE_FIELD_TOOL_INSIDE,    160, 0, 300, mSc_DIRECT_NORTH },
    { "post_office",          SCENE_POST_OFFICE,          160, 0, 300, mSc_DIRECT_NORTH },
    { "start_demo",           SCENE_START_DEMO,           120, 0, 340, mSc_DIRECT_NORTH },
    { "start_demo2",          SCENE_START_DEMO2,          160, 0, 300, mSc_DIRECT_NORTH },
    { "police_box",           SCENE_POLICE_BOX,           160, 0, 300, mSc_DIRECT_NORTH },
    { "buggy",                SCENE_BUGGY,                160, 0, 300, mSc_DIRECT_NORTH },
    { "playerselect",         SCENE_PLAYERSELECT,         160, 0, 300, mSc_DIRECT_NORTH },
    { "my_room_s",            SCENE_MY_ROOM_S,            160, 0, 300, mSc_DIRECT_NORTH },
    { "my_room_m",            SCENE_MY_ROOM_M,            160, 0, 300, mSc_DIRECT_NORTH },
    { "my_room_l",            SCENE_MY_ROOM_L,            160, 0, 300, mSc_DIRECT_NORTH },
    { "conveni",              SCENE_CONVENI,              160, 0, 300, mSc_DIRECT_NORTH },
    { "nook_n_go",            SCENE_CONVENI,              160, 0, 300, mSc_DIRECT_NORTH },
    { "super",                SCENE_SUPER,                320, 0, 460, mSc_DIRECT_NORTH },
    { "nookway",              SCENE_SUPER,                320, 0, 460, mSc_DIRECT_NORTH },
    { "depart",               SCENE_DEPART,               160, 0, 300, mSc_DIRECT_NORTH },
    { "nookingtons",          SCENE_DEPART,               160, 0, 300, mSc_DIRECT_NORTH },
    { "test5",                SCENE_TEST5,                160, 0, 300, mSc_DIRECT_NORTH },
    { "playerselect_2",       SCENE_PLAYERSELECT_2,       160, 0, 300, mSc_DIRECT_NORTH },
    { "playerselect_3",       SCENE_PLAYERSELECT_3,       160, 0, 300, mSc_DIRECT_NORTH },
    { "depart_2",             SCENE_DEPART_2,             160, 0, 300, mSc_DIRECT_NORTH },
    { "event_announcement",   SCENE_EVENT_ANNOUNCEMENT,   160, 0, 300, mSc_DIRECT_NORTH },
    { "kamakura",             SCENE_KAMAKURA,             160, 0, 300, mSc_DIRECT_NORTH },
    { "field_tool",           SCENE_FIELD_TOOL,           160, 0, 300, mSc_DIRECT_NORTH },
    { "title_demo",           SCENE_TITLE_DEMO,          2180, 200, 824, mSc_DIRECT_NORTH },
    { "playerselect_save",    SCENE_PLAYERSELECT_SAVE,    160, 0, 300, mSc_DIRECT_NORTH },
    { "museum_entrance",      SCENE_MUSEUM_ENTRANCE,      240, 0, 440, mSc_DIRECT_NORTH },
    { "museum",               SCENE_MUSEUM_ENTRANCE,      240, 0, 440, mSc_DIRECT_NORTH },
    { "museum_room_painting", SCENE_MUSEUM_ROOM_PAINTING, 160, 0, 300, mSc_DIRECT_NORTH },
    { "museum_room_fossil",   SCENE_MUSEUM_ROOM_FOSSIL,   160, 0, 300, mSc_DIRECT_NORTH },
    { "museum_room_insect",   SCENE_MUSEUM_ROOM_INSECT,   160, 0, 300, mSc_DIRECT_NORTH },
    { "museum_room_fish",     SCENE_MUSEUM_ROOM_FISH,     160, 0, 300, mSc_DIRECT_NORTH },
    { "my_room_ll1",          SCENE_MY_ROOM_LL1,          160, 0, 300, mSc_DIRECT_NORTH },
    { "my_room_ll2",          SCENE_MY_ROOM_LL2,          160, 0, 300, mSc_DIRECT_NORTH },
    { "my_room_basement_s",   SCENE_MY_ROOM_BASEMENT_S,   160, 0, 300, mSc_DIRECT_NORTH },
    { "my_room_basement_m",   SCENE_MY_ROOM_BASEMENT_M,   160, 0, 300, mSc_DIRECT_NORTH },
    { "my_room_basement_l",   SCENE_MY_ROOM_BASEMENT_L,   160, 0, 300, mSc_DIRECT_NORTH },
    { "my_room_basement_ll1", SCENE_MY_ROOM_BASEMENT_LL1, 160, 0, 300, mSc_DIRECT_NORTH },
    { "needlework",           SCENE_NEEDLEWORK,           160, 0, 300, mSc_DIRECT_NORTH },
    { "able_sisters",         SCENE_NEEDLEWORK,           160, 0, 300, mSc_DIRECT_NORTH },
    { "cottage_my",           SCENE_COTTAGE_MY,           200, 0, 380, mSc_DIRECT_NORTH },
    { "cottage_npc",          SCENE_COTTAGE_NPC,          160, 0, 300, mSc_DIRECT_NORTH },
    { "start_demo3",          SCENE_START_DEMO3,          160, 0, 300, mSc_DIRECT_NORTH },
    { "lighthouse",           SCENE_LIGHTHOUSE,           160, 0, 300, mSc_DIRECT_NORTH },
    { "tent",                 SCENE_TENT,                 160, 0, 300, mSc_DIRECT_NORTH },
};

static void pc_mod_scene_normalize_name(const char* src, char* dst, size_t dst_len) {
    size_t i = 0;
    size_t o = 0;

    while (src[i] != '\0' && o + 1 < dst_len) {
        char c = (char)tolower((unsigned char)src[i]);
        if (c == '-' || c == ' ') {
            c = '_';
        }
        if ((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '_') {
            dst[o++] = c;
        }
        i++;
    }
    dst[o] = '\0';

    if (strncmp(dst, "scene_", 6) == 0) {
        memmove(dst, dst + 6, strlen(dst + 6) + 1);
    }
}

static const PcModSceneInfo* pc_mod_scene_info_by_id(int scene_id) {
    for (size_t i = 0; i < sizeof(g_scene_info) / sizeof(g_scene_info[0]); i++) {
        if (g_scene_info[i].scene_id == scene_id) {
            return &g_scene_info[i];
        }
    }
    return nullptr;
}

static const PcModSceneInfo* pc_mod_scene_info_by_name(const char* name) {
    char normalized[64];

    pc_mod_scene_normalize_name(name, normalized, sizeof(normalized));
    if (normalized[0] == '\0') {
        return nullptr;
    }

    for (size_t i = 0; i < sizeof(g_scene_info) / sizeof(g_scene_info[0]); i++) {
        if (strcmp(g_scene_info[i].name, normalized) == 0) {
            return &g_scene_info[i];
        }
    }
    return nullptr;
}

static int pc_mod_scene_resolve(lua_State* L, int idx, int* out_scene_id, const PcModSceneInfo** out_info) {
    const PcModSceneInfo* info = nullptr;
    int scene_id = -1;

    if (lua_isnumber(L, idx)) {
        scene_id = (int)luaL_checkinteger(L, idx);
        if (scene_id < 0 || scene_id >= SCENE_NUM) {
            luaL_error(L, "scene id %d out of range (0-%d)", scene_id, SCENE_NUM - 1);
        }
        info = pc_mod_scene_info_by_id(scene_id);
    } else if (lua_isstring(L, idx)) {
        const char* name = luaL_checkstring(L, idx);
        info = pc_mod_scene_info_by_name(name);
        if (info == nullptr) {
            luaL_error(L, "unknown scene name '%s'", name);
        }
        scene_id = info->scene_id;
    } else {
        luaL_error(L, "gotoscene expects a scene id (number) or name (string)");
    }

    *out_scene_id = scene_id;
    *out_info = info;
    return 0;
}

static int pc_mod_gotoscene(lua_State* L) {
    GAME_PLAY* play;
    Door_data_c door;
    const PcModSceneInfo* info = nullptr;
    int scene_id = -1;
    int update_player_mode = FALSE;
    int result;

    play = pc_mod_try_get_play();
    if (play == nullptr) {
        luaL_error(L, "gotoscene requires active gameplay");
    }

    pc_mod_scene_resolve(L, 1, &scene_id, &info);

    memset(&door, 0, sizeof(door));
    door.next_scene_id = scene_id;
    door.exit_orientation = info != nullptr ? info->facing : mSc_DIRECT_NORTH;
    door.exit_type = FALSE;
    door.extra_data = 0;
    door.exit_position.x = info != nullptr ? info->spawn_x : 160;
    door.exit_position.y = info != nullptr ? info->spawn_y : 0;
    door.exit_position.z = info != nullptr ? info->spawn_z : 300;
    door.door_actor_name = EMPTY_NO;
    door.wipe_type = WIPE_TYPE_FADE_BLACK;

    if (lua_istable(L, 2)) {
        lua_getfield(L, 2, "x");
        if (lua_isnumber(L, -1)) {
            door.exit_position.x = (s16)lua_tointeger(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, 2, "y");
        if (lua_isnumber(L, -1)) {
            door.exit_position.y = (s16)lua_tointeger(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, 2, "z");
        if (lua_isnumber(L, -1)) {
            door.exit_position.z = (s16)lua_tointeger(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, 2, "facing");
        if (lua_isnumber(L, -1)) {
            door.exit_orientation = (u8)lua_tointeger(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, 2, "wipe");
        if (lua_isnumber(L, -1)) {
            door.wipe_type = (u8)lua_tointeger(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, 2, "updatePlayer");
        if (lua_isboolean(L, -1)) {
            update_player_mode = lua_toboolean(L, -1);
        }
        lua_pop(L, 1);
    }

    /* Keep current outdoor position when warping to FG without an explicit spawn. */
    if (scene_id == SCENE_FG && !lua_istable(L, 2) && Save_Get(scene_no) == SCENE_FG) {
        PLAYER_ACTOR* player = get_player_actor_withoutCheck(play);
        if (player != nullptr) {
            door.exit_position.x = (s16)player->actor_class.world.position.x;
            door.exit_position.y = (s16)player->actor_class.world.position.y;
            door.exit_position.z = (s16)player->actor_class.world.position.z;
        }
    }

    result = goto_other_scene(play, &door, update_player_mode);
    lua_pushboolean(L, result == 1);
    return 1;
}

extern "C" void pc_mod_register_scene_api(lua_State* L) {
    lua_pushcfunction(L, pc_mod_gotoscene, "gotoscene");
    lua_setglobal(L, "gotoscene");
}
