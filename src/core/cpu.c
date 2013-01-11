#include "cpu.h"
#include "mem/io/divt.h"
#include "mem/io/tima.h"
#include "mem/mbc/rtc.h"
#include "mem/mbc.h"
#include "cpu/ops.h"
#include "cpu/defines.h"
#include "debug/debug.h"

cpu_t cpu;

void cpu_init() {
    memset(op_chunk_map, 0, 0xFF);
    memset(op_cb_chunk_map, 0, 0xFF);
}

void cpu_reset() {
    AF = 0x01B0;
    BC = 0x0013;
    DE = 0x00D8;
    HL = 0x014D;
    SP = 0xFFFE;
    PC = 0x0100;

    cpu.ime = 0x00;
    cpu.irq = 0x00;
    cpu.ie = 0x00;

    cpu.cc = 0;
    cpu.mcs_per_second = 1048576;
}


static inline void exec_int(u8 i) {
    static u16 isr[] = {0x40, 0x48, 0x50, 0x58, 0x60};

    mem_writew(SP, PC);
    SP -= 2;
    PC = isr[i];
}

static inline void handle_ints() {
    switch(cpu.ime) {
        case IME_ON: break;
        case IME_UP: cpu.ime = IME_ON; return;
        case IME_DOWN: cpu.ime = IME_OFF; return;
        case IME_OFF: return;
    }

    u8 i;
    for(i = 0; i < 5; i++) {
        if(cpu.irq & cpu.ie & (1 << i)) {
            cpu.irq &= ~(1 << i);
            cpu.ime = 0;
            exec_int(i);
            return;
        }
    }
}

static inline void step_timers() {
    divt_step();
    tima_step();

    if(mbc.type == 3) {
        rtc_step();
    }
}

void cpu_exec(u8 op) {
	op_chunk *c = op_chunk_map[op];
	if(c == NULL) {
		op_chunk_map[op] = op_create_chunk(op);
		c = op_chunk_map[op];
	}
	c->sp = 0;
	c->funcs[c->sp++](c);
	cpu.cc += c->mcs;
}

u8 cpu_step() {
    cpu_exec(FETCHB);
    handle_ints();
    step_timers();

    return 2; // TODO, cause this is crap
}

