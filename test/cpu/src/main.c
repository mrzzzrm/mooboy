#include <assert.h>
#include <stdio.h>
#include "core/cpu.h"
#include "core/cpu_defines.h"
#include "util/defines.h"

u8 ops[] = {
    0x06,
    0xCC
};

u8 mem_readb(u16 adr) {
    assert(adr >= 0x100 && adr < 0x100 + sizeof(ops));

    printf("Read Byte from: %.2X\n", adr);
    return ops[adr - 0x100];
}

u16 mem_readw(u16 adr) {

}

void mem_writeb(u16 adr, u8 d) {

}

void mem_writew(u16 adr, u16 d) {

}

void sys_sleep(u16 ticks) {

}

void print_cpu_state() {
    printf("CPU: PC[%.4X] A[%.2X] F[%.2X] B[%.2X] C[%.2X] D[%.2X] E[%.2X] H[%.2X] L[%.2X] SP[%.4X]\n", PC, A, F, B, C, D, E, H, L, SP);
}

int main(int argc, char **argv) {
    int o;

    cpu_init();
    cpu_reset();

    print_cpu_state();

    for(o = 0; o < 1; o++) {
        printf("Cycle Emulation started\n");
        cpu_emulate(1);
        print_cpu_state();
    }

    return 0;
}
