#ifndef SOUND_H
#define SOUND_H

    #include "util/defines.h"

    typedef struct {
        u32 freq;
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
        u16 freq;
        u8 duty;
        u8 length_counter;
        u8 length_expiration;
        u8 volume;

        u8 enabled;
        u8 so1_enabled, so2_enabled;
    } ch1_t;

    typedef struct {
        u8 period;
        u8 mode;
    } env_t;

    typedef struct {
        u8 period;
        u8 negate;
        u8 shift;

        u16 shadow;
        u8 enable;
    } sweep_t;


    extern sound_t sound;
    extern ch1_t ch1;
    extern env_t env1;
    extern sweep_t sweep;

    void sound_init();
    void sound_close();
    void sound_reset();

    void sound_step();

    void sound_write_nr10(u8 val);
    void sound_write_nr11(u8 val);
    void sound_write_nr12(u8 val);
    void sound_write_nr13(u8 val);
    void sound_write_nr14(u8 val);

    void sound_write_nr50(u8 val);
    void sound_write_nr51(u8 val);
    void sound_write_nr52(u8 val);

#endif
