#ifndef CPU_TABLES_H
#define CPU_TABLES_H

    #include <stdlib.h>
    #include "cpu_ops.h"

    void (*op_func_map[])() = {
         /* X0 */   /* X1 */    /* X2 */    /* X3 */    /* X4 */    /* X5 */    /* X6 */    /* X7 */    /* X8 */    /* X9 */    /* XA */    /* XB */    /* XC */    /* XD */    /* XE */    /* XF */
/* 0Y */ op_nop,    op_ld_rri,  op_ld_rmr,  op_inc_w,   op_inc_b,   op_dec_b,   op_ld_ri,   op_rlc,     op_ld_imsp, op_add_hlrr,op_ld_rrm,  op_dec_w,   op_inc_b,   op_dec_b,   op_ld_ri,   op_rr,
/* 1Y */ op_stop,   op_ld_rri,  op_ld_rmr,  op_inc_w,   op_inc_b,   op_dec_b,   op_ld_ri,   op_rl,      op_jr,      op_add_hlrr,op_ld_rrm,  op_dec_w,   op_inc_b,   op_dec_b,   op_ld_ri,   op_rr,
/* 2Y */ op_jr,     op_ld_rri,  op_ldx_hlma,op_inc_w,   op_inc_b,   op_dec_b,   op_ld_ri,   op_daa,     op_jr,      op_add_hlrr,op_ldx_ahlm,op_dec_w,   op_inc_b,   op_dec_b,   op_ld_ri,   op_cpl,
/* 3Y */ op_jr,     op_ld_rri,  op_ldx_hlma,op_inc_w,   op_inc_b,   op_dec_b,   op_ld_rmi,  op_scf,     op_jr,      op_add_hlrr,op_ldx_ahlm,op_dec_w,   op_inc_b,   op_dec_b,   op_ld_ri,   op_ccf,
/* 4Y */ op_ld_rr,  op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rrm,  op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rrm,  op_ld_rr,
/* 5Y */ op_ld_rr,  op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rrm,  op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rrm,  op_ld_rr,
/* 6Y */ op_ld_rr,  op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rrm,  op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rrm,  op_ld_rr,
/* 7Y */ op_ld_rmr, op_ld_rmr,  op_ld_rmr,  op_ld_rmr,  op_ld_rmr,  op_ld_rmr,  op_halt,    op_ld_rmr,  op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rrm,  op_ld_rr,
/* 8Y */ op_add,    op_add,     op_add,     op_add,     op_add,     op_add,     op_add,     op_add,     op_adc,     op_adc,     op_adc,     op_adc,     op_adc,     op_adc,     op_adc,     op_adc,
/* 9Y */ op_sub,    op_sub,     op_sub,     op_sub,     op_sub,     op_sub,     op_sub,     op_sub,     op_sbc,     op_sbc,     op_sbc,     op_sbc,     op_sbc,     op_sbc,     op_sbc,     op_sbc,
/* AY */ op_and,    op_and,     op_and,     op_and,     op_and,     op_and,     op_and,     op_and,     op_xor,     op_xor,     op_xor,     op_xor,     op_xor,     op_xor,     op_xor,     op_xor,
/* BY */ op_or,     op_or,      op_or,      op_or,      op_or,      op_or,      op_or,      op_or,      op_cp,      op_cp,      op_cp,      op_cp,      op_cp,      op_cp,      op_cp,      op_cp,
/* CY */ op_ret,    op_pop_sp,  op_jp,      op_jp,      op_call,    op_push_sp, op_add,     op_rst,     op_ret,     op_ret,     op_jp,      op_cb,      op_call,    op_call,    op_adc,     op_rst,
/* DY */ op_ret,    op_pop_sp,  op_jp,      op_null,    op_call,    op_push_sp, op_sub,     op_rst,     op_ret,     op_reti,    op_jp,      op_null,    op_call,    op_null,    op_sbc,     op_rst,
/* EY */ op_ld_iioa,op_pop_sp,  op_ld_cioa, op_null,    op_null,    op_push_sp, op_and,     op_rst,     op_add_spi, op_jp,      op_ld_ima,  op_null,    op_null,    op_null,    op_xor,     op_rst,
/* FY */ op_ld_aiio,op_pop_sp,  op_ld_acio, op_di,      op_null,    op_push_sp, op_or,      op_rst,     op_ldhl_spi,op_ld_sphl, op_ld_aim,  op_ei,      op_null,    op_null,    op_cp,      op_rst
    };

#endif

