#include "watch.h"

#include <assert.h>
#include <stdio.h>

#include "core/mem.h"

#include "break.h"
#include "debug.h"

#define WATCHPOINT_BUFFER_SIZE 256

static int watchpoint_cursor = 0;
static watchpoint_t watchpoints[WATCHPOINT_BUFFER_SIZE];
static int watchpoint_enabled[WATCHPOINT_BUFFER_SIZE] = {0};

int watch_enable(watchpoint_t watchpoint) {
    assert(watchpoint_cursor < WATCHPOINT_BUFFER_SIZE);
    watchpoints[watchpoint_cursor] = watchpoint;
    watchpoint_enabled[watchpoint_cursor] = 1;
    watchpoint_cursor++;

    char *info;
    switch(watchpoint.type) {
        case WATCHPOINT_MEM_W: info = "WRITE"; break;
        case WATCHPOINT_MEM_RW: info = "READ/WRITE"; break;
        case WATCHPOINT_MEM_R: info = "READ"; break;
    }
    printf("%s WATCHPOINT %i from %.4X..%.4X enabled\n", info, watchpoint_cursor - 1, watchpoint.mem.begin, watchpoint.mem.end);

    return watchpoint_cursor - 1;
}

void watch_disable(int id) {
    assert(id < WATCHPOINT_BUFFER_SIZE && id >= 0);
    watchpoint_enabled[id] = 0;
}

void watch_event_mem_w(u16 addr, u8 a, u8 b) {
    if (in_debug_scope()) {
        return;
    }

    int wp;
    for (wp = 0; wp < watchpoint_cursor; wp++) {
        if (!watchpoint_enabled[wp]) {
            continue;
        }

        switch (watchpoints[wp].type) {
            case WATCHPOINT_MEM_RW:
            case WATCHPOINT_MEM_W:
                if (watchpoints[wp].mem.begin <= addr && watchpoints[wp].mem.end >= addr) {
                    printf("WATCHPOINT %i: %.4X changed from %.2X to %.2X\n", wp, addr, a, b);
                    debug_break();
                }
            default:;
        }
    }
}

void watch_event_mem_r(u16 addr) {
    if (in_debug_scope()) {
        return;
    }

    int wp;
    for (wp = 0; wp < watchpoint_cursor; wp++) {
        if (!watchpoint_enabled[wp]) {
            continue;
        }

        switch (watchpoints[wp].type) {
            case WATCHPOINT_MEM_RW:
            case WATCHPOINT_MEM_R:
                if (watchpoints[wp].mem.begin <= addr && watchpoints[wp].mem.end >= addr) {
                    printf("WATCHPOINT %i: %.4X read %.2X\n", wp, addr, debug_read_byte(addr));
                    debug_break();
                }
            default:;
        }
    }
}

