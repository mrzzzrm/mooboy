#ifndef DEBUG_EVENT_H
#define DEBUG_EVENT_H

#include "core/defines.h"

typedef enum {
    EVENT_JOY
} EVENT_TYPE;

typedef struct {
    EVENT_TYPE type;

    union {
        // EVENT_JOY
        struct {
            u8 button;
            u8 state;
        } joy;
    };
} debug_event_t;

void debug_event(debug_event_t event);

#endif // DEBUG_EVENT_H
