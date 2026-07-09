#include "pc_mod_lua_internal.h"
#include "lua.h"
#include "lualib.h"

static int pc_mod_call_method(lua_State* L);

static int pc_mod_props_index(lua_State* L) {
    const char* key = luaL_checkstring(L, 2);

    lua_pushvalue(L, lua_upvalueindex(1));
    lua_getfield(L, -1, key);
    if (lua_isfunction(L, -1)) {
        lua_pushvalue(L, 1);
        lua_call(L, 1, 1);
        return 1;
    }
    lua_pop(L, 2);

    lua_pushvalue(L, lua_upvalueindex(2));
    lua_getfield(L, -1, key);
    if (lua_isfunction(L, -1)) {
        lua_pushvalue(L, 1);
        lua_pushvalue(L, -2);
        lua_pushcclosure(L, pc_mod_call_method, key, 2);
        return 1;
    }
    lua_pop(L, 2);

    luaL_error(L, "'%s' is not a valid property or method", key);
    return 0;
}

static int pc_mod_call_method(lua_State* L) {
    int argc = lua_gettop(L);
    int call_args;

    if (argc >= 1) {
        lua_pushvalue(L, lua_upvalueindex(1));
        if (lua_rawequal(L, 1, -1)) {
            call_args = argc;
            lua_pop(L, 1);
        } else {
            lua_pop(L, 1);
            lua_pushvalue(L, lua_upvalueindex(1));
            lua_insert(L, 1);
            call_args = argc + 1;
        }
    } else {
        lua_pushvalue(L, lua_upvalueindex(1));
        lua_insert(L, 1);
        call_args = 1;
    }

    lua_pushvalue(L, lua_upvalueindex(2));
    lua_insert(L, 1);
    lua_call(L, call_args, LUA_MULTRET);
    return lua_gettop(L);
}

static int pc_mod_props_newindex(lua_State* L) {
    const char* key = luaL_checkstring(L, 2);

    lua_pushvalue(L, lua_upvalueindex(1));
    lua_getfield(L, -1, key);
    if (lua_isfunction(L, -1)) {
        lua_pushvalue(L, 1);
        lua_pushvalue(L, 2);
        lua_pushvalue(L, 3);
        lua_call(L, 3, 0);
        return 0;
    }

    lua_pop(L, 1);
    lua_pushvalue(L, lua_upvalueindex(2));
    lua_getfield(L, -1, key);
    if (lua_isfunction(L, -1)) {
        luaL_error(L, "'%s' is read-only", key);
    }

    luaL_error(L, "'%s' is not a valid property", key);
    return 0;
}

static void pc_mod_build_property_table(lua_State* L, const PcModPropertyDef* props, int setter) {
    lua_newtable(L);

    for (int i = 0; props[i].name != nullptr; i++) {
        PcModPropertyGet get = props[i].get;
        PcModPropertySet set = props[i].set;
        lua_CFunction fn = setter ? (lua_CFunction)set : (lua_CFunction)get;

        if (fn == nullptr) {
            continue;
        }

        lua_pushcfunction(L, fn, props[i].name);
        lua_setfield(L, -2, props[i].name);
    }

    lua_setreadonly(L, -1, 1);
}

static void pc_mod_build_method_table(lua_State* L, const luaL_Reg* methods) {
    lua_newtable(L);

    if (methods != nullptr) {
        for (int i = 0; methods[i].name != nullptr; i++) {
            lua_pushcfunction(L, methods[i].func, methods[i].name);
            lua_setfield(L, -2, methods[i].name);
        }
    }

    lua_setreadonly(L, -1, 1);
}

void pc_mod_register_userdata(lua_State* L, const PcModPropertyDef* props, const luaL_Reg* methods) {
    pc_mod_build_property_table(L, props, 0);
    pc_mod_build_property_table(L, props, 1);
    pc_mod_build_method_table(L, methods);

    lua_pushvalue(L, -3);
    lua_pushvalue(L, -2);
    lua_pushcclosure(L, pc_mod_props_index, "pc_mod_props_index", 2);
    lua_setfield(L, -5, "__index");

    lua_pushvalue(L, -2);
    lua_pushvalue(L, -3);
    lua_pushcclosure(L, pc_mod_props_newindex, "pc_mod_props_newindex", 2);
    lua_setfield(L, -5, "__newindex");

    lua_pop(L, 3);
}

void pc_mod_register_userdata_properties(lua_State* L, const PcModPropertyDef* props) {
    pc_mod_register_userdata(L, props, nullptr);
}

void pc_mod_lock_metatable(lua_State* L) {
    lua_pushboolean(L, 1);
    lua_setfield(L, -2, "__metatable");
    lua_setreadonly(L, -1, 1);
}

void pc_mod_set_module_iter(lua_State* L, const char* module_name, const char* mt_name, lua_CFunction iter) {
    luaL_newmetatable(L, mt_name);
    lua_pushcfunction(L, iter, mt_name);
    lua_setfield(L, -2, "__iter");
    pc_mod_lock_metatable(L);
    lua_pop(L, 1);

    lua_getglobal(L, module_name);
    luaL_getmetatable(L, mt_name);
    lua_setmetatable(L, -2);
    lua_pop(L, 1);
}
