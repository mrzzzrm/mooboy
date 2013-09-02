#ifndef SYS_H
#define SYS_H

#include <time.h>
#include "defines.h"


typedef struct sys_error_s {
    char text[256];
} sys_error_t;

typedef struct {
    time_t ticks;
    long long ticks_diff;
    time_t pause_start;
    int fb_ready;
    int sound_on;
    int sound_freq;
    int invoke_cc;

    int running;
    int state;

    char rompath[256];

    sys_error_t *error;

    unsigned int bits_per_pixel;
    unsigned int quantum_length;
} sys_t;

extern sys_t sys;


void sys_init(int argc, const char** argv);
void sys_reset();
void sys_close();

void sys_pause();
void sys_run();

void sys_errorf(const char *format, ...);

void sys_invoke();
void sys_fb_ready();

void sys_save_card();
void sys_load_card();

void sys_play_audio(int on);
void sys_lock_audiobuf();
void sys_unlock_audiobuf();

void sys_handle_events(void (*input_handle)(int, int));

void sys_serial_connect();
void sys_serial_step();
int sys_serial_incoming();
void sys_serial_out_bit(int bit);
int sys_serial_in_bit();
void sys_serial_transfer_complete();
void sys_serial_update_internal_period();


#endif // SYS_H
