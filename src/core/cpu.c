#include "cpu.h"
#include <stdio.h>
#include <assert.h>
#include "lcd.h"
#include "emu.h"
#include "ints.h"
#include "rtc.h"
#include "mbc.h"
#include "ops.h"
#include "defines.h"
#include "timers.h"

cpu_t cpu;

int mcs[2][256];

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

    int x = 0;
    for(x = 0; x < 256; x++) {
        mcs[0][x] = -1;
        mcs[1][x] = -1;
    }
}

u8 cpu_step() {
    u8 op;
    u32 old_cc;
    op_chunk_t *chunk;

    ints_handle();
    op = FETCH_BYTE;
    old_cc = cpu.cc;
    chunk = op_chunk_map[op];

    assert(chunk->funcs[0] != NULL);

    chunk->sp = 0;
    chunk->funcs[chunk->sp++](chunk);

    return chunk->mcs;
//
//        if(mcs[0][op] == -1)
//            mcs[0][op] = cpu.step_sf_cycles;
//        else if(mcs[0][op] == cpu.step_sf_cycles)
//            ;
//        else if(mcs[1][op] == -1)
//            mcs[1][op] = cpu.step_sf_cycles;
//        else if(mcs[1][op] == cpu.step_sf_cycles)
//            ;
//        else
//            assert(0);
}


