#include "pc_mod_console.h"
#include "pc_mod.h"
#include "pc_platform.h"

#include <atomic>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#else
#include <unistd.h>
#endif

static std::atomic<int> g_console_enabled{0};
static std::atomic<int> g_console_running{0};
static std::mutex g_console_mutex;
static std::vector<std::string> g_console_queue;
static std::thread g_console_thread;
static FILE* g_console_in = nullptr;
static FILE* g_console_out = nullptr;

static void pc_mod_console_write(const char* text) {
    if (!g_console_out || !text) {
        return;
    }

    fputs(text, g_console_out);
    fflush(g_console_out);
}

static void pc_mod_console_write_line(const char* text) {
    pc_mod_console_write(text);
    pc_mod_console_write("\n");
}

static int pc_mod_console_attach_terminal(void) {
#ifdef _WIN32
    if (!AllocConsole()) {
        return 0;
    }

    SetConsoleTitleA("Animal Crossing - Luau Console");

    if (g_console_in) {
        fclose(g_console_in);
        g_console_in = nullptr;
    }
    if (g_console_out) {
        fclose(g_console_out);
        g_console_out = nullptr;
    }

    g_console_in = fopen("CONIN$", "r");
    g_console_out = fopen("CONOUT$", "w");
    if (!g_console_in || !g_console_out) {
        return 0;
    }
#else
    g_console_in = stdin;
    g_console_out = stdout;
#endif

    setvbuf(g_console_out, nullptr, _IONBF, 0);
    return 1;
}

static void pc_mod_console_detach_terminal(void) {
#ifdef _WIN32
    if (g_console_in && g_console_in != stdin) {
        fclose(g_console_in);
    }
    if (g_console_out && g_console_out != stdout) {
        fclose(g_console_out);
    }
    FreeConsole();
#endif

    g_console_in = nullptr;
    g_console_out = nullptr;
}

static void pc_mod_console_print_help(void) {
    pc_mod_console_write_line("Luau console commands:");
    pc_mod_console_write_line("  help        Show this help");
    pc_mod_console_write_line("  exit, quit  Stop the console reader");
    pc_mod_console_write_line("  = <expr>    Evaluate an expression and print the result");
    pc_mod_console_write_line("  <code>      Run a Luau statement (same sandbox/API as mods)");
}

static void pc_mod_console_enqueue_line(std::string line) {
    while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' ' || line.back() == '\t')) {
        line.pop_back();
    }

    size_t start = 0;
    while (start < line.size() && (line[start] == ' ' || line[start] == '\t')) {
        start++;
    }

    if (start > 0) {
        line.erase(0, start);
    }

    if (line.empty()) {
        return;
    }

    std::lock_guard<std::mutex> lock(g_console_mutex);
    g_console_queue.emplace_back(std::move(line));
}

static void pc_mod_console_reader_main(void) {
    char line_buf[4096];

    pc_mod_console_write_line("[luau] console ready. Type help for commands.");

    while (g_console_running.load()) {
        pc_mod_console_write("> ");

        if (!fgets(line_buf, (int)sizeof(line_buf), g_console_in)) {
            break;
        }

        size_t len = strlen(line_buf);
        while (len > 0 && (line_buf[len - 1] == '\r' || line_buf[len - 1] == '\n')) {
            line_buf[--len] = '\0';
        }

        if (strcmp(line_buf, "exit") == 0 || strcmp(line_buf, "quit") == 0) {
            g_console_running.store(0);
            break;
        }

        if (strcmp(line_buf, "help") == 0 || strcmp(line_buf, "?") == 0) {
            pc_mod_console_print_help();
            continue;
        }

        pc_mod_console_enqueue_line(line_buf);
    }

    g_console_running.store(0);
}

extern "C" int pc_mod_console_is_enabled(void) {
    return g_console_enabled.load();
}

extern "C" void pc_mod_console_printf(const char* fmt, ...) {
    if (!g_console_out || !fmt) {
        return;
    }

    va_list ap;
    va_start(ap, fmt);
    vfprintf(g_console_out, fmt, ap);
    va_end(ap);
    fflush(g_console_out);
}

extern "C" void pc_mod_console_start(void) {
    if (!g_pc_lua_console || g_console_enabled.load()) {
        return;
    }

    if (!pc_mod_console_attach_terminal()) {
        return;
    }

    g_console_enabled.store(1);
    g_console_running.store(1);
    g_console_thread = std::thread(pc_mod_console_reader_main);
}

extern "C" void pc_mod_console_stop(void) {
    if (!g_console_enabled.load()) {
        return;
    }

    g_console_running.store(0);

    if (g_console_in && g_console_thread.joinable()) {
        fclose(g_console_in);
        g_console_in = nullptr;
    }

    if (g_console_thread.joinable()) {
        g_console_thread.join();
    }

    std::vector<std::string> pending;
    {
        std::lock_guard<std::mutex> lock(g_console_mutex);
        pending.swap(g_console_queue);
    }

    for (const std::string& line : pending) {
        pc_mod_eval_line(line.c_str());
    }

    pc_mod_console_detach_terminal();
    g_console_enabled.store(0);
}

extern "C" void pc_mod_console_poll(void) {
    if (!g_console_enabled.load()) {
        return;
    }

    std::vector<std::string> pending;
    {
        std::lock_guard<std::mutex> lock(g_console_mutex);
        pending.swap(g_console_queue);
    }

    for (const std::string& line : pending) {
        pc_mod_eval_line(line.c_str());
    }
}
