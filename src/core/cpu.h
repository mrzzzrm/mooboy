#ifndef CORE_CPU_H
#define CORE_CPU_H

#include "mem.h"
#include "defines.h"

#define NORMAL_CPU_FREQ 1048576
#define DOUBLE_CPU_FREQ 2097152

typedef union {
    u8 b[2];
    u16 w;
} reg_t;

typedef struct {
    reg_t af, bc, de, hl;
    reg_t sp, pc;

    u8 op, cb;

    u8 ime, irq, ie;

    int remainder;

    int freq;
    int freq_switch;
    int halted;
} cpu_t;


extern cpu_t cpu;

void cpu_reset();
u8 cpu_exec(u8 op);
u8 cpu_step();

#endif // CORE_CPU_H
