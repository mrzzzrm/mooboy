#ifndef CPU_TABLES_H
#define CPU_TABLES_H

    #include <stdlib.h>
    #include "cpu_ops.h"

    void (*cpu_ops[])() = {
         /* X0 */   /* X1 */    /* X2 */    /* X3 */    /* X4 */    /* X5 */    /* X6 */    /* X7 */    /* X8 */    /* X9 */    /* XA */    /* XB */    /* XC */    /* XD */    /* XE */    /* XF */
/* 0Y */ op_nop,    op_ld_rrim, op_ld_rmr,  op_inc_rr,  op_inc_r,   op_dec_r,   op_ld_ri,   op_rla,     op_ld_imsp, op_add_hlrr,op_ld_rrm,  op_dec_rr,  op_inc_r,   op_dec_r,   op_ld_ri,   op_rra,
/* 1Y */ op_stop,   op_ld_rrim, op_ld_rmr,  op_inc_rr,  op_inc_r,   op_dec_r,   op_ld_ri,   op_rla,     op_jr,      op_add_hlrr,op_ld_rrm,  op_dec_rr,  op_inc_r,   op_dec_r,   op_ld_ri,   op_rra,
/* 2Y */ op_jr_c,   op_ld_rrim, op_ldx_ma,  op_inc_rr,  op_inc_r,   op_dec_r,   op_ld_ri,   op_daa,     op_jr_c,    op_add_hlrr,op_ldx_am,  op_dec_rr,  op_inc_r,   op_dec_r,   op_ld_ri,   op_cpl,
/* 3Y */ op_jr_c,   op_ld_rrim, op_ldx_ma,  op_inc_rr,  op_inc_m,   op_dec_m,   op_ld_mi,   op_scf,     op_jr_c,    op_add_hlrr,op_ldx_am,  op_dec_rr,  op_inc_r,   op_dec_r,   op_ld_ri,   op_ccf,
/* 4Y */ op_ld_rr,  op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rrm,  op_ld_rr,
/* 5Y */ op_ld_rr,  op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rrm,  op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rrm,  op_ld_rr,
/* 6Y */ op_ld_rr,  op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rrm,  op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rrm,  op_ld_rr,
/* 7Y */ op_ld_mr,  op_ld_mr,   op_ld_mr,   op_ld_mr,   op_ld_mr,   op_ld_mr,   op_halt,    op_ld_rmr,  op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rr,   op_ld_rrm,  op_ld_rr,
/* 8Y */ op_add_r,  op_add_r,   op_add_r,   op_add_r,   op_add_r,   op_add_r,   op_add_m,   op_add_r,   op_adc_r,   op_adc_r,   op_adc_r,   op_adc_r,   op_adc_r,   op_adc_r,   op_adc_m,   op_adc_r,
/* 9Y */ op_sub_r,  op_sub_r,   op_sub_r,   op_sub_r,   op_sub_r,   op_sub_r,   op_sub_m,   op_or_r,    op_or_r,    op_or_r,    op_or_r,    op_or_r,    op_or_r,    op_or_r,    op_or_m,    op_or_r,
/* AY */ op_and_r,  op_and_r,   op_and_r,   op_and_r,   op_and_r,   op_and_r,   op_and_m,   op_and_r,   op_xor_r,   op_xor_r,   op_xor_r,   op_xor_r,   op_xor_r,   op_xor_r,   op_xor_m,   op_xor_r,
/* BY */ op_or_r,   op_or_r,    op_or_r,    op_or_r,    op_or_r,    op_or_r,    op_or_m,    op_or_r,    op_cp_r,    op_cp_r,    op_cp_r,    op_cp_r,    op_cp_r,    op_cp_r,    op_cp_m,    op_cp_r,
/* CY */ op_ret_c,  op_pop_sp,  op_jp_c,    op_jp,      op_call_c,  op_push_sp, op_add_i,   op_rst,     op_ret_c,   op_ret,     op_jp_c,    op_cb,      op_call_c,  op_call,    op_adc_i,   op_rst,
/* DY */ op_ret_c,  op_pop_sp,  op_jp_c,    op_null,    op_call_c,  op_push_sp, op_sub_i,   op_rst,     op_ret_c,   op_reti,    op_jp_c,    op_null,    op_call_c,  op_null,    op_sbc_i,   op_rst,
/* EY */ op_ld_ioa, op_pop_sp,  op_ld_ioa,  op_null,    op_null,    op_push_sp, op_and_i,   op_rst,     op_add_spi, op_jp_m,    op_ld_ima,  op_null,    op_null,    op_null,    op_and_i,   op_rst,
/* FY */ op_ld_aio, op_pop_sp,  op_ld_aio,  op_null,    op_null,    op_push_sp, op_or_i,    op_rst,     op_ldhl_spi,op_ld_sphl, op_ld_aim,  op_ei,      op_null,    op_null,    op_or_i,    op_rst
    };

#endif

