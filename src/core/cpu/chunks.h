#ifndef CPU_CHUNKS_H
#define CPU_CHUNKS_H

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
        u8 sp;
        void (*funcs[5])(op_chunk *c);
        op_arg opl, opr; // Pointers to 8/16 Bit registers
    };

    extern op_chunk *op_chunk_map[0xFF];
    extern op_chunk *op_cb_chunk_map[0xFF];

    op_chunk *op_create_chunk(u8 op);

#endif
