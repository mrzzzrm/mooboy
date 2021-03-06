#include "ints.h"

#include <assert.h>
#include <stdio.h>

#include "cpu.h"
#include "defines.h"
#include "hw.h"

static inline void exec_int(u8 i) {
    hw_step(2);

    cpu.irq &= ~(1 << i);
    cpu.ime = IME_OFF;

    SP -= 2;
    mem_write_word(SP, PC);

    hw_step(2);

    PC = 0x40 + (i<<3);

    hw_step(1);
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

    if((cpu.irq & cpu.ie) != 0x00) {
        int i;
        for(i = 0; i < 5; i++) {
            if(cpu.irq & cpu.ie & (1 << i)) {
                exec_int(i);
                return;
            }
        }
    }
}

int ints_handle_standby() {
    if((cpu.irq & cpu.ie) != 0x00) {
        int i;
        for(i = 0; i < 5; i++) {
            if(cpu.irq & cpu.ie & (1 << i)) {
                if(cpu.ime != IME_OFF) {
                    exec_int(i);
                }
                return 1;
            }
        }
    }

    return 0;
}

