#ifndef PC_MOD_API_H
#define PC_MOD_API_H

#include "pc_mod.h"

#ifdef __cplusplus
extern "C" {
#endif

void pc_mod_dispatch(const char* event);

void pc_mod_on_load(void);
void pc_mod_on_init(void);
void pc_mod_on_begin_frame(void);
void pc_mod_on_end_frame(void);
void pc_mod_on_pre_move(void);
void pc_mod_on_post_move(void);
void pc_mod_on_pre_draw(void);
void pc_mod_on_post_draw(void);

#ifdef __cplusplus
}
#endif

#endif // PC_MOD_API_H
