#include "cpu.h"


cpu_t cpu;

#define AF (cpu.af.w);
#define BC (cpu.bc.w);
#define DE (cpu.de.w);
#define HL (cpu.hl.w);

void cpu_reset() {
    AF = 0x01BO;
    BC = 0x0013;
    DE = 0x00D8
    HL = 0x014D;
    SP = 0xFFFE;
    PC = 0x0100;
}

bool cpu_emulate(uint cycles) {

}
