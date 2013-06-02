#include "timers.h"
#include "cpu.h"
#include "defines.h"
#include "cpu.h"

#define MCS_PER_DIVT 32

timers_t timers;

static const u16 MCS_PER_TIMA[4] = {0x100, 0x04, 0x10, 0x40};

void timers_reset() {
    timers.div = 0x00;
    timers.tima = 0x00;
    timers.tma = 0x00;
    timers.tac = 0x00;
    timers.div_cc = 0;
    timers.tima_cc = 0;
}

void timers_step() {
    if(timers.tac & 0x04) {
        timers.tima_cc += cpu.step_nf_cycles;
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

    timers.div_cc += cpu.step_sf_cycles;
    while(timers.div_cc >= MCS_PER_DIVT) { // Runs faster or slower depending on gameboy cpu speed
        timers.div++;
        timers.div_cc -= MCS_PER_DIVT;
    }
}

