#include "chunks.h"
#include <string.h>
#include <stdlib.h>
#include "cpu.h"
#include "ops.h"
#include "defines.h"


#define VAR_MCS() {c->mcs = (0);}
#define MCS(_mcs) {c->mcs = (_mcs);}
#define PUSH_FUNC(f) {c->funcs[c->sp++] = (f);}

op_chunk_t *op_chunk_map[0x100];
op_chunk_t *op_cb_chunk_map[0x100];

static u8 *bregv[] = {&B, &C, &D, &E, &H, &L, NULL, &A};
static u16 *wregv_sp[] = {&BC, &DE, &HL, &SP};
static u16 *wregv_af[] = {&BC, &DE, &HL, &AF};


static void chunk_opl_stdb(op_chunk_t *c, u8 b) {
    if(b == 0x06) {
        PUSH_FUNC(op_opl_memcall);
        c->opl.w = &HL;
    }
    else {
        c->opl.b = BREG(b);
    }
}

static void chunk_opr_stdb(op_chunk_t *c, u8 b) {
    if(b == 0x06) {
        PUSH_FUNC(op_opr_memread);
        c->opr.w = &HL;
    }
    else {
        c->opr.b = BREG(b);
    }
}

static void chunk_func_alu(op_chunk_t *c, u8 b) {
    switch(b) {
        case 0x00: PUSH_FUNC(op_add_b); break;
        case 0x01: PUSH_FUNC(op_adc); break;
        case 0x02: PUSH_FUNC(op_sub); break;
        case 0x03: PUSH_FUNC(op_sbc); break;
        case 0x04: PUSH_FUNC(op_and); break;
        case 0x05: PUSH_FUNC(op_xor); break;
        case 0x06: PUSH_FUNC(op_or); break;
        case 0x07: PUSH_FUNC(op_cp); break;
    }
}

