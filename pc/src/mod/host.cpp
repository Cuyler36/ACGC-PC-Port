#include "lua.h"
#include "lualib.h"
#include "luacode.h"
#include "Luau/Compiler.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
#include <string>
#include <vector>
#include <algorithm>
#include <dirent.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <strings.h>
#endif

extern "C" {
#include "pc_mod.h"
#include "pc_mod_api.h"
#include "pc_platform.h"
}

extern "C" void pc_mod_register_api(lua_State* L);

static lua_State* g_L;

extern "C" int pc_mod_init(void) {
    g_L = luaL_newstate();
    if (!g_L) return -1;
    luaL_openlibs(g_L);
    pc_mod_register_api(g_L);
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
    pc_mod_on_load();
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

static int pc_mod_is_script_name(const char* name) {
    size_t len = strlen(name);
    if (len > 4 && strcasecmp(name + len - 4, ".lua") == 0) {
        return 1;
    }
    if (len > 5 && strcasecmp(name + len - 5, ".luau") == 0) {
        return 1;
    }
    return 0;
}

static int pc_mod_skip_name(const char* name) {
    if (name[0] == '\0') {
        return 1;
    }
    if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        return 1;
    }
    if (name[0] == '_' || name[0] == '.') {
        return 1;
    }
    return 0;
}

static int pc_mod_try_append_init(const char* mods_dir, const char* dir_name, std::vector<std::string>& scripts) {
    static const char* entry_names[] = { "init.lua", "init.luau", nullptr };
    char path[512];

    for (int i = 0; entry_names[i]; i++) {
        snprintf(path, sizeof(path), "%s/%s/%s", mods_dir, dir_name, entry_names[i]);
        struct stat st;
        if (stat(path, &st) == 0 && S_ISREG(st.st_mode)) {
            scripts.emplace_back(path);
            return 1;
        }
    }

    return 0;
}

static void pc_mod_collect_scripts(const char* mods_dir, std::vector<std::string>& scripts) {
    DIR* dp = opendir(mods_dir);
    if (!dp) {
        return;
    }

    struct dirent* ent;
    while ((ent = readdir(dp)) != nullptr) {
        if (pc_mod_skip_name(ent->d_name)) {
            continue;
        }

        char path[512];
        snprintf(path, sizeof(path), "%s/%s", mods_dir, ent->d_name);

        struct stat st;
        if (stat(path, &st) != 0) {
            continue;
        }

        if (S_ISREG(st.st_mode) && pc_mod_is_script_name(ent->d_name)) {
            scripts.emplace_back(path);
        } else if (S_ISDIR(st.st_mode)) {
            pc_mod_try_append_init(mods_dir, ent->d_name, scripts);
        }
    }

    closedir(dp);
}

extern "C" void pc_mod_load_all(void) {
    static const char* mods_dir = "mods";
    std::vector<std::string> scripts;

    pc_mod_collect_scripts(mods_dir, scripts);
    std::sort(scripts.begin(), scripts.end());

    if (scripts.empty()) {
        if (g_pc_verbose) {
            fprintf(stderr, "[mod] no scripts found in %s/\n", mods_dir);
        }
        return;
    }

    for (const std::string& path : scripts) {
        if (g_pc_verbose) {
            fprintf(stderr, "[mod] loading %s\n", path.c_str());
        }
        if (pc_mod_run_file(path.c_str()) != 0) {
            fprintf(stderr, "[mod] failed to load %s\n", path.c_str());
        }
    }
}
