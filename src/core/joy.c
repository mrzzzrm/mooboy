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

static void update_reg() {
    u8 old_lines = joy.reg;

    joy.reg &= 0x30;
    joy.reg |= 0xC0;

    u8 keys = ~joy.state;

    if (!(joy.reg & SELECT_DIRECTION_BIT)) {
        joy.reg |= (keys & 0x0F);
    }

    if (!(joy.reg & SELECT_ACTION_BIT)) {
        joy.reg |= (keys >> 4);
    }

    joy.reg ^= 0x0F;

    if (old_lines & ~joy.reg & 0x0F) {
        cpu.irq |= IF_JOYPAD;
    }
}

void joy_reset() {
    joy.state = 0xFF;
    joy.reg = 0xFF;
}

void joy_set_button(u8 button, u8 state) {
    u8 old_state = joy.state & button ? JOY_STATE_RELEASED : JOY_STATE_PRESSED;

    if(old_state != state) {
#ifdef DEBUG
        event_t event;
        event.type = EVENT_JOY_INPUT;
        event.joy.button = button;
        event.joy.state = state;

        debug_event(event);
#endif // DEBUG

        if(state) {
            joy.state |= button;
        }
        else {
            joy.state &= ~button;
        }

        update_reg();
    }
}

void joy_write(u8 val) {
    joy.reg = val;
    update_reg();
}

u8 joy_read() {
    return joy.reg;
}

