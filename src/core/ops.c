#include "ops.h"
#include <assert.h>
#include <stdio.h>
#include "cpu.h"
#include "emu.h"
#include "mem.h"
#include "defines.h"
#include "chunks.h"
#include "debug/debug.h"

#define CPU_MCS(mcs) c->xmcs += (mcs);

static u8 static_byte;
static u16 static_word;

int indents = 0;

static void push(u16 w) {
    SP -= 2;
    mem_write_word(SP, w);
}

static u16 pop() {
    u16 r = mem_read_word(SP);
    SP += 2;
    return r;
}

void op_opl_memcall(op_chunk_t *c) {
    debug_trace_opl(&OPLW, 2, 1);

    op_chunk_t xc = *c;
    u16 adr = OPLW;

    emu_step_hw(c->rmcs);
    static_byte = mem_read_byte(adr);
    xc.opl.b = &static_byte;
    xc.op = c->op;
    xc.xmcs = 0;
    xc.opr = c->opr;

    c->funcs[xc.sp++](&xc);
    CPU_MCS(xc.xmcs);

    emu_step_hw(c->wmcs);
    mem_write_byte(adr, static_byte);
}

void op_opl_memwrite(op_chunk_t *c) {
    debug_trace_opl(&OPLW, 2, 1);

    op_chunk_t xc = *c;
    u16 adr = OPLW;

    xc.opl.b = &static_byte;
    xc.op = c->op;
    xc.opr = c->opr;
    xc.xmcs = 0;

    c->funcs[xc.sp++](&xc);
    CPU_MCS(xc.xmcs);

    emu_step_hw(c->wmcs);
    mem_write_byte(adr, static_byte);
}

void op_opl_ib(op_chunk_t *c) {
    static_byte = mem_read_byte(PC++);
    c->opl.b = &static_byte;
    debug_trace_opl_data(static_byte);

    c->funcs[c->sp++](c);
}

void op_opl_iw(op_chunk_t *c) {
    static_word = mem_read_word(PC);
    debug_trace_opl_data(static_word);
    PC += 2;
    c->opl.w = &static_word;

    c->funcs[c->sp++](c);
}

void op_opl_addio(op_chunk_t *c) {
    op_chunk_t xc = *c;
    static_word = (u16)OPLB + 0xFF00;
    debug_trace_opl_data(static_word);
    xc.opl.w = &static_word;
    xc.op = c->op;
    xc.opr = c->opr;
    c->funcs[xc.sp++](&xc);
}

void op_opr_memread(op_chunk_t *c) {
    debug_trace_opr(&OPRW, 2, 1);

    op_chunk_t xc = *c;
    xc.xmcs = 0;

    emu_step_hw(c->rmcs);
    static_byte = mem_read_byte(OPRW);

    xc.opr.b = &static_byte;
    c->funcs[xc.sp++](&xc);
    CPU_MCS(xc.xmcs);
}

void op_opr_memcall(op_chunk_t *c) {
    debug_trace_opl(&OPRW, 2, 1);

    op_chunk_t xc = *c;
    u16 adr = OPRW;

    emu_step_hw(c->rmcs);
    static_byte = mem_read_byte(adr);

    xc.opr.b = &static_byte;
    xc.op = c->op;
    xc.opl = c->opl;
    xc.xmcs = 0;
    c->funcs[xc.sp++](&xc);
    CPU_MCS(xc.xmcs);

    emu_step_hw(c->wmcs);
    mem_write_byte(adr, static_byte);
}

void op_opr_ib(op_chunk_t *c) {
    static_byte = mem_read_byte(PC++);
    debug_trace_opr_data(static_byte);
    c->opr.b = &static_byte;
    c->funcs[c->sp++](c);
}

void op_opr_iw(op_chunk_t *c) {
    static_word = mem_read_word(PC);
    debug_trace_opr_data(static_word);
    PC += 2;
    c->opr.w = &static_word;
    c->funcs[c->sp++](c);
}

void op_opr_addio(op_chunk_t *c) {
    op_chunk_t xc = *c;
    static_word = (u16)OPRB + 0xFF00;
    debug_trace_opr_data(static_word);
    xc.opr.w = &static_word;
    c->funcs[xc.sp++](&xc);
}

void op_null(op_chunk_t *c) {
    debug_trace_op("NOP");
}

void op_ld_b(op_chunk_t *c) {
    debug_trace_op("LD"); debug_trace_opl(&OPLB, 1, 0); debug_trace_opr(&OPRB, 1, 0);
    OPLB = OPRB;
}

void op_ld_w(op_chunk_t *c) {
    debug_trace_op("LD"); debug_trace_opl(&OPLW, 2, 0); debug_trace_opr(&OPRW, 2, 0);
    OPLW = OPRW;
}

void op_ldx(op_chunk_t *c) {
    debug_trace_op((c->op & 0x10) ? "HL--" : "HL++");  debug_trace_opr(&OPRB, 1, 0);
    OPLB = OPRB;
    HL = (c->op & 0x10) ? HL - 1 : HL + 1;
}

void op_ldhl_spi(op_chunk_t *c) {
    debug_trace_op("LDHL SP,#");

    s8 o = (s8)FETCH_BYTE;
    u32 r = (u32)SP + o;

    F = 0;
    if ((r & 0xFF) < (SP & 0xFF))
      F |= FCBIT;
    if ((r & 0xF) < (SP & 0xF))
      F |= FHBIT;

    HL = (u16)r;
}

void op_ld_imsp(op_chunk_t *c) {
    debug_trace_op("LD #, SP");
    mem_write_word(mem_read_word(PC), SP);
    PC += 2;
}

void op_push(op_chunk_t *c) {
    debug_trace_op("PUSH"); debug_trace_opl(&OPLW, 2, 0);
    push(OPLW);
}

void op_pop(op_chunk_t *c) {
    debug_trace_op("POP"); debug_trace_opl(&OPLW, 2, 0);
    OPLW = pop();
}

void op_pop_af(op_chunk_t *c) {
    debug_trace_op("POP"); debug_trace_opl(&OPLW, 2, 0);
    OPLW = pop();
    F &= 0xF0;
}

void op_add_b(op_chunk_t *c) {
    debug_trace_op("ADD"); debug_trace_opl(&OPLB, 1, 0); debug_trace_opr(&OPRB, 1, 0);

    u16 r = (u16)OPLB + (u16)OPRB;
    F = FZZ((u8)r) |
        (FHBIT & ((OPLB ^ OPRB ^ r) << 1)) |
        (FCBIT & (r >> 4));
    OPLB = (u8)r;
}

void op_add_w(op_chunk_t *c) {
    debug_trace_op("ADD"); debug_trace_opl(&OPLW, 2, 0); debug_trace_opr(&OPRW, 2, 0);

    u32 r = (u32)OPLW + (u32)OPRW;
    F = FZ |
        (FHBIT & ((OPLW ^ OPRW ^ r) >> 7)) |
        (FCBIT & (r >> 12));
    OPLW = (u16)r;
}

void op_adc(op_chunk_t *c) {
    debug_trace_op("ADC"); debug_trace_opl(&OPLB, 1, 0); debug_trace_opr(&OPRB, 1, 0);

    u8 fc = FC ? 1 : 0;
    u16 r = (u16)OPLB + (u16)OPRB + (u16)fc;
    F = FZZ((u8)r) |
        (FHBIT & ((OPLB ^ OPRB ^ fc ^ r) << 1)) |
        (FCBIT & (r >> 4));
    OPLB = (u8)r;
}

void op_sub(op_chunk_t *c) {
    debug_trace_op("SUB"); debug_trace_opl(&OPLB, 1, 0); debug_trace_opr(&OPRB, 1, 0);

    u16 r = (u16)OPLB - (u16)OPRB;
    F = FZZ((u8)r) |
        FNBIT |
        (FHBIT & ((OPLB ^ OPRB ^ r) << 1)) |
        (FCBIT & (r >> 4));
    OPLB = (u8)r;
}

void op_sbc(op_chunk_t *c) {
    debug_trace_op("SBC"); debug_trace_opl(&OPLB, 1, 0); debug_trace_opr(&OPRB, 1, 0);

    u8 fc = FC ? 1 : 0;
    u16 r = (u16)OPLB - (u16)OPRB - (u16)fc;
    F = FZZ((u8)r) |
        FNBIT |
        (FHBIT & ((OPLB ^ OPRB ^ fc ^ r) << 1)) |
        (FCBIT & (r >> 4));
    OPLB = (u8)r;
}

