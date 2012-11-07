#include "cpu.h"

#include "cpu_tables.h"

cpu_t cpu;


void cpu_reset() {
    AF = 0x01B0;
    BC = 0x0013;
    DE = 0x00D8;
    HL = 0x014D;
    SP = 0xFFFE;
    PC = 0x0100;
}

bool cpu_emulate(uint cycles) {
    u8 op = FETCH;
    printf("PC = %.4x | %.4x\n", PC, mem_readb(PC));
    cpu_ops[op](op);
    sys_sleep(500);

    return true;
}
