#ifndef CPU_OPS_H
#define CPU_OPS_H

    // No OP
    void op_null(u8 op);

    // 8-Bit Loads
    void op_ld_rr(u8 op);
    void op_ld_ri(u8 op);
    void op_ld_mr(u8 op);
    void op_ld_mi(u8 op);

    void op_ld_rrm(u8 op);
    void op_ld_rmr(u8 op);
    void op_ld_aim(u8 op);
    void op_ld_ima(u8 op);

    void op_ld_aio(u8 op);
    void op_ld_ioa(u8 op);

    void op_ldx_am(u8 op);
    void op_ldx_ma(u8 op);

    // 16-Bit Loads
    void op_ld_rrim(u8 op);
    void op_ld_imsp(u8 op);
    void op_ldhl_spi(u8 op);
    void op_ld_sphl(u8 op);
    void op_push_sp(u8 op);
    void op_pop_sp(u8 op);

    // 8-Bit Arithmetic
    void op_add_r(u8 op);
    void op_add_i(u8 op);
    void op_add_m(u8 op);
    void op_adc_r(u8 op);
    void op_adc_i(u8 op);
    void op_adc_m(u8 op);

    void op_sub_r(u8 op);
    void op_sub_i(u8 op);
    void op_sub_m(u8 op);
    void op_sbc_r(u8 op);
    void op_sbc_i(u8 op);
    void op_sbc_m(u8 op);

    void op_inc_r(u8 op);
    void op_inc_m(u8 op);
    void op_dec_r(u8 op);
    void op_dec_m(u8 op);

    // Logic
    void op_and_r(u8 op);
    void op_and_i(u8 op);
    void op_and_m(u8 op);
    void op_xor_r(u8 op);
    void op_xor_i(u8 op);
    void op_xor_m(u8 op);
    void op_or_r(u8 op);
    void op_or_i(u8 op);
    void op_or_m(u8 op);
    void op_cp_r(u8 op);
    void op_cp_i(u8 op);
    void op_cp_m(u8 op);
    void op_daa(u8 op);
    void op_cpl(u8 op);

    // 16-Bit Arithmetic
    void op_add_hlrr(u8 op);
    void op_add_spi(u8 op);
    void op_inc_rr(u8 op);
    void op_dec_rr(u8 op);
    void op_ld_hlspd(u8 op);

    // Rotates and Shifts
    void op_rla(u8 op);
    void op_rra(u8 op);

    void op_cb(u8 op);
    void op_rl_r(u8 op);
    void op_rl_m(u8 op);
    void op_rr_r(u8 op);
    void op_rr_m(u8 op);
    void op_sla_r(u8 op);
    void op_sla_m(u8 op);
    void op_sra_r(u8 op);
    void op_sra_m(u8 op);
    void op_swap_r(u8 op);
    void op_swap_m(u8 op);

    // Singlebit Operations
    void op_bit_r(u8 op);
    void op_bit_m(u8 op);
    void op_set_r(u8 op);
    void op_set_m(u8 op);
    void op_res_r(u8 op);
    void op_res_m(u8 op);

    // CPU Control
    void op_nop(u8 op);
    void op_ccf(u8 op);
    void op_scf(u8 op);
    void op_halt(u8 op);
    void op_stop(u8 op);
    void op_di(u8 op);
    void op_ei(u8 op);

    // Jumps
    void op_jp(u8 op);
    void op_jp_c(u8 op);
    void op_jp_m(u8 op);
    void op_jr(u8 op);
    void op_jr_c(u8 op);
    void op_call(u8 op);
    void op_call_c(u8 op);
    void op_rst(u8 op);
    void op_ret(u8 op);
    void op_ret_c(u8 op);
    void op_reti(u8 op);


#endif // CPU_OPS_H
