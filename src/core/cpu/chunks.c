#include "chunks.h"

#include "../cpu.h"
#include "ops.h"
#include "defines.h"


#define PUSH_FUNC(f) c->funcs[c->sp++] = (f)

op_chunk *op_chunk_map[0xFF];
op_chunk *op_cb_chunk_map[0xFF];

static u8 *bregv[] = {&B, &C, &D, &E, &H, &L, NULL, &A};
static u16 *wregv_sp[] = {&BC, &DE, &HL, &SP};
static u16 *wregv_af[] = {&BC, &DE, &HL, &AF};

static void chunk_opl_stdb(op_chunk *c, u8 b) {
    if(b == 0x06) {
        PUSH_FUNC(op_opl_memcall);
        c->opl.w = &HL;
    }
    else {
        c->opl.b = BREG(b);
    }
}

static void chunk_opr_stdb(op_chunk *c, u8 b) {
    if(b == 0x06) {
        PUSH_FUNC(op_opr_memread);
        c->opr.w = &HL;
    }
    else {
        c->opr.b = BREG(b);
    }
}

static void chunk_func_alu(op_chunk *c, u8 b) {
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

op_chunk *op_create_chunk(u8 op) {
	op_chunk *c = malloc(sizeof(op_chunk));
	c->op = op;
	c->sp = 0;

    switch(op) {
        case 0x00: PUSH_FUNC(op_nop); break;
        case 0x08: PUSH_FUNC(op_ld_imsp); break;
        case 0x10: PUSH_FUNC(op_stop); break;
        case 0x18: PUSH_FUNC(op_jr); break;
        case 0x27: PUSH_FUNC(op_daa); break;
        case 0x2F: PUSH_FUNC(op_cpl); break;
        case 0x37: PUSH_FUNC(op_scf); break;
        case 0x3F: PUSH_FUNC(op_ccf); break;
        case 0xC3:
            PUSH_FUNC(op_opl_iw);
            PUSH_FUNC(op_jp);
        break;
        case 0xC9: PUSH_FUNC(op_ret); break;
        case 0xCB: PUSH_FUNC(op_cb); break;
        case 0xCD:
            PUSH_FUNC(op_opl_iw);
            PUSH_FUNC(op_call);
        break;
        case 0xD9: PUSH_FUNC(op_reti); break;
        case 0xE8: PUSH_FUNC(op_add_spi); break;
        case 0xE9: PUSH_FUNC(op_jp); break;
        case 0xF3: PUSH_FUNC(op_di); break;
        case 0xFB: PUSH_FUNC(op_ei); break;
        case 0xF8: PUSH_FUNC(op_ldhl_spi); break;
        case 0xF9:
            c->opl.w = &SP;
            c->opr.w = &HL;
            PUSH_FUNC(op_ld_w);
        break;

        default:
            switch((op&0xC0)>>6) {
                case 0x00:
                    switch(op & 0x07) {
                        case 0x00:
                            PUSH_FUNC(op_opl_ib);
                            PUSH_FUNC(op_jr);
                        break;
                        case 0x01:
                            if(op & 0x08) {
                                c->opl.w = &HL;
                                c->opr.w = WREG_SP((op&0x30) >> 4);
                                PUSH_FUNC(op_add_w);
                            }
                            else {
                                c->opl.w = WREG_SP((op&0x30) >> 4);
                                PUSH_FUNC(op_opr_iw);
                                PUSH_FUNC(op_ld_w);
                            }
                        break;
                        case 0x02:
                            if(op & 0x20) {
                                switch((op&0x18)>>3) {
                                    case 0x00: case 0x02:
                                        c->opr.b = &A;
                                        c->opl.w = &HL;
                                        PUSH_FUNC(op_opl_memcall);
                                    break;
                                    case 0x01: case 0x03:
                                        c->opr.w = &HL;
                                        c->opl.b = &A;
                                        PUSH_FUNC(op_opr_memread);
                                    break;
                                }
                                PUSH_FUNC(op_ldx);
                            }
                            else {
                                if(op & 0x08) {
                                    c->opl.b = &A;
                                    c->opr.w = WREG_SP((op&0x10)>>4);
                                }
                                else {
                                    c->opl.w = WREG_SP((op&0x10)>>4);
                                    c->opr.b = &A;
                                }
                            }
                        break;
                        case 0x03:
                            c->opl.w = WREG_SP((op&0x30)>>4);
                            PUSH_FUNC(op&0x80 ? op_dec_w : op_inc_w);
                        break;
                        case 0x05:
                            chunk_opl_stdb(c, (op&38)>>3);
                            PUSH_FUNC(op_inc_b);
                        break;
                        case 0x06:
                            chunk_opl_stdb(c, (op&38)>>3);
                            PUSH_FUNC(op_dec_b);
                        break;
                        case 0x07:
                            c->opl.b = &A;
                            if(op&0x10) {
                                PUSH_FUNC(op&0x80 ? op_rr : op_rl);
                            }
                            else {
                                PUSH_FUNC(op&0x80 ? op_rrc : op_rlc);
                            }
                        break;
                    }
                break;
                case 0x01:
                    if(op == 0x76) {
                        PUSH_FUNC(op_halt);
                    }
                    else {
                        chunk_opr_stdb(c, op & 0x07);
                        chunk_opl_stdb(c, (op & 0x38)>>3);
                        PUSH_FUNC(op_ld_b);
                    }
                break;
                case 0x02:
                    c->opl.b = &A;
                    chunk_opr_stdb(c, op & 0x07);
                    chunk_func_alu(c, (op & 0x38) >> 3);
                break;
                case 0x03:
                    switch(op & 0x03) {
                        case 0x01:
                            c->opl.w = WREG_AF((op&0x30)>>4);
                            PUSH_FUNC(op_pop);
                        break;
                        case 0x04:
                            PUSH_FUNC(op_opl_iw);
                            PUSH_FUNC(op_call);
                        break;
                        case 0x05:
                            c->opl.w = WREG_AF((op&0x30)>>4);
                            PUSH_FUNC(op_push);
                        break;
                        case 0x06:
                            c->opl.b = &A;
                            PUSH_FUNC(op_opr_ib);
                            chunk_func_alu(c, (op&0x38)>>3);
                        break;
                        case 0x07: PUSH_FUNC(op_rst); break;

                        default:
                            if((op & 0xE0) == 0xC0) {
                                if(op & 0x02) {
                                    PUSH_FUNC(op_opl_iw);
                                    PUSH_FUNC(op_jp);
                                }
                                else {
                                    PUSH_FUNC(op_ret);
                                }
                            }
                            else {
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
                                    break;
                                    case 0x0A:
                                        if(op & 0x10) {
                                            PUSH_FUNC(op_opr_iw);
                                        }
                                        else {
                                            PUSH_FUNC(op_opl_iw);
                                        }
                                    break;
                                }
                                PUSH_FUNC(op_ld_b);
                            }
                    }
                break;
            }
    }



	return c;
}


op_chunk *op_create_cb_chunk(u8 op) {
    op_chunk *c = malloc(sizeof(op_chunk));
	c->op = op;

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
            chunk_opl_stdb(c, op & 0x07);
            c->i = (op & 0x38)>>3;
    }
    return c;
}

