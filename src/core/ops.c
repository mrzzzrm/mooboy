#include "ops.h"
#include <assert.h>
#include <stdio.h>
#include "cpu.h"
#include "moo.h"
#include "hw.h"
#include "mem.h"
#include "defines.h"

static u8 mcs[256] = {
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
    1, 3, 2, 2, 1, 1, 2, 1, 5, 2, 2, 2, 1, 1, 2, 1, // 0
    1, 3, 2, 2, 1, 1, 2, 1, 3, 2, 2, 2, 1, 1, 2, 1, // 1
    0, 3, 2, 2, 1, 1, 2, 1, 0, 2, 2, 2, 1, 1, 2, 1, // 2
    0, 3, 2, 2, 2, 2, 2, 1, 0, 2, 2, 2, 1, 1, 2, 1, // 3
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 4
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 5
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 6
    2, 2, 2, 2, 2, 2, 1, 2, 1, 1, 1, 1, 1, 1, 2, 1, // 7
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 8
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // 9
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // A
    1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, // B
    0, 3, 0, 4, 0, 4, 2, 4, 0, 4, 0, 0, 0, 0, 2, 4, // C
    0, 3, 0, 1, 0, 4, 2, 4, 0, 4, 0, 1, 0, 1, 2, 4, // D
    2, 3, 2, 1, 1, 4, 2, 4, 4, 1, 2, 1, 1, 1, 2, 4, // E
    2, 3, 2, 1, 1, 4, 2, 4, 3, 2, 2, 1, 1, 1, 2, 4  // F
};

static u8 preread_mcs[256] = {
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 2
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 3
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 4
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 5
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 6
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 7
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // A
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // B
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, // C
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // D
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // E
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0  // F
};

static u8 prewrite_mcs[256] = {
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 0
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 1
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 2
    0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 3
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 4
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 5
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 6
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 7
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 8
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 9
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // A
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // B
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, // C
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // D
    1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, // E
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0  // F
};

static u8 cb_preread_mcs[256] = {
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 0
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 1
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 2
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 3
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 4
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 5
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 6
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 7
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 8
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 9
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // A
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // B
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // C
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // D
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // E
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0  // F
};

static u8 cb_prewrite_mcs[256] = {
//  0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 0
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 1
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 2
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 3
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 4
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 5
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 6
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 7
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 8
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // 9
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // A
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // B
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // C
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // D
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, // E
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0  // F
};

static u16 pop() {
    u16 r = mem_read_word(SP);
    SP += 2;
    return r;
}

static void push(u16 w) {
    SP -= 2;
    mem_write_word(SP, w);
}

static inline u8 fetch_byte() {
    return mem_read_byte(PC++);
}

static inline u16 fetch_word() {
    u16 word = mem_read_word(PC);
    PC += 2;
    return word;
}

static inline void write_byte(u16 adr, u8 val) {
    hw_step(prewrite_mcs[cpu.op]);
    mem_write_byte(adr, val);
}

static inline void cb_write_byte(u16 adr, u8 val) {
    hw_step(cb_prewrite_mcs[cpu.cb]);
    mem_write_byte(adr, val);
}

static inline void write_word(u16 adr, u16 val) {
    hw_step(prewrite_mcs[cpu.op]);
    mem_write_word(adr, val);
}

static inline u8 read_byte(u16 adr) {
    hw_step(preread_mcs[cpu.op]);
    return mem_read_byte(adr);
}

static inline u8 cb_read_byte(u16 adr) {
    hw_step(cb_preread_mcs[cpu.cb]);
    return mem_read_byte(adr);
}

static inline u8 rlc(u8 byte) {
    byte = (byte<<1) | (byte>>7);
    F = FZZ(byte) | FCB0(byte);
    return byte;
}

static inline u8 rrc(u8 byte) {
    byte = (byte>>1) | (byte<<7);
    F = FZZ(byte) | FCB7(byte);
    return byte;
}

static inline void rlca() {
    A = rlc(A);
    F &= ~FZBIT;
}

static inline void rrca() {
    A = rrc(A);
    F &= ~FZBIT;
}

static inline u8 rl(u8 byte) {
    u8 fc = FCB7(byte);
    byte = (byte<<1) | (FC>>4);
    F = FZZ(byte) | fc;
    return byte;
}

