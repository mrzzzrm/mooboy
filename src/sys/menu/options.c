#include "options.h"
#include "sys/sdl/config.h"
#include "sys/sys.h"
#include "util.h"
#include <SDL/SDL.h>

#define LABEL_SOUND 0
#define LABEL_SAVE_LOCAL 1
#define LABEL_LOAD_LOCAL 2
#define LABEL_SAVE_GLOBAL 3
#define LABEL_LOAD_GLOBAL 4
#define LABEL_RESET 4


static menu_list_t *list;
static int finished;
static char _local_path[256];

static void set_sound(int on) {
    menu_listentry_val(list, LABEL_SOUND, on ? "on" : "off");
    sys.sound_on = on;
}

static void update_options() {
    set_sound(sys.sound_on);
}

static void draw() {
    SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
    menu_draw_list(list);
    SDL_Flip(SDL_GetVideoSurface());
}

static char *local_config_path() {
    sprintf(_local_path, "%s.conf", sys.rompath);
    return _local_path;
}

void menu_options_init() {
    list = menu_new_list("Options");

    menu_new_listentry(list, "Sound", LABEL_SOUND);
    menu_new_listentry(list, "Save gameconfig", LABEL_SAVE_LOCAL);
    menu_new_listentry(list, "Load gameconfig", LABEL_LOAD_LOCAL);
    menu_new_listentry(list, "Save global config", LABEL_SAVE_GLOBAL);
    menu_new_listentry(list, "Load global config", LABEL_LOAD_GLOBAL);
    menu_new_listentry(list, "Reset to default", LABEL_RESET);
    set_sound(sys.sound_on);
}

void menu_options_close() {
    menu_free_list(list);
}

static void options_input_event(int type, int key) {
    menu_list_input(list, type, key);

    if(type == SDL_KEYDOWN) {
        switch(menu_list_selected_id(list)) {
            case LABEL_SOUND:
                if(key == SDLK_LEFT || key == SDLK_RIGHT) {
                    set_sound(!sys.sound_on);
                }
            break;
        }
        if(key == KEY_ACCEPT) {
            switch(menu_list_selected_id(list)) {
                case LABEL_SAVE_LOCAL: config_save(local_config_path()); update_options(); break;
                case LABEL_LOAD_LOCAL: config_load(local_config_path()); update_options(); break;
                case LABEL_SAVE_GLOBAL: config_load("mooboy.conf"); update_options(); break;
                case LABEL_LOAD_GLOBAL: config_load("mooboy.conf"); update_options(); break;
            }
        }
        if(key == KEY_BACK) {
            finished = 1;
        }
    }
}

void menu_options() {
    finished = 0;

    menu_listentry_visible(list, LABEL_SAVE_LOCAL, sys.rom_loaded);
    menu_listentry_visible(list, LABEL_LOAD_LOCAL, sys.rom_loaded);

    while(sys.running && !finished) {
        draw();
        sys_handle_events(options_input_event);
        menu_list_update(list);
    }
}

