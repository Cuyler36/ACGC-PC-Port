#ifndef PC_MOD_QUEST_API_H
#define PC_MOD_QUEST_API_H

#include "lua.h"
#include "lualib.h"

#ifdef __cplusplus
extern "C" {
#endif

void pc_mod_register_quest_api(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif
