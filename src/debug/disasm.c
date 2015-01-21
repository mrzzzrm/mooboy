#include "disasm.h"

#include <stdio.h>

#include <assert.h>

#include "core/cpu.h"
#include "core/mem.h"

static char out_disasm_str[256] = { '\0' };
static u16 pc;
static u8 op;
static op_t c_op;
static op_t str_op;

static const char* op_str[_OP_COUNT_] = {
    "UNDEFINED",
    "NOP",
    "STOP",
    "JR",
    "LD",
    "LD",
    "LD",
    "LD",
    "LD",
    "PUSH",
    "POP",
    "INC",
    "DEC",
    "ADD",
    "SUB",
    "RLCA",
    "RRCA",
    "RLA",
    "RRA",
    "DAA",
    "RLL",
    "SCF",
    "CCF",
    "HALT",
    "ADC",
    "SBC",
    "JP",
    "CALL",
    "RST"
    "AND",
    "XOR",
    "OR",
    "CP",
    "RET",
    "RETI",
    "LDH",
    "DI",
    "EI",
    "RLC",
    "RRC",
    "RL",
    "RR",
    "SLA",
    "SRA",
    "SWAP",
    "SRL",
    "BIT",
    "RES",
    "SET"
};

static const char* data_str[_DATA_COUNT_] = {
    "A8",
    "A16",
    "R8",
    "D8",
    "D16"
};

static const char* flag_str[_FLAG_COUNT_] = {
    "Z",
    "NZ",
    "C",
    "NC"
};

static const char* reg_str[_REG_COUNT_] = {
    "A",
    "B",
    "C",
    "D",
    "E",
    "H",
    "L",
    "F",
    "SP",
    "PC",
    "AF",
    "BC",
    "DE",
    "HL"
};

static const char* data_val(DATA data) {
    static char tmp_str[256];

    switch(data) {
        case DATA_A8: {
            u8 a8 = mem_read_byte(pc + 1);
            sprintf(tmp_str, "%.2X", (int)a8);
            return tmp_str;
        }

        case DATA_A16: {
            u16 a16 = mem_read_word(pc + 1);
            sprintf(tmp_str, "%.2X", (int)a16);
            return tmp_str;
        }

        case DATA_R8: {
            u8 r8 = mem_read_byte(pc + 1);
            sprintf(tmp_str, "%s%.2X", r8 > 126 ? "-" : "", (int)(r8 > 126 ? (u8)(~r8 + 1) : r8));
            return tmp_str;
        }

        case DATA_D8: {
            u8 d8 = mem_read_byte(pc + 1);
            sprintf(tmp_str, "%.2X", (int)d8);
            return tmp_str;
        }

        case DATA_D16: {
            u16 d16 = mem_read_word(pc + 1);
            sprintf(tmp_str, "%.2X", (int)d16);
            return tmp_str;
        }

        default:
            assert(0);
    }
}

static u16 reg_val(REG r) {
    switch(r) {
        case REG_A: return A;
        case REG_B: return B;
        case REG_C: return C;
        case REG_D: return D;
        case REG_E: return E;
        case REG_H: return H;
        case REG_L: return L;
        case REG_F: return F;
        case REG_SP: return SP;
        case REG_PC: return PC;
        case REG_AF: return AF;
        case REG_BC: return BC;
        case REG_DE: return DE;
        case REG_HL: return HL;

        default:
            assert(0);
    }
}

static const char* flag_val(FLAG flag) {
    static char tmp_str[256];

    int val;

    switch (flag) {
        case FLAG_Z: val = FZ; break;
        case FLAG_NZ: val = !FZ; break;
        case FLAG_C: val = FC; break;
        case FLAG_NC: val = !FC; break;

        default:
            assert(0);
    }

    sprintf(tmp_str, "%s", val ? "true" : "false");

    return tmp_str;
}

static void str_noarg() {
    snprintf(out_disasm_str, sizeof(out_disasm_str), "%s", op_str[str_op.op]);
}

static void str_fd() {
    snprintf(out_disasm_str, sizeof(out_disasm_str), "%s %s(%s), %s", op_str[str_op.op], flag_str[str_op.f], flag_val(str_op.f), data_val(str_op.d));
}

static void str_r() {
    snprintf(out_disasm_str, sizeof(out_disasm_str), "%s %s(%.4X)", op_str[str_op.op], reg_str[str_op.r1], reg_val(str_op.r1));
}

