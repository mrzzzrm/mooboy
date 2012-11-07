#include "cpu_ops.h"

#include "cpu.h"
#include "cpu_defines.h"



void op_null(u8 op) {

}

void op_ld_rr(u8 op) {
    switch(op) {
        case 0x78: A = B; break;
        case 0x79: A = C; break;
        case 0x7A: A = D; break;
        case 0x7B: A = E; break;
        case 0x7C: A = H; break;
        case 0x7D: A = L; break;

        case 0x41: B = C; break;
        case 0x42: B = D; break;
        case 0x43: B = E; break;
        case 0x44: B = H; break;
        case 0x45: B = L; break;

        case 0x48: C = B; break;
        case 0x4A: C = D; break;
        case 0x4B: C = E; break;
        case 0x4C: C = H; break;
        case 0x4D: C = L; break;

        case 0x50: D = B; break;
        case 0x51: D = C; break;
        case 0x53: D = E; break;
        case 0x54: D = H; break;
        case 0x55: D = L; break;

        case 0x58: E = B; break;
        case 0x59: E = C; break;
        case 0x5A: E = D; break;
        case 0x5C: E = H; break;
        case 0x5D: E = L; break;

        case 0x60: H = B; break;
        case 0x61: H = C; break;
        case 0x62: H = D; break;
        case 0x63: H = E; break;
        case 0x65: H = L; break;

        case 0x68: L = B; break;
        case 0x69: L = C; break;
        case 0x6A: L = D; break;
        case 0x6B: L = E; break;
        case 0x6C: L = H; break;

        default: assert(0); break;
    }
}

void op_ld_ri(u8 op) {
    switch(op) {
        case 0x06: B = BFETCH; break;
        case 0x0E: C = BFETCH; break;
        case 0x16: D = BFETCH; break;
        case 0x1E: E = BFETCH; break;
        case 0x26: F = BFETCH; break;
        case 0x2E: G = BFETCH; break;
        case 0x3E: H = BFETCH; break;

        default: assert(0); break;
    }
}

void op_ld_rrm(u8 op) {
    switch(op) {
        case 0x0A: A = BC_RREF; break;
        case 0x1A: A = DE_RREF; break;
        case 0x7E: A = HL_RREF; break;

        case 0x46: B = HL_RREF; break;
        case 0x4E: C = HL_RREF; break;
        case 0x56: D = HL_RREF; break;
        case 0x5E: E = HL_RREF; break;
        case 0x66: H = HL_RREF; break;
        case 0x6E: L = HL_RREF; break;

        default: assert(0); break;
    }
}

void op_ld_rmr(u8 op) {
    switch(op) {
        case 0x02: BC_WREF(A); break;
        case 0x12: DE_WREF(A); break;
        case 0x77: HL_WREF(A); break;

        case 0x70: HL_WREF(B); break;
        case 0x71: HL_WREF(C); break;
        case 0x72: HL_WREF(D); break;
        case 0x73: HL_WREF(E); break;
        case 0x74: HL_WREF(H); break;
        case 0x75: HL_WREF(L); break;

        default: assert(0); break;
    }
}

void op_ld_mi(u8 op) {
    HL_WREF(BFETCH);
}

void op_ld_aim(u8 op) {
    A = MEM_RREF(WFETCH);
}

void op_ld_ima(u8 op) {
    MEM_WREF(WFETCH,  A);
}

void op_ld_aio(u8 op) {
    switch(op) {
        case 0xF0: A = MEM_RREF(0xFF00 + BFETCH); break;
        case 0xF2: A = MEM_RREF(0xFF00 + C); break;

        default: assert(0); break;
    }

}

void op_ld_ioa(u8 op) {
    switch(op) {
        case 0xE0: MEM_WREF(0xFF00 + BFETCH, A); break;
        case 0xE2: MEM_WREF(0xFF00 + C, A); break;

        default: assert(0); break;
    }
}

void op_ldx_am(u8 op) {
    switch(op) {
        case 0x2A: A = HL_RREF; HL++; break;
        case 0x3A: A = HL_RREF; HL--; break;

        default: assert(0); break;
    }
}

void op_ldx_ma(u8 op) {
    switch(op) {
        case 0x22: HL_WREF(A); HL++; break;
        case 0x32: HL_WREF(A); HL--; break;

        default: assert(0); break;
    }
}

