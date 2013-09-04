#ifndef CORE_SOUND_H
#define CORE_SOUND_H

#include "defines.h"

typedef struct {
    u32 freq;
    u32 sample;

    u16 buf_size;
    u16 buf_start;
    u16 buf_end;
    u8 *buf;
    u8 sample_size;

    u8 on;
    u8 so1_volume;
    u8 so2_volume;

    u8 last_timer_step;
    u32 next_sample;
    u32 next_sample_cc;

    int mix_threshold;
    int cc;
    int tick_cc;
    int counter;
} sound_t;

typedef struct {
    u8 length;
    u8 expires;
} counter_t;

typedef struct {
    u8 sweep;
    u8 tick;
    u8 dir;
} env_t;

typedef struct {
    u8 on;
    u8 l, r;
    u16 freq;
    u8 duty;
    u8 volume;
    counter_t counter;
} sqw_t;

typedef struct {
    u8 period;
    u8 dir;
    u8 shift;
    u8 tick;
} sweep_t;

typedef struct {
    u8 on;
    u8 l, r;
    u16 freq;
    u8 shift;
    u8 data[0x10];
    counter_t counter;
} wave_t;

typedef struct {
    u8 on;
    u8 l, r;
    u8 volume;
    u8 shift;
    u8 width;
    u8 divr;
    u16 lsfr;
    counter_t counter;
} noise_t;

extern sound_t sound;

extern sqw_t sqw[2];
extern env_t env[3];
extern sweep_t sweep;
extern wave_t wave;
extern noise_t noise;

void sound_init();
void sound_close();
void sound_reset();

void sound_step(int nfcs);

void sound_lock();
void sound_unlock();
void sound_mix();

void sound_write(u8 sadr, u8 val);
u8 sound_read(u8 sadr);

#endif
