#include "monitor.h"
#include "debug.h"
#include <stdio.h>

void monitor_cell(u16 adr) {
    if(mem_before[adr] != mem_after[adr])
        fprintf(stderr, "[%.4X] %.2X => %.2X\n", adr, mem_before[adr], mem_after[adr]);
}

void monitor_range(u16 from, u16 to) {
    unsigned int c;
    for(c = from; c <= to; c++) {
        monitor_cell(c);
    }
}
