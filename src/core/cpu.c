#include "cpu.h"
#include "io/divt.h"
#include "io/tima.h"
#include "etc/rtc.h"
#include "mem/mbc.h"
#include "cpu/ops.h"
#include "cpu/defines.h"
#include "debug.h"

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
    if(!cpu.ime)
        return;
    fprintf(stderr, "IME: %i\n", cpu.ime);
    u8 i;
    for(i = 0; i < 5; i++) {
        if(cpu.irq & (1 << i)) {
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
    if(dbg.verbose >= DBG_VLVL_MAX) fprintf(stderr, "Exec: %.2X\n", op);
	op_chunk *c = op_chunk_map[op];
    if(dbg.verbose >= DBG_VLVL_MAX) fprintf(stderr, "1\n");
	if(c == NULL) {
        if(dbg.verbose >= DBG_VLVL_MAX) fprintf(stderr, "2\n");
		op_chunk_map[op] = op_create_chunk(op);
		c = op_chunk_map[op];
	}
    if(dbg.verbose >= DBG_VLVL_MAX) fprintf(stderr, "3\n");
	c->sp = 0;
    if(dbg.verbose >= DBG_VLVL_MAX) fprintf(stderr, "4\n");
	c->funcs[c->sp++](c);
    if(dbg.verbose >= DBG_VLVL_MAX) fprintf(stderr, "5\n");
	cpu.cc += c->mcs;
    if(dbg.verbose >= DBG_VLVL_MAX) fprintf(stderr, "6\n");
}

u8 cpu_step() {
    cpu_exec(FETCHB);
    handle_ints();
    step_timers();

    return 2; // TODO, cause this is crap
}

