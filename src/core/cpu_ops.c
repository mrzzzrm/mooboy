#include "cpu_ops.h"

#include "cpu.h"
#include "cpu_defines.h"
#include "cpu_tables.h"

op_chunk *op_chunk_map[0xFF];
op_chunk *op_cb_chunk_map[0xFF];

static u8 *regv[] = {&B, &C, &D, &E, &H, &L, NULL, &A};

op_chunk *op_create_chunk(u8 op) {
	op_chunk *c = malloc(sizeof(op_chunk));
	c->op = op;
	c->func = op_func_map[op];

	if(c->func == op_ld_ri) {
        c->opl.b = REG(op>>3);
        printf("Reg %p %p\n", REG(op>>3), &B);
	}
	else if(c->func == op_ld_rr) {
        c->opl.b = LN == 7 ? &A : REG(HN-3 + (LN <= 5 ? 0 : 1));
        c->opr.b = LN <= 5 ? REG(LN+1) : REG(LN-7);
	}

	return c;
}

op_chunk *op_create_cb_chunk(u8 op) {
    op_chunk *c = malloc(sizeof(op_chunk));
	c->op = op;

	switch(HN) {
        case 0x0: c->func = LN < 8 ? op_rlc  : op_rrc; break;
        case 0x1: c->func = LN < 8 ? op_rl   : op_rr;  break;
        case 0x2: c->func = LN < 8 ? op_sla  : op_sra; break;
        case 0x3: c->func = LN < 8 ? op_swap : op_srl; break;
        case 0x4: case 0x5: case 0x6: case 0x7: c->func = op_bit; break;
        case 0x8: case 0x9: case 0xA: case 0xB: c->func = op_res; break;
        case 0xC: case 0xd: case 0xE: case 0xF: c->func = op_set; break;
    }

    u8 col = LN % 0x8;
    if(col != 0x6) {
        if(HN < 4) {
            c->opl.b = col == 7 ? & A : REG(col+1);
        }
        else {
            c->opr.b = col == 7 ? & A : REG(col+1);
        }
    }

    return c;
}

void op_null(op_chunk *c) {

}

void op_ld_rr(op_chunk *c)    {
    OPLB = OPRB;
}

void op_ld_ri(op_chunk *c)    {
    OPLB = FETCHB;
}

void op_ld_rrm(op_chunk *c)   {
    OPLB = mem_readb(OPRW);
}

void op_ld_rmr(op_chunk *c)   {
    mem_writeb(OPLW, OPRB);
}

void op_ld_rmi(op_chunk *c)   {
    mem_writeb(OPLW, FETCHB);
}

void op_ld_aim(op_chunk *c)   {
    A = mem_readb(FETCHW);
}

void op_ld_ima(op_chunk *c)   {
    mem_writeb(FETCHW, A);
}

void op_ld_acio(op_chunk *c)  {
    A = mem_readb(0xFF00 + C);
}

void op_ld_cioa(op_chunk *c)  {
    mem_writeb(0xFF00 + C, A);
}

void op_ld_aiio(op_chunk *c)  {
    A =  mem_readb(0xFF00 + FETCHB);
}

void op_ld_iioa(op_chunk *c)  {
    mem_writeb(0xFF00 + FETCHB, A);
}

void op_ldx_hlma(op_chunk *c) {
    mem_writeb(HL, A); HL += HN == 3 ? -1 : 1;
}

void op_ldx_ahlm(op_chunk *c) {
    A = mem_readb(HL); HL += HN == 3 ? -1 : 1;
}

void op_ld_rri(op_chunk *c)   {
    OPLW = FETCHW;
}

void op_ld_sphl(op_chunk *c)  {
    SP = HL;
}

void op_ldhl_spi(op_chunk *c) {
    HL = SP + (s8)FETCHB;
}

void op_ld_imsp(op_chunk *c)  {
    mem_writew(FETCHW, SP);
}

void op_push_sp(op_chunk *c)  {
    mem_writew(SP, OPLW); SP -= 2;
}

void op_pop_sp(op_chunk *c)   {
    OPLW = mem_readw(SP); SP += 2;
}


void op_add(op_chunk *c) {
    u8 s = (u16)(HN == 0xC ? FETCHB : (LN == 0x6 ? mem_readb(HL) : OPLB));
    u16 r = (u16)A + (u16)s;
    // TODO: Set flags
    A = (u8)r;
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

void op_add_hlrr(op_chunk *c) {
    u32 r = (u32)HL + (u32)OPLW;
    // TODO: Set flags
    HL = (u16)r;
}

void op_add_spi(op_chunk *c) {
    u32 r = (u32)SP + (s8)FETCHB;
    // TODO: Set flags
}

void op_inc_w(op_chunk *c) {
    OPLW++;
}

void op_dec_w(op_chunk *c) {
    OPLW--;
}

void op_cb(op_chunk *c) {
    u8 cbop = FETCHB;
    op_chunk *cbc = op_cb_chunk_map[cbop];
    if(cbc == NULL) {
        cbc = op_cb_chunk_map[cbop] = op_create_cb_chunk(cbop);
    }

    if(cbop % 0x08 == 0x06) {
        op_chunk xcbc;
        u8 hlp = mem_readb(HL);
        xcbc.opl.b = &hlp;
        cbc->func(&xcbc);
        mem_writeb(HL, hlp);
    }
    else {
        cbc->func(cbc);
    }
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

void op_swap(op_chunk *c) {
    OPLB = LN << 4 | HN >> 4;
    F = FZZ(OPLB);
}

void op_daa(op_chunk *c) {

}

void op_cpl(op_chunk *c) {

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


