#include "ops.h"

#include "cpu.h"
#include "mem.h"
#include "defines.h"
#include "chunks.h"


static u8 static_byte;
static u16 static_word;

static void push(u16 w) {
    mem_writew(SP, w);
    SP -= 2;
}

static u16 pop() {
    u16 r = mem_readw(SP);
    SP += 2;
    return r;
}

void op_opl_memcall(op_chunk *c) {
    op_chunk xc;

    static_byte = mem_readb(OPLW);
    xc.opl.b = &static_byte;
    xc.op = c->op;
    xc.opr = c->opr;
    c->funcs[c->sp++](&xc);
    mem_writeb(OPLW, static_byte);
}

void op_opl_ib(op_chunk *c) {
    static_byte = mem_readb(PC++);
    c->opl.b = &static_byte;
    c->funcs[c->sp++](c);
}

void op_opl_iw(op_chunk *c) {
    static_word = mem_readw(PC);
    PC += 2;
    c->opl.w = &static_word;
    c->funcs[c->sp++](c);
}

void op_opl_addio(op_chunk *c) {
    op_chunk xc;
    static_word = (u16)OPLB + 0xFF00;
    xc.opl.w = &static_word;
    xc.op = c->op;
    xc.opr = c->opr;
    c->funcs[c->sp++](&xc);
}

void op_opr_memread(op_chunk *c) {
    op_chunk xc;

    static_byte = mem_readb(OPLW);
    xc.opr.b = &static_byte;
    xc.op = c->op;
    xc.opl = c->opl;
    c->funcs[c->sp++](&xc);
}

void op_opr_ib(op_chunk *c) {
    static_byte = mem_readb(PC++);
    c->opr.b = &static_byte;
    c->funcs[c->sp++](c);
}

void op_opr_iw(op_chunk *c) {
    static_word = mem_readw(PC);
    PC += 2;
    c->opr.w = &static_word;
    c->funcs[++c->sp](c);
}

void op_opr_addio(op_chunk *c) {
    op_chunk xc;
    static_word = (u16)OPRB + 0xFF00;
    xc.opr.w = &static_word;
    xc.op = c->op;
    xc.opl = c->opl;
    c->funcs[c->sp++](&xc);
}

void op_null(op_chunk *c) {

}

void op_ld_b(op_chunk *c) {
    printf("  LD n, b\n");
    OPLB = OPRB;
}

void op_ld_w(op_chunk *c) {
    printf("  LD nn, w\n");
    OPLW = OPRW;
}

void op_ldx(op_chunk *c) {
    printf("  LDX n, b\n");
    OPLB = OPRB;
    HL = (c->op & 0x10) ? HL + 1 : HL - 1;
}

void op_ldhl_spi(op_chunk *c) {
    printf("  LDHL SP, i\n");
    HL = SP + (s8)FETCHB;
    // TODO: Flags!
}

void op_ld_imsp(op_chunk *c)  {
    printf("  LD (i), SP\n");
    mem_writew(mem_readw(PC), SP);
    PC += 2;
}

void op_push(op_chunk *c)  {
    printf("  PUSH SP\n");
    mem_writew(SP, OPLW);
    SP -= 2;
}

void op_pop(op_chunk *c)  {
    printf("  POP SP\n");
    OPLW = mem_readw(SP);
    SP += 2;
}

void op_add_b(op_chunk *c) {
    printf("  ADD b\n");
    u16 r = (u16)OPLB + (u16)OPRB;
    F = FZZ((u8)r) | (FHBIT & ((OPLB ^ OPRB ^ r) << 1)) | (FCBIT & (r >> 4));
    OPLB = (u8)r;
}

void op_add_w(op_chunk *c) {
    printf("  ADD w\n");
    u32 r = (u32)OPLW + (u32)OPRW;
    F = FZZ((u16)r) | (FHBIT & ((OPLB ^ OPRB ^ r) >> 7)) | (FCBIT & (r >> 12));
    OPLW = (u16)r;
}

