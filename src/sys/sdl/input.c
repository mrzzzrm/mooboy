#include "input.h"
#include "state.h"
#include "core/joy.h"
#include "sys/sys.h"
#include <SDL/SDL.h>

input_t input;

void input_init() {
    input.keys.up = SDLK_UP;
    input.keys.down = SDLK_DOWN;
    input.keys.left = SDLK_LEFT;
    input.keys.right = SDLK_RIGHT;
#ifdef PANDORA
    input.keys.a = SDLK_END;
    input.keys.b = SDLK_PAGEDOWN;
    input.keys.start = SDLK_LALT;
    input.keys.select = SDLK_RCTRL;
#else
    input.keys.a = SDLK_a;
    input.keys.b = SDLK_s;
    input.keys.start = SDLK_w;
    input.keys.select = SDLK_d;
#endif

    input.keys.save = SDLK_n;
    input.keys.load = SDLK_m;
    input.keys.menu = SDLK_SPACE;
}

void input_event(int type, int key) {
    u8 state = type == SDL_KEYUP ? JOY_STATE_RELEASED : JOY_STATE_PRESSED;

    if(type == SDL_KEYUP) {
        if(key == input.keys.menu) {
            sys.in_menu = !sys.in_menu;
            if(sys.in_menu) {
                sys_pause();
            }
            else {
                sys_run();
            }
        }
    }

    if(key == input.keys.up)    joy_set_button(JOY_BUTTON_UP, state);
    if(key == input.keys.down)  joy_set_button(JOY_BUTTON_DOWN, state);
    if(key == input.keys.left)  joy_set_button(JOY_BUTTON_LEFT, state);
    if(key == input.keys.right) joy_set_button(JOY_BUTTON_RIGHT, state);

    if(key == input.keys.a)      joy_set_button(JOY_BUTTON_A, state);
    if(key == input.keys.b)      joy_set_button(JOY_BUTTON_B, state);
    if(key == input.keys.start)  joy_set_button(JOY_BUTTON_START, state);
    if(key == input.keys.select) joy_set_button(JOY_BUTTON_SELECT, state);
}
