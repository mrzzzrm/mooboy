#include "break.h"

#include <stdio.h>
#include <assert.h>

typedef struct {
    breakpoint_t breakpoint;
    int enabled;
} breakpoint_handle_t;

#define BREAKPOINT_BUFFER_SIZE 256

static breakpoint_handle_t breakpoints[BREAKPOINT_BUFFER_SIZE];
static int breakpoint_cursor = 0;
static int now = 0;

int break_enable(breakpoint_t breakpoint) {
    assert(breakpoint_cursor < BREAKPOINT_BUFFER_SIZE);

    breakpoint_handle_t handle;
    handle.breakpoint = breakpoint;
    handle.enabled = 1;

    breakpoints[breakpoint_cursor] = handle;
    breakpoints[breakpoint_cursor].enabled = 1;
    breakpoint_cursor++;

    switch(breakpoints[breakpoint_cursor-1].breakpoint.type) {
        case BREAKPOINT_ADDRESS: printf("Breakpoint %i set on address %.4X\n", breakpoint_cursor - 1, breakpoint.address.pc); break;
        case BREAKPOINT_EVENT: printf("Breakpoint %i set on event\n", breakpoint_cursor - 1); break;
    }

    return breakpoint_cursor - 1;
}

void break_disable(int id) {
    assert(id >= 0 && id < BREAKPOINT_BUFFER_SIZE);
    breakpoints[id].enabled = 0;
}

void break_handle_event(event_t event) {
    int b;
    for (b = 0; b < breakpoint_cursor; b++) {
        if (breakpoints[b].breakpoint.type == BREAKPOINT_ADDRESS) {
            if (event.type == EVENT_PROGRAM_COUNTER && event.address.pc == breakpoints[b].breakpoint.address.pc ) {
                now = 1;
            }
        }
        if (breakpoints[b].breakpoint.type == BREAKPOINT_EVENT) {
            if (event.type == EVENT_JOY_NOTICED) {
                now = 1;
            }
        }
    }
}

int break_now() {
    int tmp = now;
    if (now) {
        now = 0;
    }
    return tmp;
}

void debug_break() { printf("BREAKING\n");
    now = 1;
}

