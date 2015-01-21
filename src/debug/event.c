#include "event.h"

#include "assert.h"

#include "core/joy.h"

static debug_event_t current_event;

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
    }
}

static const char* joy_state() {
    switch(current_event.joy.state) {
        case JOY_STATE_PRESSED: return "Pressed";
        case JOY_STATE_RELEASED: return "Released";
    }
}

static void joy_event() {
    printf("JOY: %s %s\n", joy_button(), joy_state());
}

void debug_event(debug_event_t event) {
    current_event = event;

    switch(event.type) {
        case EVENT_JOY: joy_event(); break;
        default:
            assert(0);
    }
}

