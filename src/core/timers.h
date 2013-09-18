#ifndef CORE_TIMERS_H
#define CORE_TIMERS_H

#include "defines.h"

typedef struct {
    u8 div, tima, tma, tac;
    u32 div_cc, tima_cc;
} timers_t;

extern timers_t timers;

void timers_reset();
void timers_step(int nfcs, int mcs);
void timers_tac(u8 tac);

#endif