void op_adc(op_chunk *c) {
    printf("  ADC b\n");
    u16 r = (u16)OPLB + (u16)OPRB + (u16)FC;
    F = FZZ((u8)r) | (FHBIT & ((OPLB ^ OPRB ^ r) << 1)) | (FCBIT & (r >> 4));
    OPLB = (u8)r;
}

void op_sub(op_chunk *c) {
    printf("  SUB b\n");
    u16 r = (u16)OPLB - (u16)OPRB;
    F = FZZ((u8)r) | FNBIT | (FHBIT & ((OPLB ^ OPRB ^ r) << 1)) | (FCBIT & (r >> 4));
    OPLB = (u8)r;
}

void op_sbc(op_chunk *c) {
    printf("  SBC b\n");
    u16 r = (u16)OPLB - (u16)OPRB - (u16)FC;
    F = FZZ((u8)r) | FNBIT | (FHBIT & ((OPLB ^ OPRB ^ r) << 1)) | (FCBIT & (r >> 4));
    OPLB = (u8)r;
}

void op_inc_b(op_chunk *c) {
    printf("  INC b\n");
    u8 r = OPLB + 1;
    F = FZZ((u8)r) | (FHBIT & ((OPLB ^ OPRB ^ r) << 1)) | FC;
    OPLB = r;
}

void op_dec_b(op_chunk *c) {
    printf("  DEC b\n");
    u8 r = OPLB - 1;
    F = FZZ((u8)r) | (FHBIT & ((OPLB ^ OPRB ^ r) << 1)) | FC;
    OPLB = r;
}

void op_inc_w(op_chunk *c) {
    printf("  INC w\n");
    OPLW++;
}

void op_dec_w(op_chunk *c) {
    printf("  DEC w\n");
    OPLW--;
}

void op_add_spi(op_chunk *c) {
    printf("  ADD SP, (i)\n");
    u32 r = (u32)SP + (s8)FETCHB;
    // TODO: Set flags
    SP = r;
}

void op_and(op_chunk *c) {
    printf("  AND b\n");
    OPLB &= OPRB;
    F = FZZ(OPLB) | FHBIT;
}

void op_xor(op_chunk *c) {
    printf("  XOR b\n");
    OPLB ^= OPRB;
    F = FZZ(OPLB);
}

void op_or(op_chunk *c) {
    printf("  OR b\n");
    OPLB |= OPRB;
    F = FZZ(OPLB);
}

void op_cp(op_chunk *c) {
    printf("  CP b\n");
    u16 r = (u16)OPLB - (u16)OPRB;
    F = FZZ((u8)r) | FNBIT | (FHBIT & ((OPLB ^ OPRB ^ r) << 1)) | (FCBIT & (r >> 4));
}

void op_daa(op_chunk *c) {
    printf("  DAA\n");

}

void op_cpl(op_chunk *c) {
    printf("  CPL\n");
    OPLB ^= 0xFF;
    F = FNBIT | FHBIT;
}

void op_rl(op_chunk *c) {
    printf("  RL\n");
    u8 fc = FCB7(OPLB);
    OPLB = (OPLB<<1) | (FC>>4);
    F = FZZ(OPLB) | fc;
}

void op_rr(op_chunk *c) {
    printf("  RR\n");
    u8 fc = FCB0(OPLB);
    OPLB = (OPLB>>1) | (FC<<3);
    F = FZZ(OPLB) | fc;
}

void op_rlc(op_chunk *c) {
    printf("  RLC\n");
    OPLB = (OPLB<<1) | (OPLB>>7);
    F = FZZ(OPLB) | FCB0(OPLB);
}

void op_rrc(op_chunk *c) {
    printf("  RRC\n");
    OPLB = (OPLB>>1) | (OPLB<<7);
    F = FZZ(OPLB) | FCB7(OPLB);
}

void op_sla(op_chunk *c) {
    printf("  SLA\n");
    u8 fc = FCB7(OPLB);
    OPLB <<= 1;
    F = FZZ(OPLB) | fc;
}

void op_sra(op_chunk *c) {
    printf("  SRA\n");
    u8 fc = FCB0(OPLB);
    u8 msb = OPLB | 0x80;
    OPLB >>= 1;
    OPLB |= msb;
    F = FZZ(OPLB) | fc;
}

