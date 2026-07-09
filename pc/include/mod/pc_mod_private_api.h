#ifndef PC_MOD_PRIVATE_API_H
#define PC_MOD_PRIVATE_API_H

#include "lua.h"

#ifdef __cplusplus
extern "C" {
#endif

void pc_mod_register_private_api(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif
