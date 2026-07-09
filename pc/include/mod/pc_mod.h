#ifndef PC_MOD_H
#define PC_MOD_H

#ifdef __cplusplus
extern "C" {
#endif

int  pc_mod_init(void);
void pc_mod_shutdown(void);
int  pc_mod_run_source(const char* source, const char* chunkname);
int  pc_mod_run_file(const char* path);
void pc_mod_load_all(void);
void pc_mod_on_frame(void);

#ifdef __cplusplus
}
#endif

#endif // PC_MOD_H
