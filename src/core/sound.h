#ifndef SOUND_H
#define SOUND_H

    #include "util/defines.h"

    typedef struct {
        u32 freq;
        u32 sample;

        u16 buf_size;
        u16 buf_start;
        u16 buf_end;
        u8 *buf;
        u8 sample_size;

        u8 enabled;
        u8 so1_volume;
        u8 so2_volume;

        u8 last_timer_step;
        u32 next_sample;
        u32 next_sample_cc;
    } sound_t;

    typedef struct {
        u8 on;
        u16 freq;
        u8 duty;
        u8 volume;
        u8 length;
        u8 expires;
    } sqw_t;

    typedef struct {
        u8 sweep;
        u8 tick;
        u8 dir;
    } env_t;

    typedef struct {
        u8 period;
        u8 dir;
        u8 shift;
        u8 tick;
    } sweep_t;

    typedef struct {
        u8 on;
        u16 freq;
        u8 shift;
        u8 length;
        u8 expires;
        u8 data[0x10];
    } wave_t;

    typedef struct {
        u8 on;
        u8 volume;
        u8 length;
        u8 expires;
        u8 shift;
        u8 width;
        u8 divr;
        u16 lsfr;
        u8 amp;
    } noise_t;

    extern sound_t sound;
    extern sqw_t ch1, ch2;
    extern env_t env1, env2, env3;
    extern sweep_t sweep;
    extern wave_t wave;
    extern noise_t noise;

    void sound_init();
    void sound_close();
    void sound_reset();

    void sound_step();

    void sound_lock();
    void sound_unlock();
    void sound_mix();

    void sound_write_nr10(u8 val);
    void sound_write_nr11(u8 val);
    void sound_write_nr12(u8 val);
    void sound_write_nr13(u8 val);
    void sound_write_nr14(u8 val);

    void sound_write_nr21(u8 val);
    void sound_write_nr22(u8 val);
    void sound_write_nr23(u8 val);
    void sound_write_nr24(u8 val);

    void sound_write_nr30(u8 val);
    void sound_write_nr31(u8 val);
    void sound_write_nr32(u8 val);
    void sound_write_nr33(u8 val);
    void sound_write_nr34(u8 val);
    void sound_write_wave(u8 i, u8 val);

    void sound_write_nr41(u8 val);
    void sound_write_nr42(u8 val);
    void sound_write_nr43(u8 val);
    void sound_write_nr44(u8 val);

    void sound_write_nr50(u8 val);
    void sound_write_nr51(u8 val);
    void sound_write_nr52(u8 val);

#endif
