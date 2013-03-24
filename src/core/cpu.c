#include "cpu.h"
#include "lcd.h"
#include "rtc.h"
#include "mbc.h"
#include "cpu/ops.h"
#include "defines.h"
#include "timers.h"
#include "debug/debug.h"

cpu_t cpu;

void cpu_init() {
    op_create_chunks();
}

void cpu_reset() {
    AF = 0x01B0;
    BC = 0x0013;
    DE = 0x00D8;
    HL = 0x014D;
    SP = 0xFFFE;
    PC = 0x0100;

    cpu.ime = 0xFF;
    cpu.irq = 0x00;
    cpu.ie = 0xFF;

    cpu.cc = 0;
    cpu.freq = 1048576;
}

inline u8 cpu_exec(u8 op) {
    u32 old_mcs = cpu.cc;

	op_chunk_t *c = op_chunk_map[op];
	c->sp = 0;
	c->funcs[c->sp++](c);
	cpu.cc += c->mcs;
    if(c->mcs>6)
    printf("Alert!!!");

	return cpu.cc - old_mcs;
}

u8 cpu_step() {
    return cpu_exec(FETCH_BYTE);
}

