#ifndef SOUND_H
#define SOUND_H

    #include "util/defines.h"

    typedef struct {
        u32 freq;
        u16 bufsize;
        u8 samplesize;
        u8 *buffer;

        u8 enabled;
        u8 so1_volume;
        u8 so2_volume;
    } sound_t;

    typedef struct {
        u16 freq;
        u8 duty;
        u8 length_counter;
        u8 length_expiration;
        u8 volume;
        u8 env_mode;
        u8 env_period;

        u8 enabled;
        u8 so1_enabled, so2_enabled;
    } ch1_t;

    typedef struct {
        u8 period;
        u8 negate;
        u8 shift;

        u16 shadow;
        u8 enable;
    } sweep_t;




    extern sound_t sound;
    extern ch1_t ch1;
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
