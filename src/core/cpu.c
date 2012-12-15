#include "cpu.h"

#include "cpu/ops.h"
#include "cpu/defines.h"

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
    cpu.ifs = 0x00;

    cpu.cc = 0;
}

static inline void emulate_op() {
    u8 op = FETCHB;
	op_chunk *c = op_chunk_map[op];
	if(c == NULL) {
		op_chunk_map[op] = op_create_chunk(op);
		c = op_chunk_map[op];
	}
	c->sp = 0;
	c->funcs[0](c);
	cpu.cc += c->mc;
}

static inline void exec_int(u8 i) {
    static u16 isr[] = {0x40, 0x48, 0x50, 0x58, 0x60};

    mem_writew(SP, cpu.pc);
    SP -= 2;
    cpu.pc = isr[i];
}

static inline void handle_ints() {
    if(!cpu.ime)
        return;

    for(u8 i = 0; i < 5; i++) {
        if(cpu.ifs & (1 << i)) {
            cpu.ifs &= ~(1 << i);c
            cpu.ime = 0;
            exec_int(i);
            return;
        }
    }
}

static inline void step_timers() {

}

bool cpu_emulate(uint cycles) {
    emulate_op();
    handle_ints();
    step_timers();

    return true;
}
