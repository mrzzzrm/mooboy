#include "tima.h"
#include "cpu.h"
#include "cpu/defines.h"

tima_t tima;

static u16 mcs_per_tick[] = {1024, 16, 64, 256};

void tima_reset() {
    tima.ticks = 0;
    tima.mod = 0;
    tima.c = 0;
    tima.cc = 0;
}

void tima_step(u8 mcs) {
    if(!(tima.c & 0x04)) {
        return;
    }

    tima.cc += mcs;
    u16 per_tick = mcs_per_tick[tima.c & 0x03];
    if(tima.cc >= per_tick) { // Runs faster or slower depending on gameboy cpu speed
        tima.ticks++;
        tima.cc -= per_tick;

        if(tima.ticks == 0x00) {
            cpu.irq |= IF_TIMER;
            tima.ticks = tima.mod;
        }
    }
}
