#include "record.h"

#include <assert.h>
#include <stdlib.h>

#include "core/cpu.h"

#include "disasm.h"

typedef struct {
    u16 pc;
} record_t;


#define RECORDS_BUFFER_SIZE 4096
//#define LOOP_STACK_SIZE 128
#define BLOCK_BUFFER_SIZE 2560

static int records_cursor = 0;
static int records_count = 0;
static record_t records[RECORDS_BUFFER_SIZE];

//static int loop_top = 0;
//static loop_t loops[LOOP_STACK_SIZE];
//
//static int in_loop = 0;

static int block_cursor = 0;
static block_t blocks[BLOCK_BUFFER_SIZE];
static block_t* current_block = NULL;
static u16 last_recorded_pc = 0;

static record_t* get_record(int offset) {
    int index = records_cursor + offset - 1;

    while (index < 0) {
       index += RECORDS_BUFFER_SIZE;
    }
    while (index >= RECORDS_BUFFER_SIZE) {
       index -= RECORDS_BUFFER_SIZE;
    }

    return &records[index];
}

static int is_branch(op_t op) {
    switch(op.op) {
        case OP_JP: case OP_JR: case OP_CALL:
        case OP_RET: case OP_RETI: case OP_RST:
            return 1;
        default:
            return 0;
    }
}

static int data_length(DATA d) {
    switch (d) {
        case DATA_A16: case DATA_D16:
            return 2;
        default:
            return 1;
    }
}

int op_length(op_t op) {
    switch(op.sig) {
        case OP_SIG_NOARG:
        case OP_SIG_FD:
        case OP_SIG_RMR:
        case OP_SIG_N:
        case OP_SIG_RN:
        case OP_SIG_RMN:
        case OP_SIG_F:
        case OP_SIG_R:
        case OP_SIG_RM:
        case OP_SIG_RR:
        case OP_SIG_RRM:
            return 1;
        case OP_SIG_RMD:
        case OP_SIG_RD:
        case OP_SIG_D:
            return 1 + data_length(op.d);
        case OP_SIG_RDM:
            return 1 + data_length(op.d);
        case OP_SIG_RRD:
        case OP_SIG_DMR:
            return 1 + data_length(op.d);
        default:
            assert(0);
    }
}

int block_index(u16 addr) {
    int b;

    for (b = 0; b < BLOCK_BUFFER_SIZE; b++) {
        if (blocks[b].begin == addr) {
            return b;
        }
    }

    return -1;
}

void record_cpu_cycle() {
    record_t record;

    record.pc = PC;
    last_recorded_pc = PC;

    records[records_cursor] = record;

    records_cursor++;
    records_cursor %= RECORDS_BUFFER_SIZE;

    records_count++;

    if (current_block) {
        if (PC > current_block->end || PC < current_block->begin) {
            current_block = NULL;
        }
    }

    if(!current_block) {
        u16 pc = PC;

        int index = block_index(pc);
        if (index > 0) {
            current_block = &blocks[index];
            //printf("Entered block %i\n", index);
        }
        else {
            current_block = &blocks[block_cursor];
            current_block->begin = pc;

            block_cursor++;
            block_cursor %= BLOCK_BUFFER_SIZE;

            //printf("Entered block %i\n", block_cursor);

            for (;;) {
                op_t op = disasm(pc);

                if (is_branch(op)) {
                    current_block->end = pc;
                    break;
                }

                assert (pc + op_length(op) <= 0xFFFF);

                pc += op_length(op);
            }
        }
    }
}

block_t * record_get_current_block() {
    if (PC != last_recorded_pc) {
        record_cpu_cycle();
    }

    return current_block;
}

