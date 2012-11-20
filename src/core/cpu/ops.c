#include "ops.h"

#include "../cpu.h"
#include "../mem.h"
#include "defines.h"
#include "chunks.h"
#include "tables.h"


static u8 static_byte;
static u16 static_word;

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
    OPLB = OPRB;
}

void op_ld_w(op_chunk *c) {
    OPLW = OPRW;
}

void op_ldx(op_chunk *c) {
    OPLB = OPRB;
    HL = (c->op & 0x10) ? HL + 1 : HL - 1;
}

void op_ld_sphl(op_chunk *c)  {
    SP = HL;
}

void op_ldhl_spi(op_chunk *c) {
    HL = SP + (s8)FETCHB;
}

void op_ld_imsp(op_chunk *c)  {
    mem_writew(mem_readw(PC), SP);
    PC += 2;
}

void op_push_sp(op_chunk *c)  {
    mem_writew(SP, OPLW);
    SP -= 2;
}

void op_pop_sp(op_chunk *c)   {
    OPLW = mem_readw(SP);
    SP += 2;
}

void op_add_b(op_chunk *c) {

}

void op_add_w(op_chunk *c) {
    u32 r = (u32)((u32)OPLW + (u32)OPRW);
    // TODO: Set flags
    OPLW = (u16)r;
}

void op_adc(op_chunk *c) {
    u8 s = (u16)(HN == 0xC ? FETCHB : (LN == 0xE ? mem_readb(HL) : OPLB)) + FC;
    u16 r = (u16)A + (u16)s;
    // TODO: Set flags
    A = (u8)r;
}

void op_sub(op_chunk *c) {
    u8 s = (u16)(HN == 0xD ? FETCHB : (LN == 0x6 ? mem_readb(HL) : OPLB));
    // TODO: Add operation
}

void op_sbc(op_chunk *c) {
    u8 s = (u16)(HN == 0xD ? FETCHB : (LN == 0xE ? mem_readb(HL) : OPLB));
    // TODO: Add operation
}

void op_inc_b(op_chunk *c) {
    if(c->op == 0x34)
        mem_writeb(HL, mem_readb(HL) + 1);
    else
        OPLB++;
    // TODO: Set flags
}

void op_dec_b(op_chunk *c) {
    if(c->op == 0x35)
        mem_writeb(HL, mem_readb(HL) - 1);
    else
        OPLB--;
    // TODO: Set flags
}

void op_inc_w(op_chunk *c) {
    OPLW++;
}

void op_dec_w(op_chunk *c) {
    OPLW--;
}

void op_add_spi(op_chunk *c) {
    u32 r = (u32)SP + (s8)FETCHB;
    // TODO: Set flags
}

void op_and(op_chunk *c) {
    A &= HN == 0xE ? FETCHB : (LN == 0x6 ? mem_readb(HL) : OPLB);
    // TODO: Set flags
}

void op_xor(op_chunk *c) {
    A ^= HN == 0xE ? FETCHB : (LN == 0xE ? mem_readb(HL) : OPLB);
    // TODO: Set flags
}

void op_or(op_chunk *c) {
    A |= HN == 0xF ? FETCHB : (LN == 0x6 ? mem_readb(HL) : OPLB);
    // TODO: Set flags
}

void op_cp(op_chunk *c) {
    u8 o = HN == 0xF ? FETCHB : (LN == 0xE ? mem_readb(HL) : OPLB);
    //u16 c = A - o;
    // TODO: Set flags
}

void op_daa(op_chunk *c) {

}

void op_cpl(op_chunk *c) {

}

void op_rl(op_chunk *c) {
    u8 fc = FCB7(OPLB);
    OPLB = (OPLB<<1) | (FC>>4);
    F = FZZ(OPLB) | fc;
}

void op_rr(op_chunk *c) {
    u8 fc = FCB0(OPLB);
    OPLB = (OPLB>>1) | (FC<<3);
    F = FZZ(OPLB) | fc;
}

void op_rlc(op_chunk *c) {
    OPLB = (OPLB<<1) | (OPLB>>7);
    F = FZZ(OPLB) | FCB0(OPLB);
}

void op_rrc(op_chunk *c) {
    OPLB = (OPLB>>1) | (OPLB<<7);
    F = FZZ(OPLB) | FCB7(OPLB);
}

void op_sla(op_chunk *c) {
    u8 fc = FCB7(OPLB);
    OPLB <<= 1;
    F = FZZ(OPLB) | fc;
}

void op_sra(op_chunk *c) {
    u8 fc = FCB0(OPLB);
    u8 msb = OPLB | 0x80;
    OPLB >>= 1;
    OPLB |= msb;
    F = FZZ(OPLB) | fc;
}

void op_srl(op_chunk *c) {
    u8 fc = FCB0(OPLB);
    OPLB >>= 1;
    F = FZZ(OPLB) | fc;
}

void op_cb(op_chunk *c) {
    u8 cbop = FETCHB;
    op_chunk *cbc = op_cb_chunk_map[cbop];
    if(cbc == NULL) {
        //cbc = op_cb_chunk_map[cbop] = op_create_cb_chunk(cbop);
    }

    if(cbop % 0x08 == 0x06) {
        op_chunk xcbc;
        u8 hlp = mem_readb(HL);
        xcbc.opl.b = &hlp;
        cbc->funcs[0](&xcbc);
        mem_writeb(HL, hlp);
    }
    else {
        cbc->funcs[0](cbc);
    }
}

void op_swap(op_chunk *c) {
    OPLB = LN << 4 | HN >> 4;
    F = FZZ(OPLB);
}

void op_bit(op_chunk *c) {
    F = FZZ((1<<OPLD)&OPRB) | FHBIT | FC;
}

void op_set(op_chunk *c) {
    OPRB |= 1<<OPLD;
}

void op_res(op_chunk *c) {
    OPRB &= ~(1<<OPLD);
}

void op_nop(op_chunk *c) {

}

void op_ccf(op_chunk *c) {
    F = FZ | (FC ? 0 : FCBIT);
}

void op_scf(op_chunk *c) {
    F = FZ | FCBIT;
}

void op_halt(op_chunk *c) {

}

void op_stop(op_chunk *c) {

}

void op_di(op_chunk *c) {

}

void op_ei(op_chunk *c) {

}

void op_jp(op_chunk *c) {
    if(LN == 0x3) {
        PC = FETCHW;
    }
    else if(HN == 0xE) {
        PC = HL;
    }
    else {
        // TODO
    }
}

void op_jr(op_chunk *c) {

}

void op_call(op_chunk *c) {

}

void op_rst(op_chunk *c) {

}

void op_ret(op_chunk *c) {

}

void op_reti(op_chunk *c) {

}


