#ifndef PC_MOD_CONSOLE_H
#define PC_MOD_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Attach terminal (AllocConsole on Windows), start stdin reader thread. */
void pc_mod_console_start(void);

/* Stop reader thread and drain pending commands on the main thread. */
void pc_mod_console_stop(void);

/* Execute queued REPL lines (call from main thread only). */
void pc_mod_console_poll(void);

int pc_mod_console_is_enabled(void);

/* Write to the dedicated Lua console window (no-op if not started). */
void pc_mod_console_printf(const char* fmt, ...);

#ifdef __cplusplus
}
#endif

#endif
