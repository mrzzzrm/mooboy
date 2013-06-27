#ifndef SYS_H
#define SYS_H

#include <time.h>
#include "util/defines.h"

typedef struct {
    time_t ticks;
    int fb_ready;
    char rompath[256];
    unsigned int bits_per_pixel;
    unsigned int quantum_length;
} sys_t;

extern sys_t sys;


void sys_init(int argc, const char** argv);
void sys_close();

void sys_begin();

int sys_invoke();
void sys_fb_ready();

void sys_save_card();
void sys_load_card();

bool sys_running();
bool sys_new_rom();

void sys_lock_audiobuf();
void sys_unlock_audiobuf();

const char *sys_get_rompath();

#endif // SYS_H
