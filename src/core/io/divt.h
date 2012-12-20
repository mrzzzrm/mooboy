#ifndef IO_DIV_H
#define IO_DIV_H

    #include "util/defines.h"

    typedef struct {
        u8 ticks;
        u32 last_mc;
    } divt_t;

    extern divt_t divt;

    divt_reset();
    divt_step();

#endif
