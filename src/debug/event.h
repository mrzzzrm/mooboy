#ifndef DEBUG_EVENT_H
#define DEBUG_EVENT_H

#include "core/defines.h"

typedef enum {
    EVENT_JOY_INPUT,
    EVENT_JOY_NOTICED,
    EVENT_PROGRAM_COUNTER
} EVENT_TYPE;

typedef struct {
    EVENT_TYPE type;

    union {
        // EVENT_JOY_INPUT/EVENT_JOY_NOTICED
        struct {
            u8 button;
            u8 state;
        } joy;
        // EVENT_PROGRAM_COUNTER
        struct {
          u16 pc;
        } address;
    };
} event_t;

void debug_event(event_t event);

#endif // DEBUG_EVENT_H
