#ifndef DEBUG_EVENT_H
#define DEBUG_EVENT_H

#include "core/defines.h"

typedef enum {
    EVENT_JOY_INPUT,
    EVENT_JOY_NOTICED
} EVENT_TYPE;

typedef struct {
    EVENT_TYPE type;

    union {
        // EVENT_JOY_INPUT/EVENT_JOY_NOTICED
        struct {
            u8 button;
            u8 state;
        } joy;
    };
} event_t;

void debug_event(event_t event);

#endif // DEBUG_EVENT_H
