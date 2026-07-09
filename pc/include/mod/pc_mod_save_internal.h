#ifndef PC_MOD_SAVE_INTERNAL_H
#define PC_MOD_SAVE_INTERNAL_H

#include "lua.h"

#ifdef __cplusplus
extern "C" {
#endif

int pc_mod_save_is_ready(void);
void pc_mod_push_game_string(lua_State* L, const unsigned char* bytes, size_t max_len);
int pc_mod_check_player_index(lua_State* L, int arg, int optional);
int pc_mod_check_animal_index(lua_State* L, int arg);

#ifdef __cplusplus
}
#endif

#endif
