#ifndef PC_MOD_ITEM_INTERNAL_H
#define PC_MOD_ITEM_INTERNAL_H

#include "lua.h"
#include "m_actor_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PC_MOD_ITEM_MT "PcModItem"

typedef struct PcModItemHandle {
    mActor_name_t item_id;
} PcModItemHandle;

void pc_mod_push_item(lua_State* L, mActor_name_t item_id);
PcModItemHandle* pc_mod_check_item(lua_State* L, int idx);
PcModItemHandle* pc_mod_try_check_item(lua_State* L, int idx);
mActor_name_t pc_mod_item_id(PcModItemHandle* handle);
int pc_mod_read_item_from_value(lua_State* L, int arg, mActor_name_t* item);

#ifdef __cplusplus
}
#endif

#endif
