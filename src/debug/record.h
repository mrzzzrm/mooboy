#ifndef DEBUG_RECORD_H
#define DEBUG_RECORD_H

#include "core/defines.h"

#include "disasm.h"

typedef struct {
    u16 begin;
    u16 end;
} block_t;

void record_cpu_cycle();
block_t * record_get_current_block();
int op_length(op_t op);

#endif // DEBUG_RECORD_H
