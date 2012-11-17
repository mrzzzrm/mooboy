#ifndef CPU_OPS_H
#define CPU_OPS_H

    #include "util/defines.h"

    typedef union op_arg_u {
        u8 d;
        u8 *b;
        u16 *w;
    } op_arg;

    typedef struct op_chunk_s op_chunk;
    struct op_chunk_s {
        u8 op;
        u8 i, b; // Only used for prefix CB; immediate and bit index
        void (*func)(op_chunk *c);
        op_arg opl, opr; // Pointers to 8/16 Bit registers
    };

    extern op_chunk *op_chunk_map[0xFF];
    extern op_chunk *op_cb_chunk_map[0xFF];

    op_chunk *op_create_chunk(u8 op);

    // No OP
    void op_null(op_chunk *c);

    // 8-Bit Loads
    void op_ld_rr(op_chunk *c);
    void op_ld_ri(op_chunk *c);
    void op_ld_rrm(op_chunk *c);
    void op_ld_rmr(op_chunk *c);
    void op_ld_rmi(op_chunk *c);
    void op_ld_aim(op_chunk *c);
    void op_ld_ima(op_chunk *c);
    void op_ld_acio(op_chunk *c);
    void op_ld_cioa(op_chunk *c);
    void op_ld_aiio(op_chunk *c);
    void op_ld_iioa(op_chunk *c);
    void op_ldx_hlma(op_chunk *c);
    void op_ldx_ahlm(op_chunk *c);

    // 16-Bit Loads
    void op_ld_rri(op_chunk *c);
    void op_ld_sphl(op_chunk *c);
    void op_ldhl_spi(op_chunk *c);
    void op_ld_imsp(op_chunk *c);
    void op_push_sp(op_chunk *c);
    void op_pop_sp(op_chunk *c);

    // 8-Bit Arithmetic
    void op_add(op_chunk *c);
    void op_adc(op_chunk *c);
    void op_sub(op_chunk *c);
    void op_sbc(op_chunk *c);
    void op_inc_b(op_chunk *c);
    void op_dec_b(op_chunk *c);

    // Logic
    void op_and(op_chunk *c);
    void op_xor(op_chunk *c);
    void op_or(op_chunk *c);
    void op_cp(op_chunk *c);
    void op_cpl(op_chunk *c);
    void op_daa(op_chunk *c);

    // 16-Bit Arithmetic
    void op_add_hlrr(op_chunk *c);
    void op_add_spi(op_chunk *c);
    void op_inc_w(op_chunk *c);
    void op_dec_w(op_chunk *c);

    // Rotates and Shifts
    void op_cb(op_chunk *c);

    void op_rl(op_chunk *c);
    void op_rr(op_chunk *c);
    void op_rlc(op_chunk *c);
    void op_rrc(op_chunk *c);
    void op_sla(op_chunk *c);
    void op_sra(op_chunk *c);
    void op_srl(op_chunk *c);

    // Misc
    void op_swap(op_chunk *c);
    void op_daa(op_chunk *c);
    void op_cpl(op_chunk *c);


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
