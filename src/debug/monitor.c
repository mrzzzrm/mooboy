#include "monitor.h"
#include "debug.h"
#include "disasm.h"
#include "cpu.h"
#include "cpu/defines.h"
#include <stdio.h>

void monitor_cell(u16 adr) {
    if(dbg.before.mem[adr] != dbg.after.mem[adr])
        fprintf(stderr, "%.4X %s: [%.4X] %.2X => %.2X\n", dbg.before.cpu.pc.w, disasm(cpu.pc.w), adr, dbg.before.mem[adr], dbg.after.mem[adr]);
}

void monitor_range(u16 from, u16 to) {
    unsigned int c;
    for(c = from; c <= to; c++) {
        monitor_cell(c);
    }
}



void debug_sym_dma(u8 hn) {
    if(dbg.monitor.sym & MONITOR_SYM_DMA) {
        fprintf(stderr, "%.4X %s: DMA=%.2XXX\n", dbg.before.cpu.pc.w, disasm(cpu.pc.w), hn);
    }
}

void debug_sym_irq(u8 f) {

}

void debug_sym_ie(u8 f) {

}

void debug_sym_jmp(u16 adr) {

}

void debug_sym_ram(u16 adr, u8 val) {

}

void debug_sym_io(u8 r, u8 val) {

}