static void str_rm() {
    snprintf(out_disasm_str, sizeof(out_disasm_str), "%s (%s)(%.4X)", op_str[str_op.op], reg_str[str_op.r1], mem_read_word(reg_val(str_op.r1)));
}

static void str_rr() {
    snprintf(out_disasm_str, sizeof(out_disasm_str), "%s %s, %s(%.4X)", op_str[str_op.op], reg_str[str_op.r1], reg_str[str_op.r2], reg_val(str_op.r2));
}

static void str_rrm() {
    snprintf(out_disasm_str, sizeof(out_disasm_str), "%s %s, (%s)(%.4X)", op_str[str_op.op], reg_str[str_op.r1], reg_str[str_op.r2], mem_read_word(reg_val(str_op.r2)));
}

static void str_rmd() {
    snprintf(out_disasm_str, sizeof(out_disasm_str), "%s (%s)(%.4X) %s(%s)", op_str[str_op.op], reg_str[str_op.r1], mem_read_word(reg_val(str_op.r1)), data_str[str_op.d], data_val(str_op.d));
}

static void str_rd() {
    snprintf(out_disasm_str, sizeof(out_disasm_str), "%s %s(%.4X), (%s)(%s)", op_str[str_op.op], reg_str[str_op.r1], reg_val(str_op.r1), data_str[str_op.d], data_val(str_op.d));
}

static void str_d() {
    snprintf(out_disasm_str, sizeof(out_disasm_str), "%s %s(%s)", op_str[str_op.op], data_str[str_op.d], data_val(str_op.d));
}

static void str_rmr() {
    snprintf(out_disasm_str, sizeof(out_disasm_str), "%s (%s)(%.4X), %s(%.4X)", op_str[str_op.op], reg_str[str_op.r1], mem_read_word(reg_val(str_op.r1)), reg_str[str_op.r2], reg_val(str_op.r2));
}

static void str_n() {
    snprintf(out_disasm_str, sizeof(out_disasm_str), "%s %.2X", op_str[str_op.op], str_op.n);
}

static void str_f() {
    snprintf(out_disasm_str, sizeof(out_disasm_str), "%s %s(%s)", op_str[str_op.op], flag_str[str_op.f], flag_val(str_op.f));
}

static void str_dmr() {
    snprintf(out_disasm_str, sizeof(out_disasm_str), "%s (%s)(%s) %s(%.4X)", op_str[str_op.op], data_str[str_op.d], data_val(str_op.d), reg_str[str_op.r1], reg_val(str_op.r1));
}

static void str_rdm() {
    snprintf(out_disasm_str, sizeof(out_disasm_str), "%s %s(%.4X) (%s)(%s)", op_str[str_op.op], reg_str[str_op.r1], reg_val(str_op.r1), data_str[str_op.d], data_val(str_op.d));
}

static void str_rrd() {
    snprintf(out_disasm_str, sizeof(out_disasm_str), "%s %s(%.4X) %s(%.4X) + %s(%s)", op_str[str_op.op], reg_str[str_op.r1], reg_val(str_op.r1), reg_str[str_op.r2], reg_val(str_op.r2), data_str[str_op.d], data_val(str_op.d));
}

static void str_rn() {
    snprintf(out_disasm_str, sizeof(out_disasm_str), "%s %s(%.4X), %i", op_str[str_op.op], reg_str[str_op.r1], reg_val(str_op.r1), str_op.n);
}

static void str_rmn() {
    snprintf(out_disasm_str, sizeof(out_disasm_str), "%s (%s)(%.4X), %i", op_str[str_op.op], reg_str[str_op.r1], reg_val(str_op.r1), str_op.n);
}

static void build_noarg(OP op) {
    c_op.op = op;
    c_op.sig = OP_SIG_NOARG;
}

static void build_fd(OP op, FLAG flag, DATA data) {
    c_op.op = op;
    c_op.sig = OP_SIG_FD;
    c_op.f = flag;
    c_op.d = data;
}

static void build_r(OP op, REG r) {
    c_op.op = op;
    c_op.sig = OP_SIG_R;
    c_op.r1 = r;
}

static void build_rm(OP op, REG r) {
    c_op.op = op;
    c_op.sig = OP_SIG_RM;
    c_op.r1 = r;
}

