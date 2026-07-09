#ifndef PC_MOD_LUA_INTERNAL_H
#define PC_MOD_LUA_INTERNAL_H

#include "lua.h"
#include "lualib.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*PcModPropertyGet)(lua_State* L);
typedef int (*PcModPropertySet)(lua_State* L);

typedef struct PcModPropertyDef {
    const char* name;
    PcModPropertyGet get;
    PcModPropertySet set;
} PcModPropertyDef;

// Registers __index/__newindex on the metatable (top of stack) using frozen
// Luau lookup tables for O(1) property dispatch. __index upvalues are
// (getters, methods); __newindex upvalues are (setters, getters).
void pc_mod_register_userdata(lua_State* L, const PcModPropertyDef* props, const luaL_Reg* methods);

void pc_mod_register_userdata_properties(lua_State* L, const PcModPropertyDef* props);

// Attaches __iter to a global module table (e.g. Animal) for `for x in Module do`.
void pc_mod_set_module_iter(lua_State* L, const char* module_name, const char* mt_name, lua_CFunction iter);

// Metatable must be on top of the stack.
void pc_mod_lock_metatable(lua_State* L);

#ifdef __cplusplus
}
#endif

#endif
