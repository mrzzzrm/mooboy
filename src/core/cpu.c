#include "cpu.h"
#include "mem/io/lcd.h"
#include "mem/mbc/rtc.h"
#include "mem/mbc.h"
#include "cpu/ops.h"
#include "cpu/defines.h"
#include "cpu/timers.h"
#include "debug/debug.h"

cpu_t cpu;

void cpu_init() {
    op_create_chunks();
}

void cpu_reset() {
    AF = 0x01B0;
    BC = 0x0013;
    DE = 0x00D8;
    HL = 0x014D;
    SP = 0xFFFE;
    PC = 0x0100;

    cpu.ime = 0xFF;
    cpu.irq = 0x00;
    cpu.ie = 0xFF;

    cpu.div = 0x00;
    cpu.tima = 0x00;
    cpu.tma = 0x00;
    cpu.tac = 0x00;

    cpu.cc = 0;
    cpu.mcs_per_second = 1048576;
    cpu.halted = 0;
    cpu.stopped = 0;

    timers_reset();
}


static inline void exec_int(u8 i) {
    static u16 isr[] = {0x40, 0x48, 0x50, 0x58, 0x60};

    debug_int_exec(1 << i);

    SP -= 2;
    mem_writew(SP, PC);
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

static inline void step_timers(u8 mcs) {
    timers_step(mcs);

    if(mbc.type == 3) {
        rtc_step(mcs);
    }
}

u8 cpu_exec(u8 op) {
    u32 old_mcs = cpu.cc;

	op_chunk *c = op_chunk_map[op];
	c->sp = 0;
	c->funcs[c->sp++](c);
	cpu.cc += c->mcs;

	return cpu.cc - old_mcs;
}

u8 cpu_step() {
    u8 mcs;

    mcs = cpu_exec(FETCHB);

    step_timers(mcs);
    handle_ints();

    return mcs;
}

