#include "div.h"
#include "cpu.h"

div_t div;

#define MCS_PER_TICK (64)

div_reset() {
    div.ticks = 0;
    div.last_mc = 0;
}

div_step() {
    u16 mcs = cpu.cc - div.last_mc;
    if(mcs >= MCS_PER_TICK) { // Runs faster or slower depending on gameboy cpu speed
        div.ticks++;
        div.last_mc = mcs - MCS_PER_TICK;
    }
}


