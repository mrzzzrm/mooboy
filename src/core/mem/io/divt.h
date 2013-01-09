#ifndef IO_DIV_H
#define IO_DIV_H

    #include "util/defines.h"

    typedef struct {
        u8 ticks;
        u32 last_mc;
    } divt_t;

    extern divt_t divt;

    void divt_reset();
    void divt_step();

#endif
