#include "cpu.h"
#include <stdio.h>
#include <assert.h>
#include "lcd.h"
#include "emu.h"
#include "ints.h"
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

u8 cpu_step() {
    if(cpu.halted) {
        cpu.cc++;
        if(cpu.freq == DOUBLE_CPU_FREQ) {
            cpu.dfcc++;
        }
        cpu.nfcc = cpu.cc - (cpu.dfcc >> 1);
        if(ints_handle_standby()) {
            cpu.halted = 0;
        }

        return 1;
    }
    else {
        u8 op;
        u32 old_cc, cc_delta;
        op_chunk_t *chunk;


        ints_handle();

        op = FETCH_BYTE;
        old_cc = cpu.cc;
        chunk = op_chunk_map[op];

        assert(chunk->funcs[0] != NULL);

        chunk->sp = 0;
        chunk->funcs[chunk->sp++](chunk);
        cpu.cc += chunk->mcs;

        cc_delta = cpu.cc - old_cc;

        if(cpu.freq == DOUBLE_CPU_FREQ) {
            cpu.dfcc += cc_delta;
        }
        cpu.nfcc = cpu.cc - (cpu.dfcc >> 1);

        return cc_delta;
	}
}


