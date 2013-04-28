#include "cpu.h"
#include <stdio.h>
#include "lcd.h"
#include "emu.h"
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
    AF = emu.hw == CGB_HW ? 0x11B0 : 0x01B0;
    BC = 0x0013;
    DE = 0x00D8;
    HL = 0x014D;
    SP = 0xFFFE;
    PC = 0x0100;

    cpu.ime = IME_ON;
    cpu.irq = 0x00;
    cpu.ie = 0x00;

    cpu.cc = 0;
    cpu.freq = NORMAL_CPU_FREQ;

    cpu.freq_switch = 0x00;
}

inline u8 cpu_exec(u8 op) {
    u32 old_mcs = cpu.cc;
	op_chunk_t *c = op_chunk_map[op];
	c->sp = 0;
	c->funcs[c->sp++](c);
	cpu.cc += c->mcs;

	return cpu.cc - old_mcs;
}

u8 cpu_step() {
    u8 op = FETCH_BYTE;
    return cpu_exec(op);
}

