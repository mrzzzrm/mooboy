#ifndef IO_TIMA_H
#define IO_TIMA_H

    typedef struct tima_s {
        u8 ticks;
        u8 mod;
        u8 creg;
        u32 last_mc;
    } tima_t;

    extern tima_t tima;

    void tima_reset();
    void time_step(u32 mc);

#endif
