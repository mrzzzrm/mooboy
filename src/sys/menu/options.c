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

static char *local_config_path() {
    sprintf(_local_path, "%s.conf", sys.rompath);
    return _local_path;
}

static void update_options() {
    set_sound(sys.sound_on);
}

static void save_local() {
    config_save(local_config_path());
    update_options();
}

static void load_local() {
    config_load(local_config_path());
    update_options();
}

static void save_global() {
    config_load("mooboy.conf");
    update_options();
}

static void load_global() {
    config_load("mooboy.conf");
    update_options();
}

static void draw() {
    SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
    menu_draw_list(list);
    SDL_Flip(SDL_GetVideoSurface());
}

void menu_options_init() {
    list = menu_new_list("Options");

    menu_new_listentry(list, "Sound", LABEL_SOUND, NULL);
    menu_new_listentry(list, "Save gameconfig", LABEL_SAVE_LOCAL, save_local);
    menu_new_listentry(list, "Load gameconfig", LABEL_LOAD_LOCAL, load_local);
    menu_new_listentry(list, "Save global config", LABEL_SAVE_GLOBAL, save_global);
    menu_new_listentry(list, "Load global config", LABEL_LOAD_GLOBAL, load_global);
    menu_new_listentry(list, "Reset to default", LABEL_RESET, NULL);
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
        if(key == KEY_BACK) {
            finished = 1;
        }
    }
}

void menu_options() {
    finished = 0;

    menu_listentry_visible(list, LABEL_SAVE_LOCAL, sys.state & MOO_ROM_LOADED_BIT);
    menu_listentry_visible(list, LABEL_LOAD_LOCAL, sys.state & MOO_ROM_LOADED_BIT);

    while(!finished && (sys.state & MOO_RUNNING_BIT)) {
        draw();
        sys_handle_events(options_input_event);
        menu_list_update(list);
    }
}

