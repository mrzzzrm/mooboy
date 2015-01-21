#include <assert.h>
#include "cpu.h"
#include "defines.h"
#include "joy.h"

#ifdef DEBUG
#include "debug/event.h"
#endif // DEBUG

#define SELECT_DIRECTION_BIT 0x10
#define SELECT_ACTION_BIT 0x20

joy_t joy;

void joy_reset() {
    joy.state = 0xFF;
    joy.col = 0;
}

void joy_set_button(u8 button, u8 state) {
    u8 old_state = joy.state & button ? JOY_STATE_RELEASED : JOY_STATE_PRESSED;
    if(old_state != state) {
#ifdef DEBUG
        debug_event_t event;
        event.type = EVENT_JOY;
        event.joy.button = button;
        event.joy.state = state;

        debug_event(event);
#endif // DEBUG
        if(state) {
            joy.state |= button;
        }
        else {
            joy.state ^= button;
            cpu.irq |= IF_JOYPAD;
        }
    }
}

void joy_select_col(u8 flag) {
    if((~flag) & SELECT_ACTION_BIT) {
        joy.col = 1;
    }
    else if((~flag) & SELECT_DIRECTION_BIT) {
        joy.col = 0;
    }
}

u8 joy_read() {
    if(joy.col == 0)
        return joy.state & 0x0F;
    else
        return joy.state >> 4;
}

