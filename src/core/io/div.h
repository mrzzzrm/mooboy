#ifndef IO_DIV_H
#define IO_DIV_H

    #include "defines.h"

    typedef struct div_s {
        u8 ticks;
        u32 last_mc;
    } div_t;

    extern div_t div;

    div_reset();
    div_step(u32 mc);

#endif
