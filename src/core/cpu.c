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
#include "sys/sys.h"

#ifdef DEBUG
#include "debug/record.h"
#endif

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
    cpu.freq_factor = 1;
    cpu.halted = 0;
    cpu.freq_switch = 0x00;

#ifdef DEBUG
    cpu.dbg_mcs = 0;
#endif
}


u8 cpu_step() {
    ints_handle();

#ifdef DEBUG
    record_cpu_cycle();
#endif

    cpu.op = mem_read_byte(PC++);
    return op_exec();
}