void op_srl(op_chunk *c) {
    printf("  SRL\n");
    u8 fc = FCB0(OPLB);
    OPLB >>= 1;
    F = FZZ(OPLB) | fc;
}

void op_cb(op_chunk *c) {
    printf("  CB\n");
    u8 cbop = FETCHB;
    op_chunk *cbc = op_cb_chunk_map[cbop];
    if(cbc == NULL) {
        cbc = op_cb_chunk_map[cbop] = op_create_cb_chunk(cbop);
    }

    cbc->sp = 0;
    cbc->funcs[0](cbc);
}

void op_swap(op_chunk *c) {
    printf("  SWAP\n");
    OPLB = LN << 4 | HN >> 4;
    F = FZZ(OPLB);
}

void op_bit(op_chunk *c) {
    printf("  BIT b\n");
    F = FZZ((1<<OPLD)&OPRB) | FHBIT | FC;
}

void op_set(op_chunk *c) {
    printf("  SET b\n");
    OPRB |= 1<<OPLD;
}

void op_res(op_chunk *c) {
    printf("  RES\n");
    OPRB &= ~(1<<OPLD);
}

void op_nop(op_chunk *c) {
    printf("  NOP\n");
}

void op_ccf(op_chunk *c) {
    printf("  CCF\n");
    F = FZ | (FC ? 0 : FCBIT);
}

void op_scf(op_chunk *c) {
    printf("  SCF\n");
    F = FZ | FCBIT;
}

void op_halt(op_chunk *c) {
    printf("  HALT\n");
    // TODO!
}

void op_stop(op_chunk *c) {
    printf("  STOP\n");
    // TODO!
}

void op_di(op_chunk *c) {
    printf("  DI\n");
    cpu.ime = 0x00;
}

void op_ei(op_chunk *c) {
    printf("  EI\n");
    cpu.ime = 0xFF;
}

void op_jp(op_chunk *c) {
    printf("  JP\n");
    switch(c->op) {
        case 0xC3: PC = mem_readw(PC); break;
        case 0xE9: PC = HL; break;
        case 0xC2: if(!FZ) PC = mem_readw(PC); break;
        case 0xCA: if(FZ)  PC = mem_readw(PC); break;
        case 0xD2: if(!FC) PC = mem_readw(PC); break;
        case 0xDA: if(FC)  PC = mem_readw(PC); break;
    }
}

void op_jr(op_chunk *c) {
    printf("  JR\n");
    switch(c->op) {
        case 0x18: cpu.pc.w += (s16)FETCHB - 0x80; break;
        case 0x20: if(!FZ) PC = mem_readw(PC); break;
        case 0x28: if(FZ)  PC = mem_readw(PC); break;
        case 0x30: if(!FC) PC = mem_readw(PC); break;
        case 0x38: if(FC)  PC = mem_readw(PC); break;
    }
}

void op_call(op_chunk *c) {
    printf("  CALL\n");
    switch(c->op) {
        case 0xCD: push(PC+2); PC = mem_readw(PC); break;
        case 0xC4: if(!FZ) push(PC+2); PC = mem_readw(PC); break;
        case 0xCC: if(FZ)  push(PC+2); PC = mem_readw(PC); break;
        case 0xD4: if(!FC) push(PC+2); PC = mem_readw(PC); break;
        case 0xDC: if(FC)  push(PC+2); PC = mem_readw(PC); break;
    }
}

void op_rst(op_chunk *c) {
    printf("  RST\n");
    push(PC);
    PC = ((c->op >> 3) & 0x07) * 8;
}

void op_ret(op_chunk *c) {
    printf("  RET\n");
    switch(c->op) {
        case 0xC9: PC = pop(); break;
        case 0xC0: if(!FZ) PC = pop(); break;
        case 0xC8: if(FZ)  PC = pop(); break;
        case 0xD0: if(!FC) PC = pop(); break;
        case 0xD8: if(FC)  PC = pop(); break;
    }
}

void op_reti(op_chunk *c) {
    printf("  RETI\n");
    PC = pop();
    cpu.ime = 0xFF;
}


