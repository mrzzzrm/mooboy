#ifndef DEBUG_WATCH_H
#define DEBUG_WATCH_H

#include "core/defines.h"

typedef enum {
    WATCHPOINT_MEM_W,
    WATCHPOINT_MEM_RW,
    WATCHPOINT_MEM_R
} WATCHPOINT_TYPE;

typedef struct {
    WATCHPOINT_TYPE type;

    union {
        struct {
            u16 begin;
            u16 end;
        } mem;
    };
} watchpoint_t;

int watch_enable(watchpoint_t watchpoint);
void watch_disable(int id);

void watch_event_mem_w(u16 addr, u8 a, u8 b);
void watch_event_mem_r(u16 addr);

void watch_debug_read_begin();
void watch_debug_read_end();

#endif // DEBUG_WATCH_H
