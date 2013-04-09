#include "ints.h"
#include <assert.h>
#include <stdio.h>
#include "cpu.h"
#include "defines.h"
#include "util/defines.h"

static inline void exec_int(u8 i) {
    static u16 isr[] = {0x40, 0x48, 0x50, 0x58, 0x60};

    debug_int_exec(1 << i);

    SP -= 2;
    mem_write_word(SP, PC);
    PC = isr[i];
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
            //printf("%.4X: INT %i\n", PC-1, i);
            cpu.irq &= ~(1 << i);
            cpu.ime = IME_OFF;
            exec_int(i);
            return;
        }
    }
}

int ints_handle_standby() {
    u8 i;
    for(i = 0; i < 5; i++) {
        if(cpu.irq & (1 << i)) {
            //printf("%.4X: Halted INT %i\n", PC-1, i);
            cpu.irq &= ~(1 << i);
            cpu.ime = IME_OFF;
            exec_int(i);
            return 1;
        }
    }

    return 0;
}

