#include "tima.h"
#include "cpu.h"
#include "cpu/defines.h"

tima_t tima;

static u16 mcs_per_tick[] = {1024, 16, 64, 256};

void tima_reset() {
    tima.ticks = 0;
    tima.mod = 0;
    tima.c = 0;
    tima.last_mc = 0;
}

void tima_step() {
    if(!(tima.c & 0x04))
        return;

    u16 mcs = cpu.cc - tima.last_mc;
    u16 per_tick = mcs_per_tick[tima.c & 0x03];
    if(mcs >= per_tick) { // Runs faster or slower depending on gameboy cpu speed
        tima.ticks++;
        tima.last_mc = mcs - per_tick;

        cpu.irq |= IF_TIMER;
    }
}
