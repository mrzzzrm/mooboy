#ifndef CPU_H
#define CPU_H

#include "mem.h"
#include "util/defines.h"

typedef union reg_s {
    u8 b[2];
    u16 w;
} reg_t;

typedef struct cpu_s {
    reg_t af, bc, de, hl;
    reg_t sp, pc;

    u8 ime, irq, ie;

    u32 cc;
    u32 mcs_per_second;
} cpu_t;

extern cpu_t cpu;

void cpu_init();
void cpu_reset();
u8 cpu_exec(u8 op);
u8 cpu_step(); // Ḿachine cycles

#endif // CPU_H
