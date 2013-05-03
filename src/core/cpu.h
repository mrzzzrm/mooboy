#ifndef CPU_H
#define CPU_H

#include "mem.h"
#include "util/defines.h"

#define NORMAL_CPU_FREQ 1048576
#define DOUBLE_CPU_FREQ 2097152

typedef union reg_s {
    u8 b[2];
    u16 w;
} reg_t;

typedef struct cpu_s {
    reg_t af, bc, de, hl;
    reg_t sp, pc;

    u8 ime, irq, ie;

    u32 cc, dfcc, nfcc;
    u32 freq;

    u8 freq_switch;

    u8 halted;
} cpu_t;

extern cpu_t cpu;


void cpu_init();
void cpu_reset();
u8 cpu_exec(u8 op);
u8 cpu_step(); // Ḿachine cycles
u8 cpu_idle_cycle();

#endif // CPU_H
