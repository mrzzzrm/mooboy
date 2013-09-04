#include "menu/menu.h"
#include "rom.h"
#include "options.h"
#include "core/moo.h"
#include "sys/sys.h"
#include "sys/sdl/input.h"
#include "sys/sdl/video.h"
#include "util/state.h"
#include <assert.h>
#include "util.h"
#include <SDL/SDL.h>


#define LABEL_RESUME     0
#define LABEL_LOAD_ROM   1
#define LABEL_RESET      2
#define LABEL_OPTIONS    3
#define LABEL_LOAD_STATE 4
#define LABEL_SAVE_STATE 5
#define LABEL_CONNECT    6
#define LABEL_QUIT       7


static menu_list_t *list = NULL;
static int load_slot = 0;
static int save_slot = 0;
static int finished;

static void set_slot(int label, int i) {
    if(i < 0 || i > 9) {
        return;
    }

    menu_listentry_val_int(list, label, i);

    if(label == LABEL_LOAD_STATE) {
        load_slot = i;
    }
    else {
        save_slot = i;
    }
}

static void load_state() {
    char file[256];
    sprintf(file, "%s.sav%i", sys.rompath, load_slot);
    if(state_load(file) == 0) {
        moo_begin();
    }
}

static void save_state() {
    char file[256];
    sprintf(file, "%s.sav%i", sys.rompath, save_slot);
    state_save(file);
}

static void resume() {
    moo_continue();
}

static void reset() {

}

static void quit() {
    finished = 1;
    moo_quit();
}

static void setup() {
    finished = 0;

    menu_listentry_visible(list, LABEL_RESUME, moo.state & MOO_ROM_LOADED_BIT);
    menu_listentry_visible(list, LABEL_RESET, moo.state & MOO_ROM_LOADED_BIT);
    menu_listentry_visible(list, LABEL_LOAD_STATE, moo.state & MOO_ROM_LOADED_BIT);
    menu_listentry_visible(list, LABEL_SAVE_STATE, moo.state & MOO_ROM_LOADED_BIT);
    menu_list_select_first(list);
}

static void draw() {
    SDL_Rect prevr = {250, 10, 200, 180};

    SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
    menu_draw_list(list);

    if(moo.state & MOO_ROM_LOADED_BIT) {
        video_render(SDL_GetVideoSurface(), prevr);
    }

    SDL_Flip(SDL_GetVideoSurface());
}

static void menu_input_event(int type, int key) {
    menu_list_input(list, type, key);

    if(type == SDL_KEYDOWN) {
        int label = menu_list_selected_id(list);

        switch(label) {
            case LABEL_LOAD_STATE:
                set_slot(label, key == SDLK_LEFT ? load_slot-1 : key == SDLK_RIGHT ? load_slot+1 : load_slot);
            break;
            case LABEL_SAVE_STATE:
                set_slot(label, key == SDLK_LEFT ? save_slot-1 : key == SDLK_RIGHT ? save_slot+1 : save_slot);
            break;
        }
    }
}


void menu_init() {
    menu_util_init();
    menu_rom_init();
    menu_options_init();

    list = menu_new_list("Main Menu");

    menu_new_listentry(list, "Resume", LABEL_RESUME, resume);
    menu_new_listentry(list, "Load ROM", LABEL_LOAD_ROM, menu_rom);
    menu_new_listentry(list, "Reset", LABEL_RESET, reset);
    menu_new_listentry(list, "Load state", LABEL_LOAD_STATE, load_state);
    menu_new_listentry(list, "Save state", LABEL_SAVE_STATE, save_state);
    menu_new_listentry(list, "Options", LABEL_OPTIONS, menu_options);
    //menu_new_listentry(list, "Connect to mooLounge", LABEL_CONNECT);
    menu_new_listentry(list, "Quit", LABEL_QUIT, quit);

    set_slot(LABEL_LOAD_STATE, 0);
    set_slot(LABEL_SAVE_STATE, 0);
}


void menu_close() {
    menu_free_list(list);
    menu_rom_close();
    menu_options_close();
}

void menu_run() {
    setup();

    while(!finished && (~moo.state & MOO_ROM_RUNNING_BIT) && (moo.state & MOO_RUNNING_BIT)) {
        draw();
        sys_handle_events(menu_input_event);
        menu_list_update(list);
    }
}


