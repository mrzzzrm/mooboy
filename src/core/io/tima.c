#include "tima.h"
#include "cpu.h"

static u16 mcs_per_tick[] = {1024, 16, 64, 256};

void tima_reset() {
    tima.ticks = 0;
    tima.mod = 0;
    tima.creg = 0;
    tima.last_mc = 0;
}

void time_step() {
    if(!(tima.creg & 0x04))
        return;

    u16 mcs = cpu.cc - tima.last_mc;
    u16 per_tick = mcs_per_tick[tima.creg & 0x03];
    if(mcs >= per_tick) { // Runs faster or slower depending on gameboy cpu speed
        tima.ticks++;
        tima.last_mc = mcs - per_tick;

        cpu.ifs |= IFLAG_TIMER;
    }
}
