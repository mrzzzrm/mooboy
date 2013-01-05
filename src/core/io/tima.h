#ifndef IO_TIMA_H
#define IO_TIMA_H

    #include "util/defines.h"

    typedef struct tima_s {
        u8 ticks;
        u8 mod;
        u8 c;
        u32 last_mc;
    } tima_t;

    extern tima_t tima;

    void tima_reset();
    void tima_step();

#endif