static op_chunk_t *op_create_chunk(u8 op) {
	op_chunk_t *c = malloc(sizeof(op_chunk_t));
	memset(c, 0x00, sizeof(*c));
	c->op = op;
	c->sp = 0;
	c->funcs[0] = NULL;
	MCS(1); // Default

    switch(op) {
        case 0xD3: case 0xE3: case 0xE4: case 0xF4:
        case 0xDB: case 0xDD: case 0xEB: case 0xEC:
        case 0xED: case 0xFC: case 0xFD:
        return c;

        case 0x00: PUSH_FUNC(op_nop); break;
        case 0x08: PUSH_FUNC(op_ld_imsp); MCS(5); break;
        case 0x10: PUSH_FUNC(op_stop); break;
        case 0x18: PUSH_FUNC(op_opl_ib); PUSH_FUNC(op_jr); VAR_MCS(); break;
        case 0x27: PUSH_FUNC(op_daa); break;
        case 0x2F: PUSH_FUNC(op_cpl); break;
        case 0x37: PUSH_FUNC(op_scf); break;
        case 0x3F: PUSH_FUNC(op_ccf); break;
        case 0x76: PUSH_FUNC(op_halt); break;
        case 0xC3:
            PUSH_FUNC(op_opl_iw);
            PUSH_FUNC(op_jp);
            VAR_MCS();
        break;
        case 0xC9: PUSH_FUNC(op_ret); VAR_MCS(); break;
        case 0xCB: PUSH_FUNC(op_cb);VAR_MCS(); break;
        case 0xCD:
            PUSH_FUNC(op_opl_iw);
            PUSH_FUNC(op_call);
            VAR_MCS();
        break;
        case 0xD9: PUSH_FUNC(op_reti); MCS(4); break;
        case 0xE8: PUSH_FUNC(op_add_spi); MCS(4); break;
        case 0xE9: PUSH_FUNC(op_jp); VAR_MCS(); break;
        case 0xF3: PUSH_FUNC(op_di); break;
        case 0xFB: PUSH_FUNC(op_ei); break;
        case 0xF8: PUSH_FUNC(op_ldhl_spi); MCS(3); break;
        case 0xF9:
            c->opl.w = &SP;
            c->opr.w = &HL;
            PUSH_FUNC(op_ld_w);
            MCS(2);
        break;

        default:
            switch((op&0xC0)>>6) { // (11)XX XXXX
                case 0x00: // 00XX XXXX
                    switch(op & 0x07) { // 00XX X(111)
                        case 0x00: // 00XX X000
                            PUSH_FUNC(op_opl_ib);
                            PUSH_FUNC(op_jr);
                            VAR_MCS();
                        break;
                        case 0x01: // 00XX X001
                            if(op & 0x08) {
                                c->opl.w = &HL;
                                c->opr.w = WREG_SP((op&0x30) >> 4);
                                PUSH_FUNC(op_add_w);
                                MCS(2);
                            }
                            else {
                                c->opl.w = WREG_SP((op&0x30) >> 4);
                                PUSH_FUNC(op_opr_iw);
                                PUSH_FUNC(op_ld_w);
                                MCS(3);
                            }
                        break;
                        case 0x02: // 00XX X010
                            if(op & 0x20) { // 00(1)X X010
                                switch((op&0x18)>>3) { // 001(1 1)010
                                    case 0x00: case 0x02: // 0010 0010 or 0011 0010
                                        c->opr.b = &A;
                                        c->opl.w = &HL;
                                        PUSH_FUNC(op_opl_memcall);
                                    break;
                                    case 0x01: case 0x03: // 0010 1010 or 0011 1010
                                        c->opr.w = &HL;
                                        c->opl.b = &A;
                                        PUSH_FUNC(op_opr_memread);
                                    break;
                                }
                                PUSH_FUNC(op_ldx);
                                MCS(2);
                            }
                            else { // 000X X010
                                if(op & 0x08) { // 000X 1010
                                    c->opl.b = &A;
                                    c->opr.w = WREG_SP((op&0x10)>>4);
                                    PUSH_FUNC(op_opr_memread);
                                }
                                else { // 000X 0010
                                    c->opl.w = WREG_SP((op&0x10)>>4);
                                    c->opr.b = &A;
                                    PUSH_FUNC(op_opl_memcall);
                                }
                                PUSH_FUNC(op_ld_b);
                                MCS(2);
                            }
                        break;
                        case 0x03: // 00XX X011
                            c->opl.w = WREG_SP((op&0x30)>>4);
                            PUSH_FUNC(op&0x08 ? op_dec_w : op_inc_w);
                            MCS(2);
                        break;
                        case 0x04: // 00XX X100
                            chunk_opl_stdb(c, (op&0x38)>>3);
                            PUSH_FUNC(op_inc_b);
                            if(op == 0x34)
                                MCS(3);
                        case 0x05: // 00XX X101
                            chunk_opl_stdb(c, (op&0x38)>>3);
                            PUSH_FUNC(op_dec_b);
                            if(op == 0x35)
                                MCS(3);
                        break;
                        case 0x06: // 00XX X110
                            chunk_opl_stdb(c, (op&0x38)>>3);
                            PUSH_FUNC(op_opr_ib);
                            PUSH_FUNC(op_ld_b);

                            MCS(op == 0x36 ? 3 : 2);
                        break;
                        case 0x07: // 00XX X111
                            c->opl.b = &A;
                            if(op&0x10) {
                                PUSH_FUNC(op&0x08 ? op_rra : op_rla);
                            }
                            else {
                                PUSH_FUNC(op&0x08 ? op_rrca : op_rlca);
                            }
                        break;
                    }
                break;
                case 0x01: // 01XX XXXX except 0111 0110
                    chunk_opr_stdb(c, op & 0x07);
                    chunk_opl_stdb(c, (op & 0x38)>>3);
                    PUSH_FUNC(op_ld_b);
                    if((op & 0x0F) == 0x06 || (op & 0x0F) == 0x0E || (((op & 0x70) == 0x70) && !(op & 0x08)))
                        MCS(2);
                break;
                case 0x02: // 10XX XXXX
                    c->opl.b = &A;
                    chunk_opr_stdb(c, op & 0x07);
                    chunk_func_alu(c, (op & 0x38) >> 3);
                    if((op & 0x0F) == 0x06 || (op & 0x0F) == 0x0E)
                        MCS(2);
                break;
                case 0x03: // 11XX XXXX
                    switch(op & 0x07) { // 11XX X(111)
                        case 0x01: // 11XX X001
                            c->opl.w = WREG_AF((op&0x30)>>4);
                            PUSH_FUNC(op == 0xF1 ? op_pop_af : op_pop);
                            MCS(3);
                        break;
                        case 0x04: // 11XX X100
                            PUSH_FUNC(op_opl_iw);
                            PUSH_FUNC(op_call);
                            VAR_MCS();
                        break;
                        case 0x05: // 11XX X101
                            c->opl.w = WREG_AF((op&0x30)>>4);
                            PUSH_FUNC(op_push);
                            MCS(4);
                        break;
                        case 0x06: // 11XX X110
                            c->opl.b = &A;
                            PUSH_FUNC(op_opr_ib);
                            chunk_func_alu(c, (op&0x38)>>3);
                            MCS(2);
                        break;
                        case 0x07: // 11XX X111
                            PUSH_FUNC(op_rst);
                            MCS(4);
                        break;

                        default: // 11XX X000 or 11XX X010 or 11XX X011
                            if((op & 0xE0) == 0xC0) { // 110X X0XX
                                if(op & 0x02) { // 110X X01X
                                    PUSH_FUNC(op_opl_iw);
                                    PUSH_FUNC(op_jp);
                                }
                                else { // 110X X00X
                                    PUSH_FUNC(op_ret);
                                }
                                VAR_MCS();
                            }
                            else {  // 111X X000 or 111X X010 or 111X X011
                                    // These are the IO access ops
                                if(op & 0x10) {
                                    c->opl.b = &A;
                                }
                                else {
                                    c->opr.b = &A;
                                }

                                switch(op & 0x0F) {
                                    case 0x00:
                                        if(op & 0x10) {
                                            PUSH_FUNC(op_opr_ib);
                                            PUSH_FUNC(op_opr_addio);
                                        }
                                        else {
                                            PUSH_FUNC(op_opl_ib);
                                            PUSH_FUNC(op_opl_addio);
                                        }
                                        MCS(3);
                                    break;
                                    case 0x02:
                                        if(op & 0x10) {
                                            c->opr.b = &C;
                                            PUSH_FUNC(op_opr_addio);
                                        }
                                        else {
                                            c->opl.b = &C;
                                            PUSH_FUNC(op_opl_addio);
                                        }
                                        MCS(2);
                                    break;
                                    case 0x0A:
                                        if(op & 0x10) {
                                            PUSH_FUNC(op_opr_iw);
                                        }
                                        else {
                                            PUSH_FUNC(op_opl_iw);
                                        }
                                        MCS(4);
                                    break;
                                }

                                if(op & 0x10) {
                                    PUSH_FUNC(op_opr_memread);
                                }
                                else {
                                    PUSH_FUNC(op_opl_memcall);
                                }
                                PUSH_FUNC(op_ld_b);
                            }
                    }
                break;
            }
    }

	return c;
}

