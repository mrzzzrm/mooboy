#include "timers.h"
#include "cpu.h"
#include "cpu/defines.h"
#include "cpu.h"

#define MCS_PER_DIVT 64

static const u16 MCS_PER_TIMA[] = {0x100, 0x04, 0x10, 0x40};
static u32 div_cc, tima_cc;

void timers_reset() {
    div_cc = 0;
    tima_cc = 0;
}

void timers_step(u8 mcs) {
    if(cpu.tac & 0x04) {
        tima_cc += mcs;
        u16 per_tick = MCS_PER_TIMA[cpu.tac & 0x03];
        if(tima_cc >= per_tick) { // Runs faster or slower depending on gameboy cpu speed
            cpu.tima++;
            tima_cc -= per_tick;

            if(cpu.tima == 0x00) {
                cpu.irq |= IF_TIMER;
                cpu.tima = cpu.tma;
            }
        }
    }

    div_cc += mcs;
    if(div_cc >= MCS_PER_DIVT) { // Runs faster or slower depending on gameboy cpu speed
        cpu.div++;
        div_cc -= MCS_PER_DIVT;
    }
}

