#include "disasm.h"
#include "cpu/ops.h"
#include "cpu/chunks.h"
#include "cpu/defines.h"

static char out[256];
static char op[16];
static char opl[16];
static char opr[16];

#define OP(func, n) if(f == (func)) {sprintf(op, "%s", (n)); return; }



void set_op(op_chunk *c) {
    void (*f)(op_chunk*) = c->funcs[c->sp++];
    OP(op_ld_b, "LD");
    OP(op_ld_w, "LDW");
    OP(op_ldx, "LDX");
    OP(op_ldhl_spi, "LDHL");
    OP(op_ld_imsp, "LDSP ");
    OP(op_push, "PUSH");
    OP(op_pop, "POP");
    OP(op_add_b, "ADD");
    OP(op_add_w, "ADDW");
    OP(op_adc, "ADC");
    OP(op_sub, "SUB");
    OP(op_sbc, "SBC");
    OP(op_inc_b, "INC");
    OP(op_dec_b, "DEC");
    OP(op_inc_w, "INCW");
    OP(op_dec_w, "DECW");
    OP(op_add_spi, "ADDSP");
    OP(op_and, "AND");
    OP(op_xor, "XOR");
    OP(op_or, "OR");
    OP(op_cp, "CP");
    OP(op_cpl, "CPL");
    OP(op_rl, "RL");
    OP(op_rr, "RR");
    OP(op_rlc, "RLC");
    OP(op_rrc, "RRC");
    OP(op_sla, "SLA");
    OP(op_sra, "SRA");
    OP(op_srl, "SRL");
    OP(op_cb, "CB");
    OP(op_swap, "SWAP");
    OP(op_daa, "DAA");
    OP(op_bit, "BIT");
    OP(op_set, "SET");
    OP(op_res, "RES");
    OP(op_nop, "NOP");
    OP(op_ccf, "CCF");
    OP(op_scf, "SCF");
    OP(op_halt, "HALT");
    OP(op_stop, "STOP");
    OP(op_di, "DI");
    OP(op_ei, "EI");
    OP(op_jp, "JP");
    OP(op_jr, "JR");
    OP(op_call, "CALL");
    OP(op_rst, "RST");
    OP(op_ret, "RET");
    OP(op_reti, "RETI");

    set_op(c);
}

const char *disasm(u16 adr) {
    op_chunk *c = op_chunk_map[mem_readb(adr)];
    c->sp = 0;
    set_op(c);

    sprintf(out, "%s", op);

    return out;
}

