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
}

bool cpu_emulate(uint cycles) {
    u8 op = FETCHB;
	op_chunk *c = op_chunk_map[op];
	if(c == NULL) {
		op_chunk_map[op] = op_create_chunk(op);
		c = op_chunk_map[op];
	}
	c->sp = 0;
	c->funcs[0](c);

    return true;
}
