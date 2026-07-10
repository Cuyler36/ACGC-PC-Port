#ifndef PC_MOD_QUEST_INTERNAL_H
#define PC_MOD_QUEST_INTERNAL_H

#include "lua.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PC_MOD_DELIVERY_QUEST_MT "PcModDeliveryQuest"
#define PC_MOD_ERRAND_QUEST_MT "PcModErrandQuest"
#define PC_MOD_CONTEST_QUEST_MT "PcModContestQuest"

typedef struct PcModDeliveryQuestHandle {
    int player_no;
    int slot_index; /* 0-based */
} PcModDeliveryQuestHandle;

typedef struct PcModErrandQuestHandle {
    int player_no;
    int slot_index; /* 0-based */
} PcModErrandQuestHandle;

typedef struct PcModContestQuestHandle {
    int animal_index;
} PcModContestQuestHandle;

void pc_mod_register_quest_typed(lua_State* L);

void pc_mod_push_delivery_quest(lua_State* L, int player_no, int slot_index);
void pc_mod_push_errand_quest(lua_State* L, int player_no, int slot_index);
void pc_mod_push_contest_quest(lua_State* L, int animal_index);
void pc_mod_push_deliveries(lua_State* L, int player_no);
void pc_mod_push_errands(lua_State* L, int player_no);

#ifdef __cplusplus
}
#endif

#endif
