#include "timers.h"
#include "cpu.h"
#include "defines.h"
#include "cpu.h"
#include "hw.h"

#define MCS_PER_DIVT 32

timers_t timers;

static const u16 MCS_PER_TIMA[4] = {0x100, 0x04, 0x10, 0x40};

static hw_event_t div_event;
static hw_event_t timer_event;


static void div_schedule(int mcs) {
    div_event.cc = MCS_PER_DIVT + mcs;
    hw_schedule(&div_event);
}

static void div_step(int mcs) {
    timers.div++;
    div_schedule(mcs);
}

static void timer_schedule(int mcs) {
    timer_event.mcs = MCS_PER_TIMA[timers.tac & 0x03];
    timer_event.cc = timer_event.mcs + mcs;
    hw_schedule(&timer_event);
}

static void timer_step(int mcs) {
    timers.tima++;
    if(timers.tima == 0x00) {
        cpu.irq |= IF_TIMER;
        timers.tima = timers.tma;
    }
    timer_schedule(mcs);
}

void timers_reset() {
    timers.div = 0x00;
    timers.tima = 0x00;
    timers.tma = 0x00;
    timers.tac = 0x00;
    timers.div_cc = 0;
    timers.tima_cc = 0;

    div_event.callback = div_step;
    div_event.mcs = MCS_PER_DIVT;
    sprintf(div_event.name, "div");
    div_schedule(0);

    timer_event.callback = timer_step;
    sprintf(timer_event.name, "timer");
}

void timers_step(int nfcs, int mcs) {
//    if(timers.tac & 0x04) {
//        timers.tima_cc += nfcs;//cpu.step_nf_cycles;
//        u16 per_tick = MCS_PER_TIMA[timers.tac & 0x03];
//        while(timers.tima_cc >= per_tick) {
//            timers.tima++;
//            timers.tima_cc -= per_tick;
//            if(timers.tima == 0x00) {
//                cpu.irq |= IF_TIMER;
//                timers.tima = timers.tma;
//            }
//        }
//    }

  //  printf("TIMERS: %i %i\n", timers.div, timers.tima);
//    timers.div_cc += mcs;//cpu.step_sf_cycles;
//    while(timers.div_cc >= MCS_PER_DIVT) { // Runs faster or slower depending on gameboy cpu speed
//        timers.div_cc -= MCS_PER_DIVT;
//    }
}


void timers_enable(int on) {
    if((timers.tac & 0x04) && !on) {
        //hw_unschedule(&timer_event);
    }
    else if(!(timers.tac & 0x04) && on) {
        printf("ENABLED\n");
        timer_schedule(0);
    }
}