static void chunk_cb_opl_stdb(op_chunk_t *c, u8 b) {
    chunk_opl_stdb(c, b);
}

static void chunk_cb_opr_stdb(op_chunk_t *c, u8 b) {
    if(b == 0x06) {
        PUSH_FUNC(op_opr_memcall);
        c->opr.w = &HL;
    }
    else {
        c->opr.b = BREG(b);
    }
}

static op_chunk_t *op_create_cb_chunk(u8 op) {
    op_chunk_t *c = malloc(sizeof(op_chunk_t));
	memset(c, 0x00, sizeof(*c));
	c->op = op;
	c->sp = 0;

    if(HN <= 3) {
        chunk_cb_opl_stdb(c, op & 0x07);
    }
    else {
        chunk_cb_opr_stdb(c, op & 0x07);
    }

	switch(HN) {
        case 0x0: PUSH_FUNC(LN < 8 ? op_rlc  : op_rrc); break;
        case 0x1: PUSH_FUNC(LN < 8 ? op_rl   : op_rr);  break;
        case 0x2: PUSH_FUNC(LN < 8 ? op_sla  : op_sra); break;
        case 0x3: PUSH_FUNC(LN < 8 ? op_swap : op_srl); break;

        default:
            switch((op&0xC0) >> 6) {
                case 0x01: PUSH_FUNC(op_bit); break;
                case 0x02: PUSH_FUNC(op_res); break;
                case 0x03: PUSH_FUNC(op_set); break;
            }
            c->opl.d = (op & 0x38)>>3;
    }


    if((op & 0x0F) == 0x06 || (op & 0x0F) == 0x0E) {
        if((op & 0xC0) == 0x40)
            MCS(3)
        else
            MCS(4)
    }
    else {
        MCS(2)
    }

    return c;
}

void op_create_chunks() {
    unsigned int b;
    for(b = 0; b <= 0xFF; b++) {
        op_chunk_map[b] = op_create_chunk(b);
        op_cb_chunk_map[b] = op_create_cb_chunk(b);
    }
}


