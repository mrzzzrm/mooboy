#include "event.h"

#include <stdio.h>
#include <assert.h>

#include "core/joy.h"

#include "break.h"
#include "debug.h"

static event_t current_event;

static const char* joy_button() {
    switch(current_event.joy.button) {
        case JOY_BUTTON_RIGHT: return "Right";
        case JOY_BUTTON_LEFT: return "Left";
        case JOY_BUTTON_UP: return "Up";
        case JOY_BUTTON_DOWN: return "Down";
        case JOY_BUTTON_A: return "A";
        case JOY_BUTTON_B: return "B";
        case JOY_BUTTON_SELECT: return "Select";
        case JOY_BUTTON_START: return "Start";
        default:
            assert(0);
    }
}

static const char* joy_state() {
    switch(current_event.joy.state) {
        case JOY_STATE_PRESSED: return "Pressed";
        case JOY_STATE_RELEASED: return "Released";
        default:
            assert(0);
    }
}

static void joy_input_event() {
    printf("JOY INPUT: %s %s\n", joy_button(), joy_state());
    break_handle_event(current_event);
}

static void joy_read_event() {
    printf("JOY READ: Pad %.2X Buttons %.2X\n", (joy.state >> 4), (joy.state & 0x0F));
    break_handle_event(current_event);
}

void debug_event(event_t event) {
    if (in_debug_scope()) {
        return;
    }

    current_event = event;

    switch(event.type) {
        case EVENT_JOY_INPUT: joy_input_event(); break;
        case EVENT_JOY_NOTICED: joy_read_event(); break;
        default:
            assert(0);
    }
}

