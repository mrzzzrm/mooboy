#ifndef DEBUG_BREAK_H
#define DEBUG_BREAK_H

#include "core/defines.h"

#include "event.h"

typedef enum {
    BREAKPOINT_ADDRESS,
    BREAKPOINT_EVENT
} BREAKPOINT_TYPE;

typedef struct {
    BREAKPOINT_TYPE type;

    union {
        // BREAK_ADDRESS
        struct {
            u16 pc;
        } address;

        // BREAK_EVENT
        struct {
            EVENT_TYPE type;
        } event;
    };
} breakpoint_t;

int break_enable(breakpoint_t breakpoint);
void break_disable(int id);
void break_handle_event(event_t event);
int break_now();
void debug_break();

#endif // DEBUG_BREAK_H
