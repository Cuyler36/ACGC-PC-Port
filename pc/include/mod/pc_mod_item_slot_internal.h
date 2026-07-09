#ifndef PC_MOD_ITEM_SLOT_INTERNAL_H
#define PC_MOD_ITEM_SLOT_INTERNAL_H

#include "lua.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PC_MOD_ITEM_SLOT_MT "PcModItemSlot"
#define PC_MOD_POCKETS_MT "PcModPockets"

typedef struct PcModItemSlotHandle {
    int player_no;
    int slot_index;
} PcModItemSlotHandle;

void pc_mod_push_item_slot(lua_State* L, int player_no, int slot_index);
void pc_mod_push_pockets(lua_State* L, int player_no);
int pc_mod_check_pocket_slot(lua_State* L, int arg);
void pc_mod_register_item_slot_api(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif
