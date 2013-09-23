#ifndef CORE_TIMERS_H
#define CORE_TIMERS_H

#include "defines.h"
#include "hw.h"

typedef struct {
    u8 div;
    u8 tima;
    u8 tma;
    u8 tac;
    u32 div_cc, tima_cc;
} timers_t;

extern timers_t timers;

extern hw_event_t timers_div_event;
extern hw_event_t timers_tima_event;


void timers_reset();
void timers_begin();

void timers_tac(u8 tac);

#endif
