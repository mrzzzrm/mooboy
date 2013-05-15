#ifndef CPU_CHUNKS_H
#define CPU_CHUNKS_H

    #include "util/defines.h"

    typedef union op_arg_u {
        u8 d;
        u8 *b;
        u16 *w;
    } op_arg;

    typedef struct op_chunk_s op_chunk_t; // Forward decl
    struct op_chunk_s {
        u8 op;
        u8 i, b; // Only used for prefix CB; immediate and bit index
        u8 sp;
        void (*funcs[5])(op_chunk_t *c);
        op_arg opl, opr; // Pointers to 8/16 Bit memory locations
        u8 mcs;
    };

    extern op_chunk_t *op_chunk_map[0x100];
    extern op_chunk_t *op_cb_chunk_map[0x100];

    void op_create_chunks();

#endif