void op_inc_b(op_chunk_t *c) {
    debug_trace_op("INC"); debug_trace_opl(&OPLB, 1, 0);

    u16 r = (u16)OPLB + 1;
    F = FZZ((u8)r) |
        (FHBIT & ((OPLB ^ r) << 1)) |
        FC;
    OPLB = r;
}

void op_dec_b(op_chunk_t *c) {
    debug_trace_op("DEC"); debug_trace_opl(&OPLB, 1, 0);

    u8 r = OPLB - 1;
    F = FZZ((u8)r) |
        FNBIT |
        (FHBIT & ((OPLB ^ r) << 1)) |
        FC;
    OPLB = r;
}

void op_inc_w(op_chunk_t *c) {
    debug_trace_op("INC"); debug_trace_opl(&OPLW, 2, 0);

    OPLW++;
}

void op_dec_w(op_chunk_t *c) {
    debug_trace_op("DEC"); debug_trace_opl(&OPLW, 2, 0);

    OPLW--;
}

void op_add_spi(op_chunk_t *c) {
    debug_trace_op("ADD SP,#");

    s8 o = (s8)FETCH_BYTE;
    u32 r = (u32)SP + o;

    F = 0x00;
    if ((r & 0xFF) < (SP & 0xFF)) {
        F |= FCBIT;
    }
    if ((r & 0xF) < (SP & 0xF)) {
      F |= FHBIT;
    }

    SP = r;
}

void op_and(op_chunk_t *c) {
    debug_trace_op("AND"); debug_trace_opl(&OPLB, 1, 0); debug_trace_opr(&OPRB, 1, 0);

    OPLB &= OPRB;
    F = FZZ(OPLB) | FHBIT;
}

void op_xor(op_chunk_t *c) {
    debug_trace_op("XOR"); debug_trace_opl(&OPLB, 1, 0); debug_trace_opr(&OPRB, 1, 0);

    OPLB ^= OPRB;
    F = FZZ(OPLB);
}

void op_or(op_chunk_t *c) {
    debug_trace_op("OR"); debug_trace_opl(&OPLB, 1, 0); debug_trace_opr(&OPRB, 1, 0);

    OPLB |= OPRB;
    F = FZZ(OPLB);
}

void op_cp(op_chunk_t *c) {
    debug_trace_op("CP"); debug_trace_opl(&OPLB, 1, 0); debug_trace_opr(&OPRB, 1, 0);

    u16 r = (u16)OPLB - (u16)OPRB;
    F = FZZ((u8)r) | FNBIT | (FHBIT & ((OPLB ^ OPRB ^ r) << 1)) | (FCBIT & (r >> 4));
}

void op_daa(op_chunk_t *c) {
    debug_trace_op("DAA");

    int a = A;

    if (!FN) {
        if (FH || (a & 0xF) > 9)
            a += 0x06;
        if (FC || a > 0x9F)
            a += 0x60;
    }
    else {
        if (FH)
            a = (a - 6) & 0xFF;
        if (FC)
            a -= 0x60;
    }

    F &= ~(FHBIT | FZBIT);
    if ((a & 0x100) == 0x100)
        F |= FCBIT;
    a &= 0xFF;
    if (a == 0)
        F |= FZBIT;
    A = a;
}

void op_cpl(op_chunk_t *c) {
    debug_trace_op("CPL");

    A ^= 0xFF;
    F = FZ | FNBIT | FHBIT | FC;
}

void op_rl(op_chunk_t *c) {
    debug_trace_op("RL"); debug_trace_opl(&OPLB, 1, 0);

    u8 fc = FCB7(OPLB);
    OPLB = (OPLB<<1) | (FC>>4);
    F = FZZ(OPLB) | fc;
}

void op_rr(op_chunk_t *c) {
    debug_trace_op("RR"); debug_trace_opl(&OPLB, 1, 0);

    u8 fc = FCB0(OPLB);
    OPLB = (OPLB>>1) | (FC<<3);
    F = FZZ(OPLB) | fc;
}

void op_rlc(op_chunk_t *c) {
    debug_trace_op("RLC"); debug_trace_opl(&OPLB, 1, 0);

    OPLB = (OPLB<<1) | (OPLB>>7);
    F = FZZ(OPLB) | FCB0(OPLB);
}