static void build_rr(OP op, REG r1, REG r2) {
    c_op.op = op;
    c_op.sig = OP_SIG_RR;
    c_op.r1 = r1;
    c_op.r2 = r2;
}

static void build_rrm(OP op, REG r1, REG r2) {
    c_op.op = op;
    c_op.sig = OP_SIG_RRM;
    c_op.r1 = r1;
    c_op.r2 = r2;
}

static void build_rmd(OP op, REG r, DATA d) {
    c_op.op = op;
    c_op.sig = OP_SIG_RMD;
    c_op.r1 = r;
    c_op.d = d;
}

static void build_rd(OP op, REG r, DATA d) {
    c_op.op = op;
    c_op.sig = OP_SIG_RD;
    c_op.r1 = r;
    c_op.d = d;
}

static void build_d(OP op, DATA d) {
    c_op.op = op;
    c_op.sig = OP_SIG_D;
    c_op.d = d;
}

static void build_rmr(OP op, REG r1, REG r2) {
    c_op.op = op;
    c_op.sig = OP_SIG_RMR;
    c_op.r1 = r1;
    c_op.r2 = r2;
}

static void build_n(OP op, u8 n) {
    c_op.op = op;
    c_op.sig = OP_SIG_N;
    c_op.n = n;
}

static void build_f(OP op, FLAG f) {
    c_op.op = op;
    c_op.sig = OP_SIG_F;
    c_op.f = f;
}

static void build_dmr(OP op, DATA d, REG r) {
    c_op.op = op;
    c_op.sig = OP_SIG_RR;
    c_op.d = d;
    c_op.r1 = r;
}

static void build_rdm(OP op, REG r, DATA d) {
    c_op.op = op;
    c_op.sig = OP_SIG_RDM;
    c_op.r1 = r;
    c_op.d = d;
}

static void build_rrd(OP op, REG r1, REG r2, DATA d) {
    c_op.op = op;
    c_op.sig = OP_SIG_RR;
    c_op.r1 = r1;
    c_op.r2 = r2;
    c_op.d = d;
}

static void build_rn(OP op, REG r, u8 n) {
    c_op.op = op;
    c_op.sig = OP_SIG_RR;
    c_op.r1 = r;
    c_op.n = n;
}

static void build_rmn(OP op, REG r, u8 n) {
    c_op.op = op;
    c_op.sig = OP_SIG_RMN;
    c_op.r1 = r;
    c_op.n = n;
}

#define CB_OP_CASES_NOARG(base, op) \
    case base + 0: build_r(op, REG_B); break; \
    case base + 1: build_r(op, REG_C); break; \
    case base + 2: build_r(op, REG_D); break; \
    case base + 3: build_r(op, REG_E); break; \
    case base + 4: build_r(op, REG_H); break; \
    case base + 5: build_r(op, REG_L); break; \
    case base + 6: build_rm(op, REG_HL); break; \
    case base + 7: build_r(op, REG_L); break;

#define CB_OP_CASES_ARG(base, op, arg) \
    case base + 0: build_rn(op, REG_B, arg); break; \
    case base + 1: build_rn(op, REG_C, arg); break; \
    case base + 2: build_rn(op, REG_D, arg); break; \
    case base + 3: build_rn(op, REG_E, arg); break; \
    case base + 4: build_rn(op, REG_H, arg); break; \
    case base + 5: build_rn(op, REG_L, arg); break; \
    case base + 6: build_rmn(op, REG_HL, arg); break; \
    case base + 7: build_rn(op, REG_A, arg); break;

#define CB_BITMANIP_CASES(base, op) \
    CB_OP_CASES_ARG(base + 0x00, op, 0); CB_OP_CASES_ARG(base + 0x08, op, 1);\
    CB_OP_CASES_ARG(base + 0x10, op, 2); CB_OP_CASES_ARG(base + 0x18, op, 3);\
    CB_OP_CASES_ARG(base + 0x20, op, 4); CB_OP_CASES_ARG(base + 0x28, op, 5);\
    CB_OP_CASES_ARG(base + 0x30, op, 6); CB_OP_CASES_ARG(base + 0x38, op, 7);


