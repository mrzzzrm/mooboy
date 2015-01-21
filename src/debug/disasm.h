#ifndef DEBUG_DISASM_H
#define DEBUG_DISASM_H

#include "core/defines.h"

typedef enum {
    OP_UNDEFINED,
    OP_NOP,
    OP_STOP,
    OP_JR,
    OP_LD,
    OP_LDINCSRC,
    OP_LDDECSRC,
    OP_LDINCDST,
    OP_LDDECDST,
    OP_PUSH,
    OP_POP,
    OP_INC,
    OP_DEC,
    OP_ADD,
    OP_SUB,
    OP_RLCA,
    OP_RRCA,
    OP_RLA,
    OP_RRA,
    OP_DAA,
    OP_CPL,
    OP_SCF,
    OP_CCF,
    OP_HALT,
    OP_ADC,
    OP_SBC,
    OP_JP,
    OP_CALL,
    OP_RST,
    OP_AND,
    OP_XOR,
    OP_OR,
    OP_CP,
    OP_RET,
    OP_RETI,
    OP_LDH,
    OP_DI,
    OP_EI,
    OP_RLC,
    OP_RRC,
    OP_RL,
    OP_RR,
    OP_SLA,
    OP_SRA,
    OP_SWAP,
    OP_SRL,
    OP_BIT,
    OP_RES,
    OP_SET,

    _OP_COUNT_
} OP;

typedef enum {
    FLAG_Z,
    FLAG_NZ,
    FLAG_C,
    FLAG_NC,

    _FLAG_COUNT_
} FLAG;

typedef enum {
    DATA_A8,
    DATA_A16,
    DATA_R8,
    DATA_D8,
    DATA_D16,

    _DATA_COUNT_
} DATA;

typedef enum {
    REG_A,
    REG_B,
    REG_C,
    REG_D,
    REG_E,
    REG_H,
    REG_L,
    REG_F,

    REG_SP,
    REG_PC,

    REG_AF,
    REG_BC,
    REG_DE,
    REG_HL,

    _REG_COUNT_
} REG;

typedef enum {
    OP_SIG_NOARG,
    OP_SIG_FD,
    OP_SIG_R,
    OP_SIG_RM,
    OP_SIG_RR,
    OP_SIG_RRM,
    OP_SIG_RMD,
    OP_SIG_RD,
    OP_SIG_D,
    OP_SIG_RMR,
    OP_SIG_N,
    OP_SIG_DMR,
    OP_SIG_RDM,
    OP_SIG_RRD,
    OP_SIG_RN,
    OP_SIG_RMN,
    OP_SIG_F
} OP_SIG;

typedef struct {
    OP op;
    OP_SIG sig;
    FLAG f;
    REG r1;
    REG r2;
    DATA d;
    u8 n;
} op_t;

op_t disasm(u16 addr);

const char* disasm_str(op_t op);

#endif // DEBUG_DISASM_H
