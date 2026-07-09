#ifndef PC_MOD_ACTOR_INTERNAL_H
#define PC_MOD_ACTOR_INTERNAL_H

#include "lua.h"

extern "C" {
#include "game.h"
#include "m_play.h"
#include "m_actor.h"
}

#define PC_MOD_ACTOR_MT "PcModActor"
#define PC_MOD_ACTOR_ITER_MT "PcModActorIter"

typedef struct PcModActorHandle {
    ACTOR* actor;
} PcModActorHandle;

GAME_PLAY* pc_mod_try_get_play(void);
int pc_mod_actor_is_live(ACTOR* actor, GAME_PLAY* play);
void pc_mod_push_actor(lua_State* L, ACTOR* actor);
ACTOR* pc_mod_check_actor(lua_State* L, int idx);
int pc_mod_parse_part(lua_State* L, int idx);
const char* pc_mod_part_name(int part);

#endif
