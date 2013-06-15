#include <assert.h>
#include <stdio.h>
#include "core/cpu.h"
#include "core/cpu/chunks.h"
#include "core/cpu/defines.h"
#include "core/cpu/ops.h"
#include "util/defines.h"

u8 ops[] = {
    0x01,
    0xCC,
    0x22
};

u8 mem_readb(u16 adr) {
    assert(adr >= 0x100 && adr < 0x100 + sizeof(ops));

    printf("Read Byte from: %.4X\n", adr);
    return ops[adr - 0x100];
}

u16 mem_readw(u16 adr) {
    assert(adr >= 0x100 && adr+1 < 0x100 + sizeof(ops));

    printf("Read Word from: %.4X\n", adr);
    return ((u16)ops[adr - 0x100]<<8) | ops[adr - 0x100 + 1];
}

void mem_writeb(u16 adr, u8 d) {
    printf("Write Byte to %.4X: %.2X\n", adr, d);
}

void mem_writew(u16 adr, u16 d) {
    printf("Write Word to %.4X: %.4X\n", adr, d);

}

void sys_sleep(u16 ticks) {

}

void print_cpu_state() {
    printf("CPU: PC[%.4X] A[%.2X] F[%.2X] B[%.2X] C[%.2X] D[%.2X] E[%.2X] H[%.2X] L[%.2X] SP[%.4X]\n", PC, A, F, B, C, D, E, H, L, SP);
}

int main(int argc, char **argv) {
    /*int o;

    cpu_init();
    cpu_reset();

    print_cpu_state();

    for(o = 0; o < 1; o++) {
        printf("Cycle Emulation started\n");
        cpu_emulate(1);
        printf("Cycle Emulation finished\n");
        print_cpu_state();
    }*/

    u8 l = 0xFF;
    u8 r = 0x01;

    op_chunk *c = malloc(sizeof(op_chunk));
    c->opl.b = &l;
    c->opr.b = &r;

    op_add_b(c);

    print_cpu_state();
    printf("%i\n",l);

    return 0;
}