void op_rrc(op_chunk_t *c) {
    debug_trace_op("RRC"); debug_trace_opl(&OPLB, 1, 0);

    OPLB = (OPLB>>1) | (OPLB<<7);
    F = FZZ(OPLB) | FCB7(OPLB);
}

void op_rla(op_chunk_t *c) {
    op_rl(c);
    F &= ~FZBIT;
}

void op_rra(op_chunk_t *c) {
    op_rr(c);
    F &= ~FZBIT;
}

void op_rlca(op_chunk_t *c) {
    op_rlc(c);
    F &= ~FZBIT;
}

void op_rrca(op_chunk_t *c) {
    op_rrc(c);
    F &= ~FZBIT;
}

void op_sla(op_chunk_t *c) {
    debug_trace_op("SLA"); debug_trace_opl(&OPLB, 1, 0);

    u8 fc = FCB7(OPLB);
    OPLB <<= 1;
    F = FZZ(OPLB) | fc;
}

void op_sra(op_chunk_t *c) {
    debug_trace_op("SRA"); debug_trace_opl(&OPLB, 1, 0);

    u8 fc = FCB0(OPLB);
    u8 msb = OPLB & 0x80;
    OPLB >>= 1;
    OPLB |= msb;
    F = FZZ(OPLB) | fc;
}

void op_srl(op_chunk_t *c) {
    debug_trace_op("SRL"); debug_trace_opl(&OPLB, 1, 0);

    u8 fc = FCB0(OPLB);
    OPLB >>= 1;
    F = FZZ(OPLB) | fc;
}

void op_cb(op_chunk_t *c) {
    u8 cbop = FETCH_BYTE;
    op_chunk_t *cbc = op_cb_chunk_map[cbop];

    cbc->sp = 0;
    cbc->funcs[cbc->sp++](cbc);

    cpu.cc += cbc->mcs;

}

void op_swap(op_chunk_t *c) {
    debug_trace_op("SWAP"); debug_trace_opl(&OPLB, 1, 0);
    OPLB = ((OPLB & 0x0F) << 4) | (OPLB >> 4);
    F = FZZ(OPLB);
}

void op_bit(op_chunk_t *c) {
    debug_trace_op("BIT"); debug_trace_opl(&OPLD, 1, 0); debug_trace_opr(&OPRB, 1, 0);

    F = FZZ(OPRB & (1<<OPLD)) | FHBIT | FC;
}

void op_set(op_chunk_t *c) {
    debug_trace_op("SET"); debug_trace_opl(&OPLD, 1, 0); debug_trace_opr(&OPRB, 1, 0);

    OPRB |= 1<<OPLD;
}

void op_res(op_chunk_t *c) {
    debug_trace_op("RES"); debug_trace_opl(&OPLD, 1, 0); debug_trace_opr(&OPRB, 1, 0);

    OPRB &= ~(1<<OPLD);
}

void op_nop(op_chunk_t *c) {
    debug_trace_op("NOP");
}

void op_ccf(op_chunk_t *c) {
    debug_trace_op("CCF");
    F = FZ | (FC ? 0 : FCBIT);
}

void op_scf(op_chunk_t *c) {
    debug_trace_op("SCF");
    F = FZ | FCBIT;
}

void op_halt(op_chunk_t *c) {
    debug_trace_op("HALT");
    if(cpu.ime == IME_OFF) {
        cpu.halted = 1;
    }
    else {
        cpu.halted = 1;
    }
}

void op_stop(op_chunk_t *c) {
    debug_trace_op("STOP");
    printf("ALERT ALERT ALERT ---- STOP CALLED ---- ALERT ALERT ALERT ---- %s\n", cpu.freq_switch ? "Switching CPU freq" : "");

    if(cpu.freq_switch) {
        if(cpu.freq == NORMAL_CPU_FREQ) {
            cpu.freq = DOUBLE_CPU_FREQ;
        }
        else {
            cpu.freq = NORMAL_CPU_FREQ;
        }

        cpu.freq_switch = 0x00;
    }
    else {

    }
}

void op_di(op_chunk_t *c) {
    debug_trace_op("DI"); //debug_int_ime(0);
    if(cpu.ime == IME_ON) {
        cpu.ime = IME_DOWN;
    }
//    printf("DI\n");
}

