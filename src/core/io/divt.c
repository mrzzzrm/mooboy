#include "divt.h"
#include "cpu.h"

divt_t divt;

#define MCS_PER_TICK (64)

void divt_reset() {
    divt.ticks = 0;
    divt.last_mc = 0;
}

void divt_step() {
    u16 mcs = cpu.cc - divt.last_mc;
    if(mcs >= MCS_PER_TICK) { // Runs faster or slower depending on gameboy cpu speed
        divt.ticks++;
        divt.last_mc = mcs - MCS_PER_TICK;
    }
}


