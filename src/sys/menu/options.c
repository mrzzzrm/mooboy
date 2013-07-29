#include "options.h"
#include "sys/sys.h"
#include "util.h"
#include <SDL/SDL.h>

#define LABEL_SOUND 0


static menu_list_t *list;
static int finished;

static void set_sound(int on) {
    menu_listentry_val(list, LABEL_SOUND, on ? "on" : "off");
    sys.sound_on = on;
}

static void draw() {
    SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
    menu_draw_list(list);
    SDL_Flip(SDL_GetVideoSurface());
}

void menu_options_init() {
    list = menu_new_list("Options");

    menu_new_listentry(list, "Sound", LABEL_SOUND);
    set_sound(sys.sound_on);
}

void menu_options_close() {
    menu_free_list(list);
}

static void options_input_event(int type, int key) {
    if(type == SDL_KEYDOWN) {
        switch(menu_list_selected_id(list)) {
            case LABEL_SOUND:
                if(key == SDLK_LEFT || key == SDLK_RIGHT) {
                    set_sound(!sys.sound_on);
                }
            break;
        }

        if(key == KEY_BACK) {
            finished = 1;
        }
    }
}

void menu_options() {
    finished = 0;

    while(sys.running && !finished) {
        sys_handle_events(options_input_event);
        draw();
    }
}

