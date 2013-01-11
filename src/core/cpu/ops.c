#include "ops.h"

#include "cpu.h"
#include "mem.h"
#include "defines.h"
#include "chunks.h"
#include "debug/debug.h"

#define CPU_MCS(mcs) cpu.cc += (mcs)

static u8 static_byte;
static u16 static_word;

static void push(u16 w) {
    //if(dbg.verbose) fprintf(stderr, "  _push(%X) SP => %.4X\n", w, SP);
    SP -= 2;
    mem_writew(SP, w);
}

static u16 pop() {
    //if(dbg.verbose) fprintf(stderr, "  _pop() SP => %.4X\n", SP);
    u16 r = mem_readw(SP);
    SP += 2;
    return r;
}

void op_opl_memcall(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  _opl_memcall() ADR = %.4X\n", OPLW);
    op_chunk xc = *c;
    u16 adr = OPLW;

    static_byte = mem_readb(adr);
    xc.opl.b = &static_byte;
    xc.op = c->op;
    xc.opr = c->opr;
    c->funcs[xc.sp++](&xc);
    //printf("[%X] (= %X) = %X\n", OPLW,  mem_readb(adr), static_byte);
    mem_writeb(adr, static_byte);
}

void op_opl_ib(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  _opl_ib()");
    static_byte = mem_readb(PC++);//if(dbg.verbose) fprintf(stderr, " B = %.2X\n", static_byte);
    c->opl.b = &static_byte;
    c->funcs[c->sp++](c);
}

void op_opl_iw(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  _opl_iw()");
    static_word = mem_readw(PC); //if(dbg.verbose) fprintf(stderr, " W = %.4X\n", static_word);
    PC += 2;
    c->opl.w = &static_word;
    c->funcs[c->sp++](c);
}

void op_opl_addio(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  _opl_addio() ADR = %.4X\n", (u16)OPLB + 0xFF00);
    op_chunk xc = *c;
    static_word = (u16)OPLB + 0xFF00;
    xc.opl.w = &static_word;
    xc.op = c->op;
    xc.opr = c->opr;
    c->funcs[xc.sp++](&xc);
}

void op_opr_memread(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  _opr_memread() ADR = %.4X", OPRW);
    op_chunk xc = *c;

    static_byte = mem_readb(OPRW); //if(dbg.verbose) fprintf(stderr, " B = %.4X\n", static_byte);
    xc.opr.b = &static_byte;
    c->funcs[xc.sp++](&xc);
}

void op_opr_ib(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  _opr_ib()");
    static_byte = mem_readb(PC++);//if(dbg.verbose) fprintf(stderr, " B = %.2X\n", static_byte);
    c->opr.b = &static_byte;
    c->funcs[c->sp++](c);
}

void op_opr_iw(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  _opr_iw()");
    static_word = mem_readw(PC);//if(dbg.verbose) fprintf(stderr, " W = %.4X\n", static_word);
    PC += 2;
    c->opr.w = &static_word;
    c->funcs[c->sp++](c);
}

void op_opr_addio(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  _opr_addio()");
    op_chunk xc = *c;
    static_word = (u16)OPRB + 0xFF00;//if(dbg.verbose) fprintf(stderr, " ADR = %.4X\n", static_word);
    xc.opr.w = &static_word;
    c->funcs[xc.sp++](&xc);
}

void op_null(op_chunk *c) {

}

void op_ld_b(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  LD n, b\n");
    OPLB = OPRB;
}

void op_ld_w(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  LD nn, w\n");
    OPLW = OPRW;
}

void op_ldx(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  LDX n, b\n");
    OPLB = OPRB;

    HL = (c->op & 0x10) ? HL - 1 : HL + 1;
}

void op_ldhl_spi(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  LDHL SP, i\n");
    HL = SP + (s8)FETCHB;
    // TODO: Flags!
}

void op_ld_imsp(op_chunk *c)  {
    mem_writew(mem_readw(PC), SP);
    //if(dbg.verbose) fprintf(stderr, "  LD (i), SP\n");
    PC += 2;
}

void op_push(op_chunk *c)  {
    //if(dbg.verbose) fprintf(stderr, "  PUSH SP\n");
    push(OPLW);
}

