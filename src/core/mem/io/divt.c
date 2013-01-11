#include "divt.h"
#include "cpu.h"

divt_t divt;

#define MCS_PER_TICK (64)

void divt_reset() {
    divt.ticks = 0;
    divt.cc = 0;
}

void divt_step(u8 mcs) {
    divt.cc += mcs;
    if(divt.cc >= MCS_PER_TICK) { // Runs faster or slower depending on gameboy cpu speed
        divt.ticks++;
        divt.cc -= MCS_PER_TICK;
    }
}


