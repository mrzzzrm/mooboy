#include "div.h"

div_t div;

div_reset() {
    div.ticks = 0;
    div.last_mc = 0;
}

div_step(u32 mc) {
    u16 mcs = mc - div.last_mc;
    if(mcs >= 64) {
        div.ticks++;
        div.last_mc = mcs - 64;
    }
}


