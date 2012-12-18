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

        u8 ime;
        u8 ifs;

        u32 cc;
        u32 mcs_per_second;
    } cpu_t;

    extern cpu_t cpu;

    void cpu_init();
    void cpu_reset();
    bool cpu_emulate(uint cycles); // Ḿachine cycles

#endif // CPU_H
