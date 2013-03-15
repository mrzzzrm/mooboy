#ifndef TIMERS_H
#define TIMERS_H

#include "util/defines.h"

typedef struct {
    u8 div, tima, tma, tac;
    u32 div_cc, tima_cc;
} timers_t;

extern timers_t timers;

void timers_reset();
void timers_step(u8 mcs);

#endif
