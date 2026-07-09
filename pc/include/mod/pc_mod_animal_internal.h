#ifndef PC_MOD_ANIMAL_INTERNAL_H
#define PC_MOD_ANIMAL_INTERNAL_H

#include "lua.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PC_MOD_ANIMAL_INDEX_INVALID (-1)

#define PC_MOD_ANIMAL_MT "PcModAnimal"
#define PC_MOD_ANIMAL_ITER_MT "PcModAnimalIter"
#define PC_MOD_ANIMAL_MODULE_MT "PcModAnimalModule"

typedef struct PcModAnimalHandle {
    int animal_index;
} PcModAnimalHandle;

typedef struct PcModAnimalIterState {
    int next_index;
} PcModAnimalIterState;

struct animal_s;

void pc_mod_push_animal(lua_State* L, int animal_index);
PcModAnimalHandle* pc_mod_check_animal(lua_State* L, int idx);
struct animal_s* pc_mod_animal_data(PcModAnimalHandle* handle);

#ifdef __cplusplus
}
#endif

#endif
