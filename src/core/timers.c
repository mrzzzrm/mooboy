#include "timers.h"
#include "cpu.h"
#include "defines.h"
#include "cpu.h"

#define MCS_PER_DIVT 32

timers_t timers;

static const u16 MCS_PER_TIMA[2][4] = {{0x100, 0x04, 0x10, 0x40},{0x200, 0x08, 0x20, 0x80}};

void timers_reset() {
    timers.div = 0x00;
    timers.tima = 0x00;
    timers.tma = 0x00;
    timers.tac = 0x00;
    timers.div_cc = 0;
    timers.tima_cc = 0;
}

void timers_step(u8 mcs) {
    if(timers.tac & 0x04) {
        timers.tima_cc += mcs;
        u16 per_tick = MCS_PER_TIMA[cpu.freq == DOUBLE_CPU_FREQ ? 0 : 0][timers.tac & 0x03];
        if(timers.tima_cc >= per_tick) { // Runs faster or slower depending on gameboy cpu speed
            timers.tima++;
            timers.tima_cc -= per_tick;

            if(timers.tima == 0x00) {
                cpu.irq |= IF_TIMER;
                timers.tima = timers.tma;
            }
        }
    }

    timers.div_cc += mcs;
    if(timers.div_cc >= MCS_PER_DIVT) { // Runs faster or slower depending on gameboy cpu speed
        timers.div++;
        timers.div_cc -= MCS_PER_DIVT;
    }
}