static inline u8 rr(u8 byte) {
    u8 fc = FCB0(byte);
    byte = (byte>>1) | (FC<<3);
    F = FZZ(byte) | fc;
    return byte;
}

static inline void rla() {
    A = rl(A);
    F &= ~FZBIT;
}

static inline void rra() {
    A = rr(A);
    F &= ~FZBIT;
}

static inline u8 sla(u8 byte) {
    u8 fc = FCB7(byte);
    byte <<= 1;
    F = FZZ(byte) | fc;
    return byte;
}

static inline u8 sra(u8 byte) {
    u8 fc = FCB0(byte);
    byte = (byte & 0x80) | (byte >> 1);
    F = FZZ(byte) | fc;
    return byte;
}

static inline u8 swap(u8 byte) {
    byte = ((byte & 0x0F) << 4) | (byte >> 4);
    F = FZZ(byte);
    return byte;
}

static inline u8 srl(u8 byte) {
    u8 fc = FCB0(byte);
    byte >>= 1;
    F = FZZ(byte) | fc;
    return byte;
}

static inline u8 bit(u8 b, u8 byte) {
    F = FZZ(byte & (1<<b)) | FHBIT | FC;
    return byte;
}

static inline u8 res(u8 b, u8 byte) {
    return byte & ~(1<<b);
}

static inline u8 set(u8 b, u8 byte) {
    return byte | (1<<b);
}

static inline void daa() {
    int a = A;

    if (!FN) {
        if (FH || (a & 0xF) > 9)
            a += 0x06;
        if (FC || a > 0x9F)
            a += 0x60;
    }
    else {
        if (FH)
            a = (a - 6) & 0xFF;
        if (FC)
            a -= 0x60;
    }

    F &= ~(FHBIT | FZBIT);
    if ((a & 0x100) == 0x100)
        F |= FCBIT;
    a &= 0xFF;
    if (a == 0)
        F |= FZBIT;
    A = a;
}

static inline void scf() {
    F = FZ | FCBIT;
}

static inline void ccf() {
    F = FZ | (FC ? 0 : FCBIT);
}

static inline void cpl() {
    A ^= 0xFF;
    F = FZ | FNBIT | FHBIT | FC;
}

static inline int jr(int cond, u8 val) {
    if(cond) {
        PC += (s8)val;
        return 3;
    }
    else {
        return 2;
    }
}

static inline int jp(int cond, u16 adr) {
    if(cond) {
        PC = adr;
        return 4;
    }
    else {
        return 3;
    }
}

static inline void rst(u8 val) {
    push(PC);
    PC = val;
}

static inline int ret(int cond) {
    if(cond) {
        PC = pop();
        return 5;
    }
    else {
        return 2;
    }
}

static inline int call(int cond, u16 adr) {
    if(cond) {
        push(PC);
        PC = adr;
        return 6;
    }
    else {
        return 3;
    }
}

static inline void stop() {
    if(cpu.freq_switch) {
        if(cpu.freq == NORMAL_CPU_FREQ) {
            cpu.freq = DOUBLE_CPU_FREQ;
            cpu.freq_factor = 2;
        }
        else {
            cpu.freq = NORMAL_CPU_FREQ;
            cpu.freq_factor = 1;
        }

        cpu.freq_switch = 0x00;
    }
    else {

    }
}

static inline void halt() {
    if(cpu.ime == IME_OFF) {
        cpu.halted = 1;
    }
    else {
        cpu.halted = 1;
    }
}

static inline u8 inc_byte(u8 byte) {
    u16 r = (u16)byte + 1;
    F = FZZ((u8)r) |
        (FHBIT & ((byte ^ r) << 1)) |
        FC;
    return r;
}

static inline u8 dec_byte(u8 byte) {
    u8 r = byte - 1;
    F = FZZ((u8)r) |
        FNBIT |
        (FHBIT & ((byte ^ r) << 1)) |
        FC;
    return r;
}

static inline void inc_mem(u16 adr) {
    u8 val = read_byte(adr);
    val = inc_byte(val);
    write_byte(adr, val);
}

static inline void dec_mem(u16 adr) {
    u8 val = read_byte(adr);
    val = dec_byte(val);
    write_byte(adr, val);
}

