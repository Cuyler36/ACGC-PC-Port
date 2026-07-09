#include "pc_mod_save_api.h"
#include "pc_mod_save_internal.h"
#include "pc_mod_private_api.h"
#include "pc_mod_animal_api.h"
#include "pc_mod_town_api.h"
#include "lualib.h"

extern "C" {
#include "m_common_data.h"
#include "m_land_h.h"
#include "m_font.h"
#include "m_npc.h"
}

int pc_mod_save_is_ready(void) {
    return Save_Get(land_info).exists != 0;
}

void pc_mod_push_game_string(lua_State* L, const unsigned char* bytes, size_t max_len) {
    char buf[256+1];
    int len = mMl_strlen((u8*)bytes, max_len, CHAR_SPACE);

    if (len > sizeof(buf) - 1) {
        len = sizeof(buf) - 1;
    }

    memmove(buf, bytes, len);
    buf[len] = '\0';
    lua_pushstring(L, buf);
}

int pc_mod_check_player_index(lua_State* L, int arg, int optional) {
    if (optional && lua_isnoneornil(L, arg)) {
        return (int)Common_Get(player_no);
    }

    int idx = (int)luaL_checkinteger(L, arg);
    if (idx < 0 || idx >= PLAYER_NUM) {
        luaL_argerror(L, arg, "player index out of range");
    }
    return idx;
}

int pc_mod_check_animal_index(lua_State* L, int arg) {
    int idx = (int)luaL_checkinteger(L, arg);
    if (idx < 0 || idx >= ANIMAL_NUM_MAX) {
        luaL_argerror(L, arg, "animal index out of range");
    }
    return idx;
}

extern "C" void pc_mod_register_save_api(lua_State* L) {
    pc_mod_register_private_api(L);
    pc_mod_register_animal_api(L);
    pc_mod_register_town_api(L);
}
