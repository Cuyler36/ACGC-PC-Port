#ifndef PC_MOD_TOWN_INTERNAL_H
#define PC_MOD_TOWN_INTERNAL_H

#include "lua.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PC_MOD_TOWN_MT "PcModTown"

typedef struct PcModTownHandle {
    int present;
} PcModTownHandle;

void pc_mod_push_town(lua_State* L);
PcModTownHandle* pc_mod_check_town(lua_State* L, int idx);

#ifdef __cplusplus
}
#endif

#endif