static inline void add(u8 byte) {
    u16 r = (u16)A + (u16)byte;
    F = FZZ((u8)r) |
        (FHBIT & ((A ^ byte ^ r) << 1)) |
        (FCBIT & (r >> 4));
    A = (u8)r;
}

static inline void adc(u8 byte) {
    u8 fc = FC ? 1 : 0;
    u16 r = (u16)A + (u16)byte + (u16)fc;
    F = FZZ((u8)r) |
        (FHBIT & ((A ^ byte ^ fc ^ r) << 1)) |
        (FCBIT & (r >> 4));
    A = (u8)r;
}

static inline void sub(u8 byte) {
    u16 r = (u16)A - (u16)byte;
    F = FZZ((u8)r) |
        FNBIT |
        (FHBIT & ((A ^ byte ^ r) << 1)) |
        (FCBIT & (r >> 4));
    A = (u8)r;
}

static inline void sbc(u8 byte) {
    u8 fc = FC ? 1 : 0;
    u16 r = (u16)A - (u16)byte - (u16)fc;
    F = FZZ((u8)r) |
        FNBIT |
        (FHBIT & ((A ^ byte ^ fc ^ r) << 1)) |
        (FCBIT & (r >> 4));
    A = (u8)r;
}

static inline void and(u8 byte) {
    A &= byte;
    F = FZZ(A) | FHBIT;
}

static inline void xor(u8 byte) {
    A ^= byte;
    F = FZZ(A);
}

static inline void or(u8 byte) {
    A |= byte;
    F = FZZ(A);
}

static inline void cp(u8 byte) {
    u16 r = (u16)A - (u16)byte;
    F = FZZ((u8)r) | FNBIT | (FHBIT & ((A ^ byte ^ r) << 1)) | (FCBIT & (r >> 4));
}

static inline u16 add_word(u16 a, u16 b) {
    u32 r = (u32)a + (u32)b;
    F = FZ |
        (FHBIT & ((a ^ b ^ r) >> 7)) |
        (FCBIT & (r >> 12));
    return (u16)r;
}

static inline void add_sp(u8 byte) {
    s8 o = (s8)byte;
    u32 r = (u32)SP + o;

    F = 0x00;
    if ((r & 0xFF) < (SP & 0xFF)) {
        F |= FCBIT;
    }
    if ((r & 0xF) < (SP & 0xF)) {
      F |= FHBIT;
    }

    SP = r;
}

static inline void ld_hl_spi() {
    s8 o = (s8)fetch_byte();
    u32 r = (u32)SP + o;

    F = 0;
    if ((r & 0xFF) < (SP & 0xFF))
      F |= FCBIT;
    if ((r & 0xF) < (SP & 0xF))
      F |= FHBIT;

    HL = (u16)r;
}

static inline void reti() {
    PC = pop();
    cpu.ime = IME_ON;
}

#define CB_OP_CASES_NOARG(base, func) \
    case base + 0: B = func(B); break; \
    case base + 1: C = func(C); break; \
    case base + 2: D = func(D); break; \
    case base + 3: E = func(E); break; \
    case base + 4: H = func(H); break; \
    case base + 5: L = func(L); break; \
    case base + 6: cb_write_byte(HL, func(cb_read_byte(HL))); break; \
    case base + 7: A = func(A); break;

#define CB_OP_CASES_ARG(base, func, arg, writeback) \
    case base + 0: B = func(arg, B); break; \
    case base + 1: C = func(arg, C); break; \
    case base + 2: D = func(arg, D); break; \
    case base + 3: E = func(arg, E); break; \
    case base + 4: H = func(arg, H); break; \
    case base + 5: L = func(arg, L); break; \
    case base + 6: \
        if(writeback) \
            cb_write_byte(HL, func(arg, cb_read_byte(HL))); \
        else \
            func(arg, cb_read_byte(HL)); \
    break; \
    case base + 7: A = func(arg, A); break;

#define CB_BITMANIP_CASES(base, func, writeback) \
    CB_OP_CASES_ARG(base + 0x00, func, 0, writeback); CB_OP_CASES_ARG(base + 0x08, func, 1, writeback);\
    CB_OP_CASES_ARG(base + 0x10, func, 2, writeback); CB_OP_CASES_ARG(base + 0x18, func, 3, writeback);\
    CB_OP_CASES_ARG(base + 0x20, func, 4, writeback); CB_OP_CASES_ARG(base + 0x28, func, 5, writeback);\
    CB_OP_CASES_ARG(base + 0x30, func, 6, writeback); CB_OP_CASES_ARG(base + 0x38, func, 7, writeback);


