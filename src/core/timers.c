#include "timers.h"
#include "cpu.h"
#include "defines.h"
#include "cpu.h"
#include "hw.h"
#include <stdio.h>

#define MCS_PER_DIVT 32

timers_t timers;

static const u16 MCS_PER_TIMA[4] = {0x100, 0x04, 0x10, 0x40};

static hw_event_t div_event;
static hw_event_t timer_event;



static void div_step(int mcs) {
    timers.div++;
    hw_schedule(&div_event, MCS_PER_DIVT - mcs);
}

static void timer_step(int mcs) {
    for(;;) {
        timers.tima++;
        if(timers.tima == 0x00) {
            cpu.irq |= IF_TIMER;
            timers.tima = timers.tma;
        }

        if(mcs >= MCS_PER_TIMA[timers.tac & 0x03])
            mcs -= MCS_PER_TIMA[timers.tac & 0x03];
        else
            break;
    }
    hw_schedule(&timer_event, MCS_PER_TIMA[timers.tac & 0x03] - mcs);
}

void timers_reset() {
    timers.div = 0x00;
    timers.tima = 0x00;
    timers.tma = 0x00;
    timers.tac = 0x00;
    timers.div_cc = 0;
    timers.tima_cc = 0;

    div_event.callback = div_step;
    timer_event.callback = timer_step;

    hw_schedule(&div_event, MCS_PER_DIVT);
}

void timers_step(int nfcs, int mcs) {
    if(timers.tac & 0x04) {
        timers.tima_cc += nfcs;//cpu.step_nf_cycles;
        u16 per_tick = MCS_PER_TIMA[timers.tac & 0x03];
        while(timers.tima_cc >= per_tick) {
            timers.tima++;
            timers.tima_cc -= per_tick;
            if(timers.tima == 0x00) {
                cpu.irq |= IF_TIMER;
                timers.tima = timers.tma;
            }
        }
    }

    timers.div_cc += mcs;//cpu.step_sf_cycles;
    while(timers.div_cc >= MCS_PER_DIVT) { // Runs faster or slower depending on gameboy cpu speed
        timers.div++;
        timers.div_cc -= MCS_PER_DIVT;
    }
}


void timers_tac(u8 tac) {
    if((timers.tac & 0x04) && !(tac & 0x04)) {
        hw_unschedule(&timer_event);
    }
    else if(!(timers.tac & 0x04) && (tac & 0x04)) {
        hw_schedule(&timer_event, MCS_PER_TIMA[timers.tac & 0x03]);
    }

    if((timers.tac & 0x03) != (tac & 0x03)) {
        hw_unschedule(&timer_event);
        hw_schedule(&timer_event, MCS_PER_TIMA[tac & 0x03]);
    }

    timers.tac = tac;
}

