#ifndef CPU_H
#define CPU_H

    #include "mem.h"
    #include "util/defines.h"

    typedef union reg_s {
        byte b[2];
        word w;
    } reg_t;

    typedef struct cpu_s {
        reg_t af, bc, de, hl;
        reg_t sp, pc;
    } cpu_t;

    extern cpu_t cpu;

    void cpu_init();
    void cpu_reset();
    bool cpu_emulate(uint cycles);

#endif // CPU_H
