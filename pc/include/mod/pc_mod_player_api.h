#ifndef PC_MOD_PLAYER_API_H
#define PC_MOD_PLAYER_API_H

#include "lua.h"

#ifdef __cplusplus
extern "C" {
#endif

void pc_mod_register_player_api(lua_State* L);
void pc_mod_player_apply_pending_animation(void);

#ifdef __cplusplus
}
#endif

#endif
