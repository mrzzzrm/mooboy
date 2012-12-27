#include <stdio.h>
#include "cpu/defines.h"
#include "cpu/chunks.h"
#include "cpu/ops.h"
#include "cpu.h"
#include "debug.h"

ram_t ram;
rom_t rom;

u8 ops[] = {
    0x06, 0x81, // LD B, 0x03;
    0x05, // DEC B
    0x05, // DEC B
    0x05, // DEC B
    0x05, // DEC B
    0x05, // DEC B
    0x05, // DEC B
    0x05, // DEC B
    0x05, // DEC B
    0x05, // DEC B
    0x05, // DEC B
    0x05, // DEC B
    0x05, // DEC B
    0x05, // DEC B
    0x05, // DEC B
    0x05, // DEC B
    0x05, // DEC B
    0x05, // DEC B
    0x05, // DEC B
    0x05, // DEC B
    0x05, // DEC B
    0x05  // DEC B
};

void mem_reset() {

}

u8 mem_readb(u16 adr) {
    fprintf(stderr, "readb %X\n", (int)adr);
    return ops[adr];
}

u16 mem_readw(u16 adr) {
    fprintf(stderr, "readw %X\n", (int)adr);
    return 0;
}

void mem_writeb(u16 adr, u8 val) {
    fprintf(stderr, "writeb %X=%X\n", (int)adr, (int)val);

}

void mem_writew(u16 adr, u16 val) {
    fprintf(stderr, "writew %X=%X\n", (int)adr, (int)val);

}



int main() {
    cpu_init();
    cpu_reset();

    for(PC = 0; PC < sizeof(ops);) {
        debug_cpu_before();
        cpu_exec(ops[PC++]);
        debug_cpu_after();
        debug_cpu_print_diff();
    }

    return 0;
}