static void build_cb(u8 cb) {
    switch(cpu.cb) {
        CB_OP_CASES_NOARG(0x00, OP_RLC);
        CB_OP_CASES_NOARG(0x08, OP_RRC);
        CB_OP_CASES_NOARG(0x10, OP_RL);
        CB_OP_CASES_NOARG(0x18, OP_RR);
        CB_OP_CASES_NOARG(0x20, OP_SLA);
        CB_OP_CASES_NOARG(0x28, OP_SRA);
        CB_OP_CASES_NOARG(0x30, OP_SWAP);
        CB_OP_CASES_NOARG(0x38, OP_SRL);

        CB_BITMANIP_CASES(0x40, OP_BIT);
        CB_BITMANIP_CASES(0x80, OP_RES);
        CB_BITMANIP_CASES(0xC0, OP_SET);
    }
}

static void build_undefined() {
    build_noarg(OP_UNDEFINED);
}

op_t disasm(u16 addr)
{
    pc = addr;
    op = mem_read_byte(addr);

    switch(op)
    {
        case 0x00: build_noarg(OP_NOP); break;
        case 0x01: build_rd(OP_LD, REG_BC, DATA_D16); break;
        case 0x02: build_rmr(OP_LD, REG_BC, REG_A); break;
        case 0x03: build_r(OP_INC, REG_BC); break;
        case 0x04: build_r(OP_INC, REG_B); break;
        case 0x05: build_r(OP_DEC, REG_B); break;
        case 0x06: build_rd(OP_LD, REG_B, DATA_D8); break;
        case 0x07: build_noarg(OP_RLCA); break;
        case 0x08: build_d(OP_PUSH, DATA_A16); break;
        case 0x09: build_rr(OP_ADD, REG_HL, REG_BC); break;
        case 0x0A: build_rrm(OP_LD, REG_A, REG_BC); break;
        case 0x0B: build_r(OP_DEC, REG_BC); break;
        case 0x0C: build_r(OP_INC, REG_C); break;
        case 0x0D: build_r(OP_DEC, REG_C); break;
        case 0x0E: build_rd(OP_LD, REG_C, DATA_D8); break;
        case 0x0F: build_noarg(OP_RRCA); break;

        case 0x10: build_noarg(OP_STOP); break;
        case 0x11: build_rd(OP_LD, REG_DE, DATA_D16); break;
        case 0x12: build_rmr(OP_LD, REG_DE, REG_A); break;
        case 0x13: build_r(OP_INC, REG_DE); break;
        case 0x14: build_r(OP_INC, REG_D); break;
        case 0x15: build_r(OP_DEC, REG_D); break;
        case 0x16: build_rd(OP_LD, REG_D, DATA_D8); break;
        case 0x17: build_noarg(OP_RLA); break;
        case 0x18: build_d(OP_JR, DATA_R8); break;
        case 0x19: build_rr(OP_ADD, REG_HL, REG_DE); break;
        case 0x1A: build_rrm(OP_LD, REG_A, REG_DE); break;
        case 0x1B: build_r(OP_DEC, REG_DE); break;
        case 0x1C: build_r(OP_INC, REG_E); break;
        case 0x1D: build_r(OP_DEC, REG_E); break;
        case 0x1E: build_rd(OP_LD, REG_E, DATA_D8); break;
        case 0x1F: build_noarg(OP_RRA); break;

        case 0x20: build_fd(OP_JR, FLAG_NZ, DATA_R8); break;
        case 0x21: build_rd(OP_LD, REG_HL, DATA_D16); break;
        case 0x22: build_rmr(OP_LDINCDST, REG_HL, REG_A); break;
        case 0x23: build_r(OP_INC, REG_HL); break;
        case 0x24: build_r(OP_INC, REG_H); break;
        case 0x25: build_r(OP_DEC, REG_H); break;
        case 0x26: build_rd(OP_LD, REG_H, DATA_D8); break;
        case 0x27: build_noarg(OP_DAA); break;
        case 0x28: build_fd(OP_JR, FLAG_Z, DATA_R8); break;
        case 0x29: build_rr(OP_ADD, REG_HL, REG_HL); break;
        case 0x2A: build_rrm(OP_LDINCSRC, REG_A, REG_HL); break;
        case 0x2B: build_r(OP_DEC, REG_HL); break;
        case 0x2C: build_r(OP_INC, REG_L); break;
        case 0x2D: build_r(OP_DEC, REG_L); break;
        case 0x2E: build_rd(OP_LD, REG_L, DATA_D8); break;
        case 0x2F: build_noarg(OP_CPL); break;

        case 0x30: build_fd(OP_JR, FLAG_NC, DATA_R8); break;
        case 0x31: build_rd(OP_LD, REG_SP, DATA_D16); break;
        case 0x32: build_rmr(OP_LDDECDST, REG_HL, REG_A); break;
        case 0x33: build_r(OP_INC, REG_SP); break;
        case 0x34: build_rm(OP_INC, REG_HL); break;
        case 0x35: build_rm(OP_DEC, REG_HL); break;
        case 0x36: build_rmd(OP_LD, REG_HL, DATA_D8); break;
        case 0x37: build_noarg(OP_SCF); break;
        case 0x38: build_fd(OP_JR, FLAG_C, DATA_R8); break;
        case 0x39: build_rr(OP_ADD, REG_HL, REG_SP); break;
        case 0x3A: build_rrm(OP_LDDECSRC, REG_A, REG_HL); break;
        case 0x3B: build_r(OP_DEC, REG_SP); break;
        case 0x3C: build_r(OP_INC, REG_A); break;
        case 0x3D: build_r(OP_DEC, REG_A); break;
        case 0x3E: build_rd(OP_LD, REG_A, DATA_D8); break;
        case 0x3F: build_noarg(OP_CCF); break;

        case 0x40: build_rr(OP_LD, REG_B, REG_B); break;
        case 0x41: build_rr(OP_LD, REG_B, REG_C); break;
        case 0x42: build_rr(OP_LD, REG_B, REG_D); break;
        case 0x43: build_rr(OP_LD, REG_B, REG_E); break;
        case 0x44: build_rr(OP_LD, REG_B, REG_H); break;
        case 0x45: build_rr(OP_LD, REG_B, REG_L); break;
        case 0x46: build_rrm(OP_LD, REG_B, REG_HL); break;
        case 0x47: build_rr(OP_LD, REG_B, REG_A); break;
        case 0x48: build_rr(OP_LD, REG_C, REG_B); break;
        case 0x49: build_rr(OP_LD, REG_C, REG_C); break;
        case 0x4A: build_rr(OP_LD, REG_C, REG_D); break;
        case 0x4B: build_rr(OP_LD, REG_C, REG_E); break;
        case 0x4C: build_rr(OP_LD, REG_C, REG_H); break;
        case 0x4D: build_rr(OP_LD, REG_C, REG_L); break;
        case 0x4E: build_rrm(OP_LD, REG_C, REG_HL); break;
        case 0x4F: build_rr(OP_LD, REG_C, REG_A); break;

        case 0x50: build_rr(OP_LD, REG_D, REG_B); break;
        case 0x51: build_rr(OP_LD, REG_D, REG_C); break;
        case 0x52: build_rr(OP_LD, REG_D, REG_D); break;
        case 0x53: build_rr(OP_LD, REG_D, REG_E); break;
        case 0x54: build_rr(OP_LD, REG_D, REG_H); break;
        case 0x55: build_rr(OP_LD, REG_D, REG_L); break;
        case 0x56: build_rrm(OP_LD, REG_D, REG_HL); break;
        case 0x57: build_rr(OP_LD, REG_D, REG_A); break;
        case 0x58: build_rr(OP_LD, REG_E, REG_B); break;
        case 0x59: build_rr(OP_LD, REG_E, REG_C); break;
        case 0x5A: build_rr(OP_LD, REG_E, REG_D); break;
        case 0x5B: build_rr(OP_LD, REG_E, REG_E); break;
        case 0x5C: build_rr(OP_LD, REG_E, REG_H); break;
        case 0x5D: build_rr(OP_LD, REG_E, REG_L); break;
        case 0x5E: build_rrm(OP_LD, REG_E, REG_HL); break;
        case 0x5F: build_rr(OP_LD, REG_E, REG_A); break;

        case 0x60: build_rr(OP_LD, REG_H, REG_B); break;
        case 0x61: build_rr(OP_LD, REG_H, REG_C); break;
        case 0x62: build_rr(OP_LD, REG_H, REG_D); break;
        case 0x63: build_rr(OP_LD, REG_H, REG_E); break;
        case 0x64: build_rr(OP_LD, REG_H, REG_H); break;
        case 0x65: build_rr(OP_LD, REG_H, REG_L); break;
        case 0x66: build_rrm(OP_LD, REG_H, REG_HL); break;
        case 0x67: build_rr(OP_LD, REG_H, REG_A); break;
        case 0x68: build_rr(OP_LD, REG_L, REG_B); break;
        case 0x69: build_rr(OP_LD, REG_L, REG_C); break;
        case 0x6A: build_rr(OP_LD, REG_L, REG_D); break;
        case 0x6B: build_rr(OP_LD, REG_L, REG_E); break;
        case 0x6C: build_rr(OP_LD, REG_L, REG_H); break;
        case 0x6D: build_rr(OP_LD, REG_L, REG_L); break;
        case 0x6E: build_rrm(OP_LD, REG_L, REG_HL); break;
        case 0x6F: build_rr(OP_LD, REG_L, REG_A); break;

        case 0x70: build_rmr(OP_LD, REG_HL, REG_B); break;
        case 0x71: build_rmr(OP_LD, REG_HL, REG_C); break;
        case 0x72: build_rmr(OP_LD, REG_HL, REG_D); break;
        case 0x73: build_rmr(OP_LD, REG_HL, REG_E); break;
        case 0x74: build_rmr(OP_LD, REG_HL, REG_H); break;
        case 0x75: build_rmr(OP_LD, REG_HL, REG_L); break;
        case 0x76: build_noarg(OP_HALT); break;
        case 0x77: build_rmr(OP_LD, REG_HL, REG_A); break;
        case 0x78: build_rr(OP_LD, REG_A, REG_B); break;
        case 0x79: build_rr(OP_LD, REG_A, REG_C); break;
        case 0x7A: build_rr(OP_LD, REG_A, REG_D); break;
        case 0x7B: build_rr(OP_LD, REG_A, REG_E); break;
        case 0x7C: build_rr(OP_LD, REG_A, REG_H); break;
        case 0x7D: build_rr(OP_LD, REG_A, REG_L); break;
        case 0x7E: build_rrm(OP_LD, REG_A, REG_HL); break;
        case 0x7F: build_rr(OP_LD, REG_A, REG_A); break;

        case 0x80: build_rr(OP_ADD, REG_A, REG_B); break;
        case 0x81: build_rr(OP_ADD, REG_A, REG_C); break;
        case 0x82: build_rr(OP_ADD, REG_A, REG_D); break;
        case 0x83: build_rr(OP_ADD, REG_A, REG_E); break;
        case 0x84: build_rr(OP_ADD, REG_H, REG_H); break;
        case 0x85: build_rr(OP_ADD, REG_A, REG_L); break;
        case 0x86: build_rrm(OP_ADD, REG_A, REG_HL); break;
        case 0x87: build_rr(OP_ADD, REG_A, REG_A); break;
        case 0x88: build_rr(OP_ADC, REG_A, REG_B); break;
        case 0x89: build_rr(OP_ADC, REG_A, REG_C); break;
        case 0x8A: build_rr(OP_ADC, REG_A, REG_D); break;
        case 0x8B: build_rr(OP_ADC, REG_A, REG_E); break;
        case 0x8C: build_rr(OP_ADC, REG_A, REG_H); break;
        case 0x8D: build_rr(OP_ADC, REG_A, REG_L); break;
        case 0x8E: build_rrm(OP_ADC, REG_A, REG_HL); break;
        case 0x8F: build_rr(OP_ADC, REG_A, REG_A); break;

        case 0x90: build_rr(OP_SUB, REG_A, REG_B); break;
        case 0x91: build_rr(OP_SUB, REG_A, REG_C); break;
        case 0x92: build_rr(OP_SUB, REG_A, REG_D); break;
        case 0x93: build_rr(OP_SUB, REG_A, REG_E); break;
        case 0x94: build_rr(OP_SUB, REG_H, REG_H); break;
        case 0x95: build_rr(OP_SUB, REG_A, REG_L); break;
        case 0x96: build_rrm(OP_SUB, REG_A, REG_HL); break;
        case 0x97: build_rr(OP_SUB, REG_A, REG_A); break;
        case 0x98: build_rr(OP_SBC, REG_A, REG_B); break;
        case 0x99: build_rr(OP_SBC, REG_A, REG_C); break;
        case 0x9A: build_rr(OP_SBC, REG_A, REG_D); break;
        case 0x9B: build_rr(OP_SBC, REG_A, REG_E); break;
        case 0x9C: build_rr(OP_SBC, REG_A, REG_H); break;
        case 0x9D: build_rr(OP_SBC, REG_A, REG_L); break;
        case 0x9E: build_rrm(OP_SBC, REG_A, REG_HL); break;
        case 0x9F: build_rr(OP_SBC, REG_A, REG_A); break;

        case 0xA0: build_rr(OP_AND, REG_A, REG_B); break;
        case 0xA1: build_rr(OP_AND, REG_A, REG_C); break;
        case 0xA2: build_rr(OP_AND, REG_A, REG_D); break;
        case 0xA3: build_rr(OP_AND, REG_A, REG_E); break;
        case 0xA4: build_rr(OP_AND, REG_H, REG_H); break;
        case 0xA5: build_rr(OP_AND, REG_A, REG_L); break;
        case 0xA6: build_rrm(OP_AND, REG_A, REG_HL); break;
        case 0xA7: build_rr(OP_AND, REG_A, REG_A); break;
        case 0xA8: build_rr(OP_XOR, REG_A, REG_B); break;
        case 0xA9: build_rr(OP_XOR, REG_A, REG_C); break;
        case 0xAA: build_rr(OP_XOR, REG_A, REG_D); break;
        case 0xAB: build_rr(OP_XOR, REG_A, REG_E); break;
        case 0xAC: build_rr(OP_XOR, REG_A, REG_H); break;
        case 0xAD: build_rr(OP_XOR, REG_A, REG_L); break;
        case 0xAE: build_rrm(OP_XOR, REG_A, REG_HL); break;
        case 0xAF: build_rr(OP_XOR, REG_A, REG_A); break;

        case 0xB0: build_rr(OP_OR, REG_A, REG_B); break;
        case 0xB1: build_rr(OP_OR, REG_A, REG_C); break;
        case 0xB2: build_rr(OP_OR, REG_A, REG_D); break;
        case 0xB3: build_rr(OP_OR, REG_A, REG_E); break;
        case 0xB4: build_rr(OP_OR, REG_H, REG_H); break;
        case 0xB5: build_rr(OP_OR, REG_A, REG_L); break;
        case 0xB6: build_rrm(OP_OR, REG_A, REG_HL); break;
        case 0xB7: build_rr(OP_OR, REG_A, REG_A); break;
        case 0xB8: build_rr(OP_CP, REG_A, REG_B); break;
        case 0xB9: build_rr(OP_CP, REG_A, REG_C); break;
        case 0xBA: build_rr(OP_CP, REG_A, REG_D); break;
        case 0xBB: build_rr(OP_CP, REG_A, REG_E); break;
        case 0xBC: build_rr(OP_CP, REG_A, REG_H); break;
        case 0xBD: build_rr(OP_CP, REG_A, REG_L); break;
        case 0xBE: build_rrm(OP_CP, REG_A, REG_HL); break;
        case 0xBF: build_rr(OP_CP, REG_A, REG_A); break;

        case 0xC0: build_f(OP_RET, FLAG_NZ); break;
        case 0xC1: build_r(OP_POP, REG_BC); break;
        case 0xC2: build_fd(OP_JP, FLAG_NZ, DATA_A16); break;
        case 0xC3: build_d(OP_JP, DATA_A16); break;
        case 0xC4: build_fd(OP_CALL, FLAG_NZ, DATA_A16); break;
        case 0xC5: build_r(OP_PUSH, REG_BC); break;
        case 0xC6: build_rd(OP_ADD, REG_A, DATA_D8); break;
        case 0xC7: build_n(OP_RST, 0x00); break;
        case 0xC8: build_f(OP_RET, FLAG_Z); break;
        case 0xC9: build_noarg(OP_RET); break;
        case 0xCA: build_fd(OP_JP, FLAG_Z, DATA_A16); break;
        case 0xCB: build_cb(mem_read_byte(addr+1)); break;
        case 0xCC: build_fd(OP_CALL, FLAG_Z, DATA_A16); break;
        case 0xCD: build_d(OP_CALL, DATA_A16); break;
        case 0xCE: build_rd(OP_ADC, REG_A, DATA_A8); break;
        case 0xCF: build_n(OP_RST, 0x08); break;

        case 0xD0: build_f(OP_RET, FLAG_NC); break;
        case 0xD1: build_r(OP_POP, REG_DE); break;
        case 0xD2: build_fd(OP_JP, FLAG_NC, DATA_A16); break;
        case 0xD3: build_undefined(); break;
        case 0xD4: build_fd(OP_CALL, FLAG_NC, DATA_A16); break;
        case 0xD5: build_r(OP_PUSH, REG_DE); break;
        case 0xD6: build_rd(OP_SUB, REG_A, DATA_D8); break;
        case 0xD7: build_n(OP_RST, 0x10); break;
        case 0xD8: build_f(OP_RET, FLAG_C); break;
        case 0xD9: build_noarg(OP_RETI); break;
        case 0xDA: build_fd(OP_JP, FLAG_C, DATA_A16); break;
        case 0xDB: build_undefined(); break;
        case 0xDC: build_fd(OP_CALL, FLAG_C, DATA_A16); break;
        case 0xDD: build_undefined(OP_CALL, DATA_A16); break;
        case 0xDE: build_rd(OP_SBC, REG_A, DATA_A8); break;
        case 0xDF: build_n(OP_RST, 0x18); break;

        case 0xE0: build_dmr(OP_LDH, DATA_A8, REG_A); break;
        case 0xE1: build_r(OP_POP, REG_HL); break;
        case 0xE2: build_rmr(OP_LD, REG_C, REG_A); break;
        case 0xE3: build_undefined(); break;
        case 0xE4: build_undefined(); break;
        case 0xE5: build_r(OP_PUSH, REG_HL); break;
        case 0xE6: build_rd(OP_AND, REG_A, DATA_D8); break;
        case 0xE7: build_n(OP_RST, 0x20); break;
        case 0xE8: build_rd(OP_ADD, REG_SP, DATA_R8); break;
        case 0xE9: build_rm(OP_JP, REG_HL); break;
        case 0xEA: build_dmr(OP_LD, DATA_A16, REG_A); break;
        case 0xEB: build_undefined(); break;
        case 0xEC: build_undefined(); break;
        case 0xED: build_undefined(); break;
        case 0xEE: build_rd(OP_XOR, REG_A, DATA_D8); break;
        case 0xEF: build_n(OP_RST, 0x28); break;

        case 0xF0: build_rdm(OP_LDH, REG_A, DATA_A8); break;
        case 0xF1: build_r(OP_POP, REG_AF); break;
        case 0xF2: build_rrm(OP_LD, REG_A, REG_C); break;
        case 0xF3: build_noarg(OP_DI); break;
        case 0xF4: build_undefined(); break;
        case 0xF5: build_r(OP_PUSH, REG_AF); break;
        case 0xF6: build_rd(OP_OR, REG_A, DATA_D8); break;
        case 0xF7: build_n(OP_RST, 0x30); break;
        case 0xF8: build_rrd(OP_LD, REG_HL, REG_SP, DATA_R8); break;
        case 0xF9: build_rr(OP_LD, REG_SP, REG_HL); break;
        case 0xFA: build_rdm(OP_LD, REG_A, DATA_A16); break;
        case 0xFB: build_noarg(OP_EI); break;
        case 0xFC: build_undefined(); break;
        case 0xFD: build_undefined(); break;
        case 0xFE: build_rd(OP_CP, REG_A, DATA_D8); break;
        case 0xFF: build_n(OP_RST, 0x38); break;

        default:
            assert(0);
    }

    return c_op;
}

const char* disasm_str(op_t op) {
    str_op = op;

    switch(op.sig) {
        case OP_SIG_NOARG: str_noarg(); break;
        case OP_SIG_FD: str_fd(); break;
        case OP_SIG_R: str_r(); break;
        case OP_SIG_RM: str_rm(); break;
        case OP_SIG_RR: str_rr(); break;
        case OP_SIG_RRM: str_rrm(); break;
        case OP_SIG_RMD: str_rmd(); break;
        case OP_SIG_RD: str_rd(); break;
        case OP_SIG_D: str_d(); break;
        case OP_SIG_RMR: str_rmr(); break;
        case OP_SIG_N: str_n(); break;
        case OP_SIG_DMR: str_dmr(); break;
        case OP_SIG_RDM: str_rdm(); break;
        case OP_SIG_RRD: str_rrd(); break;
        case OP_SIG_RN: str_rn(); break;
        case OP_SIG_RMN: str_rmn(); break;
        case OP_SIG_F: str_f(); break;
    }

    return out_disasm_str;
}

