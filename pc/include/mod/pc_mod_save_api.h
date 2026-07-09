#ifndef PC_MOD_SAVE_API_H
#define PC_MOD_SAVE_API_H

#include "lua.h"

#ifdef __cplusplus
extern "C" {
#endif

void pc_mod_register_save_api(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif
