#ifndef CPU_OPS_H
#define CPU_OPS_H

    #include "util/defines.h"
    #include "chunks.h"

    // Prefunctions
    void op_opl_memcall(op_chunk *c);
    void op_opl_ib(op_chunk *c);
    void op_opl_iw(op_chunk *c);
    void op_opl_addio(op_chunk *c);
    void op_opr_memread(op_chunk *c);
    void op_opr_ib(op_chunk *c);
    void op_opr_iw(op_chunk *c);
    void op_opr_addio(op_chunk *c);

    // No OP
    void op_null(op_chunk *c);

    // 8-Bit Loads
    void op_ld_b(op_chunk *c);
    void op_ld_w(op_chunk *c);
    void op_ldx(op_chunk *c);

    // 16-Bit Loads
    void op_ldhl_spi(op_chunk *c);
    void op_ld_imsp(op_chunk *c);
    void op_push(op_chunk *c);
    void op_pop(op_chunk *c);

    // Alu Arithmetic
    void op_add_b(op_chunk *c);
    void op_add_w(op_chunk *c);
    void op_adc(op_chunk *c);
    void op_sub(op_chunk *c);
    void op_sbc(op_chunk *c);
    void op_inc_b(op_chunk *c);
    void op_dec_b(op_chunk *c);
    void op_inc_w(op_chunk *c);
    void op_dec_w(op_chunk *c);
    void op_add_spi(op_chunk *c);

    // Logic
    void op_and(op_chunk *c);
    void op_xor(op_chunk *c);
    void op_or(op_chunk *c);
    void op_cp(op_chunk *c);
    void op_cpl(op_chunk *c);

    // Rotates and Shifts
    void op_rl(op_chunk *c);
    void op_rr(op_chunk *c);
    void op_rlc(op_chunk *c);
    void op_rrc(op_chunk *c);
    void op_sla(op_chunk *c);
    void op_sra(op_chunk *c);
    void op_srl(op_chunk *c);

    // Misc
    void op_cb(op_chunk *c);
    void op_swap(op_chunk *c);
    void op_daa(op_chunk *c);

    // Singlebit Operations
    void op_bit(op_chunk *c);
    void op_set(op_chunk *c);
    void op_res(op_chunk *c);

    // CPU Control
    void op_nop(op_chunk *c);
    void op_ccf(op_chunk *c);
    void op_scf(op_chunk *c);
    void op_halt(op_chunk *c);
    void op_stop(op_chunk *c);
    void op_di(op_chunk *c);
    void op_ei(op_chunk *c);

    // Jumps
    void op_jp(op_chunk *c);
    void op_jr(op_chunk *c);
    void op_call(op_chunk *c);
    void op_rst(op_chunk *c);
    void op_ret(op_chunk *c);
    void op_reti(op_chunk *c);


#endif // CPU_OPS_H