void op_pop(op_chunk *c)  {
    //if(dbg.verbose) fprintf(stderr, "  POP SP\n");
    OPLW = pop();
}

void op_add_b(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  ADD b\n");
    u16 r = (u16)OPLB + (u16)OPRB;
    F = FZZ((u8)r) |
        (FHBIT & ((OPLB ^ OPRB ^ r) << 1)) |
        (FCBIT & (r >> 4));
    OPLB = (u8)r;
}

void op_add_w(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  ADD w\n");
    u32 r = (u32)OPLW + (u32)OPRW;
    F = FZZ((u16)r) |
        (FHBIT & ((OPLB ^ OPRB ^ r) >> 7)) |
        (FCBIT & (r >> 12));
    OPLW = (u16)r;
}

void op_adc(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  ADC b\n");
    u16 r = (u16)OPLB + (u16)OPRB + (u16)FC;
    F = FZZ((u8)r) |
        (FHBIT & ((OPLB ^ OPRB ^ r) << 1)) |
        (FCBIT & (r >> 4));
    OPLB = (u8)r;
}

void op_sub(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  SUB b\n");
    u16 r = (u16)OPLB - (u16)OPRB;
    F = FZZ((u8)r) |
        FNBIT |
        (FHBIT & ((OPLB ^ OPRB ^ r) << 1)) |
        (FCBIT & (r >> 4));
    OPLB = (u8)r;
}

void op_sbc(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  SBC b\n");
    u16 r = (u16)OPLB - (u16)OPRB - (u16)FC;
    F = FZZ((u8)r) |
        FNBIT |
        (FHBIT & ((OPLB ^ OPRB ^ r) << 1)) |
        (FCBIT & (r >> 4));
    OPLB = (u8)r;
}

void op_inc_b(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  INC b\n");
    u16 r = (u16)OPLB + 1;
    F = FZZ((u8)r) |
        (FHBIT & ((OPLB ^ r) << 1)) |
        (FCBIT & (r >> 4));
    OPLB = r;
}

void op_dec_b(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  DEC b\n");
    u8 r = OPLB - 1;
    F = FZZ((u8)r) | FNBIT | (FHBIT & ((OPLB ^ r) << 1)) | (FCBIT & (r >> 4));
    OPLB = r;
}

void op_inc_w(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  INC w\n");
    OPLW++;
}

void op_dec_w(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  DEC w\n");
    OPLW--;
}

void op_add_spi(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  ADD SP, (i)\n");
    u32 r = (u32)SP + (s8)FETCHB;
    // TODO: Set flags
    SP = r;
}

void op_and(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  AND b\n");
    OPLB &= OPRB;
    F = FZZ(OPLB) | FHBIT;
}

void op_xor(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  XOR b\n");
    OPLB ^= OPRB;
    F = FZZ(OPLB);
}

void op_or(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  OR b\n");
    OPLB |= OPRB;
    F = FZZ(OPLB);
}

void op_cp(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  CP b\n");
    u16 r = (u16)OPLB - (u16)OPRB;
    F = FZZ((u8)r) | FNBIT | (FHBIT & ((OPLB ^ OPRB ^ r) << 1)) | (FCBIT & (r >> 4));
}

void op_daa(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  DAA\n");
}

void op_cpl(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  CPL\n");
    A ^= 0xFF;
    F = FNBIT | FHBIT;
}

void op_rl(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  RL\n");
    u8 fc = FCB7(OPLB);
    OPLB = (OPLB<<1) | (FC>>4);
    F = FZZ(OPLB) | fc;
}

void op_rr(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  RR\n");
    u8 fc = FCB0(OPLB);
    OPLB = (OPLB>>1) | (FC<<3);
    F = FZZ(OPLB) | fc;
}

void op_rlc(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  RLC\n");
    OPLB = (OPLB<<1) | (OPLB>>7);
    F = FZZ(OPLB) | FCB0(OPLB);
}

void op_rrc(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  RRC\n");
    OPLB = (OPLB>>1) | (OPLB<<7);
    F = FZZ(OPLB) | FCB7(OPLB);
}

void op_sla(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  SLA\n");
    u8 fc = FCB7(OPLB);
    OPLB <<= 1;
    F = FZZ(OPLB) | fc;
}

