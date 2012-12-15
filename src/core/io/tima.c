#include "tima.h"

void tima_reset() {
    tima.ticks = 0;
    tima.mod = 0;
    tima.creg = 0;
    tima.last_mc = ;
}

void time_step(u32 mc) {
    if(!(tima.creg & 0x04))
        return;

    switch(tima.creg & 0x03) {
        case 0x00: break;
        case 0x01: break;
        case 0x10: break;
        case 0x11: break;
    }
}
