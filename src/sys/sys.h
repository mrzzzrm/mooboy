#ifndef SYS_H
#define SYS_H

#include <time.h>
#include "util/defines.h"

typedef struct {
    time_t ticks;
    long long ticks_diff;
    time_t pause_start;
    int fb_ready;
    int sound_on;
    int in_menu;
    int rom_loaded;
    int running;
    char rompath[256];
    unsigned int bits_per_pixel;
    unsigned int quantum_length;
} sys_t;

extern sys_t sys;


void sys_init(int argc, const char** argv);
void sys_close();

void sys_begin();

void sys_pause();
void sys_run();

void sys_invoke();
void sys_fb_ready();

void sys_save_card();
void sys_load_card();

void sys_lock_audiobuf();
void sys_unlock_audiobuf();

void sys_handle_events(void (*input_handle)(int, int));

void sys_serial_shift(int send, int bit);



#endif // SYS_H