void op_ld_rrim(u8 op) {
    switch(op) {
        case 0x01: BC = WFETCH; break;
        case 0x11: DE = WFETCH; break;
        case 0x21: HL = WFETCH; break;
        case 0x31: SP = WFETCH; break;
    }
}

void op_ld_imsp(u8 op) {
    MEM_WREF(WFETCH, SP);
}

void op_ldhl_spi(u8 op) {
    HL = SP + (s8)BFETCH;
    Z = 0;
    N = 0;
    H = ?;
    C = ?;
}

void op_ld_sphl(u8 op) {
    SP = HL;
}

void op_push_sp(u8 op) {
    switch(op) {
        case 0xF5: MEM_WREF(SP, AF); SP -= 2; break;
        case 0xC5: MEM_WREF(SP, BC); SP -= 2; break;
        case 0xD5: MEM_WREF(SP, DE); SP -= 2; break;
        case 0xE5: MEM_WREF(SP, HL); SP -= 2; break;

        default: assert(0); break;
    }
}

void op_pop_sp(u8 op) {
    switch(op) {
        case 0xF1: AF = MEM_RREF(SP); SP += 2; break;
        case 0xC1: BC = MEM_RREF(SP); SP += 2; break;
        case 0xD1: DE = MEM_RREF(SP); SP += 2; break;
        case 0xE1: HL = MEM_RREF(SP); SP += 2; break;

        default: assert(0); break;
    }
}

void op_add_r(u8 op) {

}

void op_add_i(u8 op) {

}

void op_add_m(u8 op) {

}

void op_adc_r(u8 op) {

}

void op_adc_i(u8 op) {

}

void op_adc_m(u8 op) {

}

void op_sub_r(u8 op) {

}

void op_sub_i(u8 op) {

}

void op_sub_m(u8 op) {

}

void op_sbc_r(u8 op) {

}

void op_sbc_i(u8 op) {

}

void op_sbc_m(u8 op) {

}

void op_inc_r(u8 op) {

}

void op_inc_m(u8 op) {

}

void op_dec_r(u8 op) {

}

void op_dec_m(u8 op) {

}

void op_and_r(u8 op) {

}

void op_and_i(u8 op) {

}

void op_and_m(u8 op) {

}

void op_xor_r(u8 op) {

}

void op_xor_i(u8 op) {

}

void op_xor_m(u8 op) {

}

void op_or_r(u8 op) {

}

void op_or_i(u8 op) {

}

void op_or_m(u8 op) {

}

void op_cp_r(u8 op) {

}

void op_cp_i(u8 op) {

}

void op_cp_m(u8 op) {

}

void op_daa(u8 op) {

}

void op_cpl(u8 op) {

}

void op_add_hlrr(u8 op) {

}

void op_add_spi(u8 op) {

}

void op_inc_rr(u8 op) {

}

void op_dec_rr(u8 op) {

}

void op_ld_hlspd(u8 op) {

}

void op_rla(u8 op) {

}

void op_rra(u8 op) {

}

void op_cb(u8 op) {

}

void op_rl_r(u8 op) {

}

void op_rl_m(u8 op) {

}

void op_rr_r(u8 op) {

}

void op_rr_m(u8 op) {

}

void op_sla_r(u8 op) {

}

void op_sla_m(u8 op) {

}

void op_sra_r(u8 op) {

}

void op_sra_m(u8 op) {

}

void op_swap_r(u8 op) {

}

void op_swap_m(u8 op) {

}

void op_bit_r(u8 op) {

}

void op_bit_m(u8 op) {

}

void op_set_r(u8 op) {

}

void op_set_m(u8 op) {

}

void op_res_r(u8 op) {

}

void op_res_m(u8 op) {

}

void op_nop(u8 op) {

}

void op_ccf(u8 op) {

}

void op_scf(u8 op) {

}

void op_halt(u8 op) {

}

void op_stop(u8 op) {

}

void op_di(u8 op) {

}

void op_ei(u8 op) {

}

void op_jp(u8 op) {

}

void op_jp_c(u8 op) {

}

void op_jp_m(u8 op) {

}

void op_jr(u8 op) {

}

void op_jr_c(u8 op) {

}

void op_call(u8 op) {

}

void op_call_c(u8 op) {

}

void op_rst(u8 op) {

}

void op_ret(u8 op) {

}

void op_ret_c(u8 op) {

}

void op_reti(u8 op) {

}