void op_sra(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  SRA\n");
    u8 fc = FCB0(OPLB);
    u8 msb = OPLB | 0x80;
    OPLB >>= 1;
    OPLB |= msb;
    F = FZZ(OPLB) | fc;
}

void op_srl(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  SRL\n");
    u8 fc = FCB0(OPLB);
    OPLB >>= 1;
    F = FZZ(OPLB) | fc;
}

void op_cb(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  CB\n");
    u8 cbop = FETCHB;
    op_chunk *cbc = op_cb_chunk_map[cbop];
    if(cbc == NULL) {
        cbc = op_cb_chunk_map[cbop] = op_create_cb_chunk(cbop);
    }

    cbc->sp = 0;
    cbc->funcs[cbc->sp++](cbc);
    cpu.cc += cbc->mcs;
}

void op_swap(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  SWAP\n");
    OPLB = LN << 4 | HN >> 4;
    F = FZZ(OPLB);
}

void op_bit(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  BIT b\n");
    F = FZZ((1<<OPLD)&OPRB) | FHBIT | FC;
}

void op_set(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  SET b\n");
    OPRB |= 1<<OPLD;
}

void op_res(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  RES\n");
    OPRB &= ~(1<<OPLD);
}

void op_nop(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  NOP\n");
}

void op_ccf(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  CCF\n");
    F = FZ | (FC ? 0 : FCBIT);
}

void op_scf(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  SCF\n");
    F = FZ | FCBIT;
}

void op_halt(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  HALT\n");
    // TODO!
}

void op_stop(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  STOP\n");
    // TODO!
}

void op_di(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  DI\n");
    if(cpu.ime != IME_OFF)
        cpu.ime = IME_DOWN;

}

void op_ei(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  EI\n");
    if(cpu.ime != IME_ON)
        cpu.ime = IME_UP;
}

void op_jp(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  JP\n");
    switch(c->op) {
        case 0xC3: PC = OPLW; break;
        case 0xE9: PC = HL; CPU_MCS(1); return;
        case 0xC2: if(!FZ) PC = OPLW; break;
        case 0xCA: if(FZ)  PC = OPLW; break;
        case 0xD2: if(!FC) PC = OPLW; break;
        case 0xDA: if(FC)  PC = OPLW; break;
    }
    CPU_MCS(PC == OPLW ? 4 : 3);
}

void op_jr(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  JR %i\n", (int)((s8)OPLB));
    u16 _pc = PC;

    switch(c->op) {
        case 0x18: cpu.pc.w += (s8)OPLB; break;
        case 0x20: if(!FZ) PC += (s8)OPLB; break;
        case 0x28: if(FZ)  PC += (s8)OPLB; break;
        case 0x30: if(!FC) PC += (s8)OPLB; break;
        case 0x38: if(FC)  PC += (s8)OPLB; break;
    }
    CPU_MCS(_pc == PC ? 2 : 3);
}

void op_call(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  CALL\n");
    u16 _pc = PC;

    switch(c->op) {
        case 0xCD: push(PC); PC = OPLW; break;
        case 0xC4: if(!FZ) push(PC); PC = OPLW; break;
        case 0xCC: if(FZ)  push(PC); PC = OPLW; break;
        case 0xD4: if(!FC) push(PC); PC = OPLW; break;
        case 0xDC: if(FC)  push(PC); PC = OPLW; break;
    }
    CPU_MCS(_pc == PC ? 3 : 6);
}

void op_rst(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  RST\n");
    push(PC);
    PC = ((c->op >> 3) & 0x07) * 8;
}

void op_ret(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  RET\n");
    u16 _pc = PC;

    switch(c->op) {
        case 0xC9: PC = pop(); CPU_MCS(4); return;
        case 0xC0: if(!FZ) PC = pop(); break;
        case 0xC8: if(FZ)  PC = pop(); break;
        case 0xD0: if(!FC) PC = pop(); break;
        case 0xD8: if(FC)  PC = pop(); break;
    }
    CPU_MCS(_pc == PC ? 2 : 5);
}

void op_reti(op_chunk *c) {
    //if(dbg.verbose) fprintf(stderr, "  RETI\n");
    PC = pop();
    cpu.ime = 0xFF;
}