void op_ei(op_chunk_t *c) {
    debug_trace_op("EI"); //debug_int_ime(1);
    if(cpu.ime == IME_OFF) {
        cpu.ime = IME_UP;
    }
 //   printf("EI\n");
}

void op_jp(op_chunk_t *c) {
    debug_trace_op("JP"); if(c->op == 0xE9) debug_trace_opl(&HL, 2, 0); else debug_trace_opl(&OPLW, 2, 0);

    switch(c->op) {
        case 0xC3: PC = OPLW; CPU_MCS(1); break;
        case 0xE9: PC = HL; CPU_MCS(1); return;
        case 0xC2: if(!FZ) {PC = OPLW; CPU_MCS(1);} break;
        case 0xCA: if(FZ)  {PC = OPLW; CPU_MCS(1);} break;
        case 0xD2: if(!FC) {PC = OPLW; CPU_MCS(1);} break;
        case 0xDA: if(FC)  {PC = OPLW; CPU_MCS(1);} break;
        default: assert(0);
    }
    CPU_MCS(3);
}

void op_jr(op_chunk_t *c) {
    debug_trace_op("JR"); debug_trace_opl_data(PC + (s8)OPLB);

    CPU_MCS(2);
    switch(c->op) {
        case 0x18: cpu.pc.w += (s8)OPLB; CPU_MCS(1); break;
        case 0x20: if(!FZ) {PC += (s8)OPLB; CPU_MCS(1); } break;
        case 0x28: if(FZ)  {PC += (s8)OPLB; CPU_MCS(1); } break;
        case 0x30: if(!FC) {PC += (s8)OPLB; CPU_MCS(1); } break;
        case 0x38: if(FC)  {PC += (s8)OPLB; CPU_MCS(1); } break;
        default: assert(0);
    }
}

void op_call(op_chunk_t *c) {
    debug_trace_op("CALL"); debug_trace_opl(&OPLW, 2, 0);

    CPU_MCS(3);
    switch(c->op) {
        case 0xCD: push(PC); PC = OPLW; CPU_MCS(3); debug_call(OPLW); break;
        case 0xC4: if(!FZ) {push(PC); PC = OPLW; CPU_MCS(3);debug_call(OPLW);} break;
        case 0xCC: if(FZ)  {push(PC); PC = OPLW; CPU_MCS(3);debug_call(OPLW);} break;
        case 0xD4: if(!FC) {push(PC); PC = OPLW; CPU_MCS(3);debug_call(OPLW);} break;
        case 0xDC: if(FC)  {push(PC); PC = OPLW; CPU_MCS(3);debug_call(OPLW);} break;
        default: assert(0);
    }
//
//    if(_pc != PC) {
//        int i; for(i=0; i<indents;i++) printf(" ");
//        printf("CALL %.4X {\n", PC);
//        indents+=2;
//    }
}

void op_rst(op_chunk_t *c) {
    debug_trace_op("RST");
    push(PC);
    PC = ((c->op >> 3) & 0x07) * 8;

//    int i; for(i=0; i<indents;i++) printf(" ");
//    printf("RST %.4X {\n", PC);
//    indents+=2;
}

void op_ret(op_chunk_t *c) {
    debug_trace_op("RET");
    u16 _pc = PC;

    CPU_MCS(2);
    switch(c->op) {
        case 0xC9: PC = pop(); CPU_MCS(2); debug_ret(); break;
        case 0xC0: if(!FZ) {PC = pop(); CPU_MCS(3); debug_ret();} break;
        case 0xC8: if(FZ)  {PC = pop(); CPU_MCS(3); debug_ret();} break;
        case 0xD0: if(!FC) {PC = pop(); CPU_MCS(3); debug_ret();} break;
        case 0xD8: if(FC)  {PC = pop(); CPU_MCS(3); debug_ret();} break;
    }

//    ret:
//    int a = 2;
//    if(_pc != PC) {
//    indents-=2;if(indents < 0) indents = 0;
//    int i; for(i=0; i<indents;i++) printf(" ");
//    printf("} RET\n");
//    }
}

void op_reti(op_chunk_t *c) {
    debug_trace_op("RETI");
    PC = pop();
    cpu.ime = IME_ON;


//    indents-=2;if(indents < 0) indents = 0;
//    int i; for(i=0; i<indents;i++) printf(" ");
//    printf("RETI\n");
}

