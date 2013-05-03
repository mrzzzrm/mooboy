#include "cpu.h"
#include <stdio.h>
#include <assert.h>
#include "lcd.h"
#include "emu.h"
#include "rtc.h"
#include "mbc.h"
#include "cpu/ops.h"
#include "defines.h"
#include "timers.h"

cpu_t cpu;

void cpu_init() {
    op_create_chunks();
}

void cpu_reset() {
    if(emu.hw == CGB_HW) {
        AF = 0x11B0;
    }
    else {
        AF = 0x01B0;
    }

    BC = 0x0013;
    DE = 0x00D8;
    HL = 0x014D;
    SP = 0xFFFE;
    PC = 0x0100;

    cpu.ime = IME_ON;
    cpu.irq = 0x00;
    cpu.ie = 0x00;

    cpu.cc = 0;
    cpu.dfcc = 0;
    cpu.nfcc = 0;
    cpu.freq = NORMAL_CPU_FREQ;

    cpu.halted = 0;

    cpu.freq_switch = 0x00;
}

inline u8 cpu_exec(u8 op) {
    if(cpu.halted) {
        return cpu_idle_cycle();
    }
    else {
        u32 old_cc = cpu.cc, cc_delta;
        op_chunk_t *c = op_chunk_map[op];

        assert(c->funcs[0] != NULL);

        c->sp = 0;
        c->funcs[c->sp++](c);
        cpu.cc += c->mcs;

        cc_delta = cpu.cc - old_cc;

        if(cpu.freq == DOUBLE_CPU_FREQ) {
            cpu.dfcc += cc_delta;
        }
        cpu.nfcc = cpu.cc - (cpu.dfcc >> 1);

        return cc_delta;
	}
}

u8 cpu_step() {
    u8 op = FETCH_BYTE;
    return cpu_exec(op);
}

u8 cpu_idle_cycle() {
    cpu.cc++;
	if(cpu.freq == DOUBLE_CPU_FREQ) {
        cpu.dfcc++;
	}
	cpu.nfcc = cpu.cc - (cpu.dfcc >> 1);

    return 1;
}

