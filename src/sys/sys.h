#ifndef SYS_H
#define SYS_H

#include <time.h>
#include "core/defines.h"

typedef struct {
    time_t ticks;
    long long ticks_diff;

    int fb_ready;

    int sound_on;
    int sound_freq;
    int sound_sample_size;
    u16 sound_buf_size;
    u16 sound_buf_start;
    u16 sound_buf_end;
    u8 *sound_buf;

    int invoke_cc;

    int running;
    int state;

    int show_statusbar;

    int scalingmode;
    int num_scalingmodes;
    char **scalingmode_names;

    int auto_continue;

    int auto_rtc;
    int warned_rtc_sav_conflict;
    int speed_factor;

    unsigned int bits_per_pixel;
    unsigned int bytes_per_pixel;
    unsigned int quantum_length;
} sys_t;

#define SYS_AUTO_CONTINUE_NO 0
#define SYS_AUTO_CONTINUE_ASK 1
#define SYS_AUTO_CONTINUE_YES 2

extern sys_t sys;


void sys_init(int argc, const char** argv);
void sys_reset();
void sys_close();

void sys_pause();
void sys_continue();
void sys_begin();

void sys_delay(int ticks);

void sys_invoke();
void sys_fb_ready();

void sys_play_audio(int on);
void sys_lock_audiobuf();
void sys_unlock_audiobuf();

void sys_handle_events(void (*input_handle)(int, int));

void sys_new_performance_info();

void sys_set_scalingmode(int mode);

u16 sys_map_cgb_color(u16 lcd_color);
u16 sys_map_dmg_color(u16 lcd_color);

//void sys_serial_connect();
//void sys_serial_step();
//int sys_serial_incoming();
//void sys_serial_out_bit(int bit);
//int sys_serial_in_bit();
//void sys_serial_transfer_complete();
//void sys_serial_update_internal_period();


#endif // SYS_H
