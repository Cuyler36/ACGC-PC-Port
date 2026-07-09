#ifndef PC_MOD_PRIVATE_INTERNAL_H
#define PC_MOD_PRIVATE_INTERNAL_H

#include "lua.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PC_MOD_PRIVATE_MT "PcModPrivate"

typedef struct PcModPrivateHandle {
    int player_no;
} PcModPrivateHandle;

struct private_s;

void pc_mod_push_private(lua_State* L, int player_no);
PcModPrivateHandle* pc_mod_check_private(lua_State* L, int idx);
struct private_s* pc_mod_private_data(PcModPrivateHandle* handle);

#ifdef __cplusplus
}
#endif

#endif
