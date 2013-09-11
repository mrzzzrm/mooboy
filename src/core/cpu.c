#include "cpu.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "lcd.h"
#include "moo.h"
#include "ints.h"
#include "rtc.h"
#include "mem.h"
#include "mbc.h"
#include "ops.h"
#include "defines.h"
#include "timers.h"

cpu_t cpu;

void cpu_reset() {
    if(moo.hw == CGB_HW) {
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

    cpu.remainder = 0;
    cpu.freq = NORMAL_CPU_FREQ;
    cpu.halted = 0;
    cpu.freq_switch = 0x00;
}

static u8 old_vrambanks[2][0x2000] = {{0}};
static u8 old_d, old_e;

static void debug_screwed_bg() {
//    if(memcmp(old_vrambanks, ram.vrambanks, sizeof(old_vrambanks)) != 0) {
//        int a, b;
//        for(b = 0; b < 2; b++) {
//            for(a = 0; a < sizeof(old_vrambanks)/2; a++) {
//                if(old_vrambanks[b][a] != ram.vrambanks[b][a]) {
//                    printf("%.4X %.2X | %i:%.2X %.2X => %.2X\n", cpu.pc.w, cpu.op, b, a, old_vrambanks[b][a], ram.vrambanks[b][a]);
//                }
//            }
//        }
//    }
//    if(old_d != D) {
//        printf("%.4X %.2X | D %.2X => %.2X\n", cpu.pc.w, cpu.op, old_d, D);
//    }
//    if(old_e!= E) {
//        printf("%.4X %.2X | E %.2X => %.2X\n", cpu.pc.w, cpu.op, old_e, E);
//    }
//
//    memcpy(old_vrambanks, ram.vrambanks, sizeof(old_vrambanks));
//    old_d = D;
//    old_e = E;
}

u8 cpu_step() {
    debug_screwed_bg();

    ints_handle();
    cpu.op = mem_read_byte(PC++);
    return op_exec();
}


