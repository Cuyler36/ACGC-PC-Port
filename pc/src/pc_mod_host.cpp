#include "lua.h"
#include "lualib.h"
#include "luacode.h"
#include "Luau/Compiler.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include <string>

extern "C" {
#include "pc_mod.h"
}

static lua_State* g_L;

extern "C" int pc_mod_init(void) {
    g_L = luaL_newstate();
    if (!g_L) return -1;
    luaL_openlibs(g_L);
    luaL_sandbox(g_L);
    return 0;
}

extern "C" void pc_mod_shutdown(void) {
    if (g_L) {
        lua_close(g_L);
        g_L = nullptr;
    }
}

extern "C" int pc_mod_run_source(const char* source, const char* chunkname) {
    if (!g_L || !source || !chunkname) return -1;

    std::string bytecode;
    try {
        bytecode = Luau::compile(std::string(source, strlen(source)));
    } catch (const std::exception& e) {
        fprintf(stderr, "[mod] compile err: %s\n", e.what());
        return -1;
    } catch (...) {
        fprintf(stderr, "[mod] compile err: unknown\n");
        return -1;
    }

    if (bytecode.empty()) {
        fprintf(stderr, "[mod] compile err: empty output\n");
        return -1;
    }

    lua_State* thread = lua_newthread(g_L);
    luaL_sandboxthread(thread);

    int result = luau_load(thread, chunkname, bytecode.data(), bytecode.size(), 0);
    if (result != LUA_OK) {
        fprintf(stderr, "[mod] load err: %s\n", lua_tostring(thread, -1));
        lua_pop(thread, 1);
        lua_pop(g_L, 1);
        return -1;
    }

    if (lua_pcall(thread, 0, 0, 0) != LUA_OK) {
        fprintf(stderr, "[mod] runtime err: %s\n", lua_tostring(thread, -1));
        lua_pop(thread, 1);
        lua_pop(g_L, 1);
        return -1;
    }

    lua_pop(g_L, 1);
    return 0;
}

extern "C" int pc_mod_run_file(const char* path) {
    FILE* f = fopen(path, "rb");
    char* buffer = nullptr;

    if (!f) return -1;

    if (fseek(f, 0, SEEK_END) != 0) {
        fclose(f);
        return -1;
    }

    long file_size = ftell(f);
    if (file_size < 0) {
        fclose(f);
        return -1;
    }

    rewind(f);

    size_t size = (size_t)file_size;
    buffer = (char*)malloc(size + 1);
    if (!buffer) {
        fclose(f);
        return -1;
    }

    size_t read_size = fread(buffer, 1, size, f);
    if (read_size != size) {
        free(buffer);
        fclose(f);
        return -1;
    }

    buffer[size] = '\0';
    fclose(f);

    int result = pc_mod_run_source(buffer, path);
    free(buffer);
    return result;
}

extern "C" void pc_mod_on_frame(void) {}
