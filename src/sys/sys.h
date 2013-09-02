#ifndef SYS_H
#define SYS_H

#include <time.h>
#include "util/defines.h"

#define MOO_ROM_LOADED_BIT  0x01
#define MOO_ROM_RUNNING_BIT 0x02
#define MOO_ERROR_BIT       0x04
#define MOO_RUNNING_BIT     0x08

typedef struct sys_error_s {
    char text[256];
} sys_error_t;

typedef struct {
    time_t ticks;
    long long ticks_diff;
    time_t pause_start;
    int fb_ready;
    int sound_on;
    int paused;
    int rom_loaded;
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