static inline int cb() {
    switch(cpu.cb) {
        CB_OP_CASES_NOARG(0x00, rlc);
        CB_OP_CASES_NOARG(0x08, rrc);
        CB_OP_CASES_NOARG(0x10, rl);
        CB_OP_CASES_NOARG(0x18, rr);
        CB_OP_CASES_NOARG(0x20, sla);
        CB_OP_CASES_NOARG(0x28, sra);
        CB_OP_CASES_NOARG(0x30, swap);
        CB_OP_CASES_NOARG(0x38, srl);

        CB_BITMANIP_CASES(0x40, bit, 0);
        CB_BITMANIP_CASES(0x80, res, 1);
        CB_BITMANIP_CASES(0xC0, set, 1);
    }

    return 2;
}

int op_exec() {

    switch(cpu.op) {
        case 0x00: break;
        case 0x01: BC = fetch_word(); break;
        case 0x02: write_byte(BC, A); break;
        case 0x03: BC++; break;
        case 0x04: B = inc_byte(B); break;
        case 0x05: B = dec_byte(B); break;
        case 0x06: B = fetch_byte(); break;
        case 0x07: rlca(); break;
        case 0x08: write_word(fetch_word(), SP); break;
        case 0x09: HL = add_word(HL, BC); break;
        case 0x0A: A = read_byte(BC); break;
        case 0x0B: BC--; break;
        case 0x0C: C = inc_byte(C); break;
        case 0x0D: C = dec_byte(C); break;
        case 0x0E: C = fetch_byte(); break;
        case 0x0F: rrca(); break;

        case 0x10: stop(); break;
        case 0x11: DE = fetch_word(); break;
        case 0x12: write_byte(DE, A); break;
        case 0x13: DE++; break;
        case 0x14: D = inc_byte(D); break;
        case 0x15: D = dec_byte(D); break;
        case 0x16: D = fetch_byte(); break;
        case 0x17: rla(); break;
        case 0x18: return jr(1, fetch_byte());
        case 0x19: HL = add_word(HL, DE); break;
        case 0x1A: A = read_byte(DE); break;
        case 0x1B: DE--; break;
        case 0x1C: E = inc_byte(E); break;
        case 0x1D: E = dec_byte(E); break;
        case 0x1E: E = fetch_byte(); break;
        case 0x1F: rra(); break;

        case 0x20: return jr(!FZ, fetch_byte());
        case 0x21: HL = fetch_word(); break;
        case 0x22: write_byte(HL++, A); break;
        case 0x23: HL++; break;
        case 0x24: H = inc_byte(H); break;
        case 0x25: H = dec_byte(H); break;
        case 0x26: H = fetch_byte(); break;
        case 0x27: daa(); break;
        case 0x28: return jr(FZ, fetch_byte());
        case 0x29: HL = add_word(HL, HL); break;
        case 0x2A: A = read_byte(HL++); break;
        case 0x2B: HL--; break;
        case 0x2C: L = inc_byte(L); break;
        case 0x2D: L = dec_byte(L); break;
        case 0x2E: L = fetch_byte(); break;
        case 0x2F: cpl(); break;

        case 0x30: return jr(!FC, fetch_byte());
        case 0x31: SP = fetch_word(); break;
        case 0x32: write_byte(HL--, A); break;
        case 0x33: SP++; break;
        case 0x34: inc_mem(HL); break;
        case 0x35: dec_mem(HL); break;
        case 0x36: write_byte(HL, fetch_byte()); break;
        case 0x37: scf(); break;
        case 0x38: return jr(FC, fetch_byte());
        case 0x39: HL = add_word(HL, SP); break;
        case 0x3A: A = read_byte(HL--); break;
        case 0x3B: SP--; break;
        case 0x3C: A = inc_byte(A); break;
        case 0x3D: A = dec_byte(A); break;
        case 0x3E: A = fetch_byte(); break;
        case 0x3F: ccf(); break;

        case 0x40: B = B; break;
        case 0x41: B = C; break;
        case 0x42: B = D; break;
        case 0x43: B = E; break;
        case 0x44: B = H; break;
        case 0x45: B = L; break;
        case 0x46: B = read_byte(HL); break;
        case 0x47: B = A; break;
        case 0x48: C = B; break;
        case 0x49: C = C; break;
        case 0x4A: C = D; break;
        case 0x4B: C = E; break;
        case 0x4C: C = H; break;
        case 0x4D: C = L; break;
        case 0x4E: C = read_byte(HL); break;
        case 0x4F: C = A; break;

        case 0x50: D = B; break;
        case 0x51: D = C; break;
        case 0x52: D = D; break;
        case 0x53: D = E; break;
        case 0x54: D = H; break;
        case 0x55: D = L; break;
        case 0x56: D = read_byte(HL); break;
        case 0x57: D = A; break;
        case 0x58: E = B; break;
        case 0x59: E = C; break;
        case 0x5A: E = D; break;
        case 0x5B: E = E; break;
        case 0x5C: E = H; break;
        case 0x5D: E = L; break;
        case 0x5E: E = read_byte(HL); break;
        case 0x5F: E = A; break;

        case 0x60: H = B; break;
        case 0x61: H = C; break;
        case 0x62: H = D; break;
        case 0x63: H = E; break;
        case 0x64: H = H; break;
        case 0x65: H = L; break;
        case 0x66: H = read_byte(HL); break;
        case 0x67: H = A; break;
        case 0x68: L = B; break;
        case 0x69: L = C; break;
        case 0x6A: L = D; break;
        case 0x6B: L = E; break;
        case 0x6C: L = H; break;
        case 0x6D: L = L; break;
        case 0x6E: L = read_byte(HL); break;
        case 0x6F: L = A; break;

        case 0x70: write_byte(HL, B); break;
        case 0x71: write_byte(HL, C); break;
        case 0x72: write_byte(HL, D); break;
        case 0x73: write_byte(HL, E); break;
        case 0x74: write_byte(HL, H); break;
        case 0x75: write_byte(HL, L); break;
        case 0x76: halt(); break;
        case 0x77: write_byte(HL, A); break;
        case 0x78: A = B; break;
        case 0x79: A = C; break;
        case 0x7A: A = D; break;
        case 0x7B: A = E; break;
        case 0x7C: A = H; break;
        case 0x7D: A = L; break;
        case 0x7E: A = read_byte(HL); break;
        case 0x7F: A = A; break;

        case 0x80: add(B); break;
        case 0x81: add(C); break;
        case 0x82: add(D); break;
        case 0x83: add(E); break;
        case 0x84: add(H); break;
        case 0x85: add(L); break;
        case 0x86: add(read_byte(HL)); break;
        case 0x87: add(A); break;
        case 0x88: adc(B); break;
        case 0x89: adc(C); break;
        case 0x8A: adc(D); break;
        case 0x8B: adc(E); break;
        case 0x8C: adc(H); break;
        case 0x8D: adc(L); break;
        case 0x8E: adc(read_byte(HL)); break;
        case 0x8F: adc(A); break;

        case 0x90: sub(B); break;
        case 0x91: sub(C); break;
        case 0x92: sub(D); break;
        case 0x93: sub(E); break;
        case 0x94: sub(H); break;
        case 0x95: sub(L); break;
        case 0x96: sub(read_byte(HL)); break;
        case 0x97: sub(A); break;
        case 0x98: sbc(B); break;
        case 0x99: sbc(C); break;
        case 0x9A: sbc(D); break;
        case 0x9B: sbc(E); break;
        case 0x9C: sbc(H); break;
        case 0x9D: sbc(L); break;
        case 0x9E: sbc(read_byte(HL)); break;
        case 0x9F: sbc(A); break;

        case 0xA0: and(B); break;
        case 0xA1: and(C); break;
        case 0xA2: and(D); break;
        case 0xA3: and(E); break;
        case 0xA4: and(H); break;
        case 0xA5: and(L); break;
        case 0xA6: and(read_byte(HL)); break;
        case 0xA7: and(A); break;
        case 0xA8: xor(B); break;
        case 0xA9: xor(C); break;
        case 0xAA: xor(D); break;
        case 0xAB: xor(E); break;
        case 0xAC: xor(H); break;
        case 0xAD: xor(L); break;
        case 0xAE: xor(read_byte(HL)); break;
        case 0xAF: xor(A); break;

        case 0xB0: or(B); break;
        case 0xB1: or(C); break;
        case 0xB2: or(D); break;
        case 0xB3: or(E); break;
        case 0xB4: or(H); break;
        case 0xB5: or(L); break;
        case 0xB6: or(read_byte(HL)); break;
        case 0xB7: or(A); break;
        case 0xB8: cp(B); break;
        case 0xB9: cp(C); break;
        case 0xBA: cp(D); break;
        case 0xBB: cp(E); break;
        case 0xBC: cp(H); break;
        case 0xBD: cp(L); break;
        case 0xBE: cp(read_byte(HL)); break;
        case 0xBF: cp(A); break;

        case 0xC0: return ret(!FZ);
        case 0xC1: BC = pop(); break;
        case 0xC2: return jp(!FZ, fetch_word());
        case 0xC3: return jp(1, fetch_word());
        case 0xC4: return call(!FZ, fetch_word());
        case 0xC5: push(BC); break;
        case 0xC6: add(fetch_byte()); break;
        case 0xC7: rst(0x00); break;
        case 0xC8: return ret(FZ);
        case 0xC9: PC = pop(); break;
        case 0xCA: return jp(FZ, fetch_word());
        case 0xCB: cpu.cb = fetch_byte(); return cb();
        case 0xCC: return call(FZ, fetch_word());
        case 0xCD: return call(1, fetch_word());
        case 0xCE: adc(fetch_byte()); break;
        case 0xCF: rst(0x08); break;

        case 0xD0: return ret(!FC);
        case 0xD1: DE = pop(); break;
        case 0xD2: return jp(!FC, fetch_word());
        case 0xD3: break;
        case 0xD4: return call(!FC, fetch_word());
        case 0xD5: push(DE); break;
        case 0xD6: sub(fetch_byte()); break;
        case 0xD7: rst(0x10); break;
        case 0xD8: return ret(FC);
        case 0xD9: reti(); break;
        case 0xDA: return jp(FC, fetch_word());
        case 0xDB: break;
        case 0xDC: return call(FC, fetch_word());
        case 0xDD: break;
        case 0xDE: sbc(fetch_byte()); break;
        case 0xDF: rst(0x18); break;

        case 0xE0: write_byte(0xFF00 + fetch_byte(), A); break;
        case 0xE1: HL = pop(); break;
        case 0xE2: write_byte(0xFF00 + C, A); break;
        case 0xE3: break;
        case 0xE4: break;
        case 0xE5: push(HL); break;
        case 0xE6: and(fetch_byte()); break;
        case 0xE7: rst(0x20); break;
        case 0xE8: add_sp(fetch_byte()); break;
        case 0xE9: PC = HL; break;
        case 0xEA: write_byte(fetch_word(), A); break;
        case 0xEB: break;
        case 0xEC: break;
        case 0xED: break;
        case 0xEE: xor(fetch_byte()); break;
        case 0xEF: rst(0x28); break;

        case 0xF0: A = read_byte(0xFF00 + fetch_byte()); break;
        case 0xF1: AF = pop() & 0xFFF0; break;
        case 0xF2: A = read_byte(0xFF00 + C); break;
        case 0xF3: cpu.ime = cpu.ime == IME_ON ? IME_DOWN : cpu.ime; break;
        case 0xF4: break;
        case 0xF5: push(AF); break;
        case 0xF6: or(fetch_byte()); break;
        case 0xF7: rst(0x30); break;
        case 0xF8: ld_hl_spi(); break;
        case 0xF9: SP = HL; break;
        case 0xFA: A = read_byte(fetch_word()); break;
        case 0xFB: cpu.ime = cpu.ime == IME_OFF ? IME_UP : cpu.ime; break;
        case 0xFC: break;
        case 0xFD: break;
        case 0xFE: cp(fetch_byte()); break;
        case 0xFF: rst(0x38); break;

        default:;
#ifdef DEBUG
            printf("op %.2X not implemented\n", cpu.op);
#endif
    }

    return mcs[cpu.op];
}

