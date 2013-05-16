#include "ints.h"
#include <assert.h>
#include <stdio.h>
#include "cpu.h"
#include "defines.h"
#include "util/defines.h"

static inline void exec_int(u8 i) {
    cpu.irq &= ~(1 << i);
    cpu.ime = IME_OFF;

   // printf("INT %i\n", i);
    SP -= 2;
    mem_write_word(SP, PC);

    PC = 0x40 + (i<<3);
    cpu.cc += 4;

//    int x; for(x = 0; x < indents; x++) {
//        printf(" ");
//    }
//    printf("INT { %.4X\n", PC);
//    indents+=2;
}

void ints_handle() {
    switch(cpu.ime) {
        case IME_ON: break;
        case IME_UP: cpu.ime = IME_ON; return;
        case IME_DOWN: cpu.ime = IME_OFF; return;
        case IME_OFF: return;
        default:
            assert(0);
    }

    u8 i;
    for(i = 0; i < 5; i++) {
        if(cpu.irq & cpu.ie & (1 << i)) {
            exec_int(i);
            return;
        }
    }
}

int ints_handle_standby() {
    u8 i;
    for(i = 0; i < 5; i++) {
        if(cpu.irq & cpu.ie & (1 << i)) {
           // if(cpu.ime == IME_ON) {
                exec_int(i);
           // }
            return 1;
        }
    }

    return 0;
}

