#include "menu/menu.h"
#include "error.h"
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


#define LABEL_RESUME            0
#define LABEL_CONTINUE_LAST_ROM 1
#define LABEL_LOAD_LAST_ROM     2
#define LABEL_LOAD_ROM          3
#define LABEL_RESET             4
#define LABEL_OPTIONS           5
#define LABEL_LOAD_STATE        6
#define LABEL_SAVE_STATE        7
#define LABEL_CONNECT           8
#define LABEL_QUIT              9


static menu_list_t *list = NULL;
static int load_slot = 0;
static int save_slot = 0;

static int exists(const char *path) {
    FILE *f = fopen(path, "r");
    if(f != NULL) {
        fclose(f);
    }
    return f != NULL;
}

static char *last_rom_path() {
    FILE *f = fopen("lastrom.txt", "r");
    if(f == NULL) {
        return NULL;
    }
    fseek(f, 0, SEEK_END);
    long pathlen = ftell(f);
    char *path = malloc(pathlen + 1);
    fseek(f, 0, SEEK_SET);
    fread(path, 1, pathlen, f);
    fclose(f);
    path[pathlen] = '\0';
    return path;
}

static char *last_state_path() {
    char *rom_path = last_rom_path();
    if(rom_path == NULL) {
        return NULL;
    }

    char *ext = ".continue.sav";
    int state_path_len = strlen(rom_path) + strlen(ext);
    char *state_path = malloc(state_path_len + 1);
    sprintf(state_path, "%s%s", rom_path, ext);

    return state_path;
}

static int have_last_state() {
    char *state_path = last_state_path();

    if(state_path == NULL) {
        return 0;
    }

    int re = exists(state_path);
    free(state_path);

    return re;
}

static int have_last_rom() {
    char *path = last_rom_path();
    int re = exists(path);
    free(path);
    return re;
}

static void load_last_rom() {
    char *path = last_rom_path();
    moo_load_rom(path);
    free(path);
}

static void continue_last_rom() {
    load_last_rom();
    char *state_path = last_state_path();
    state_load(state_path);
    free(state_path);
}

static void back() {
    if(moo.state & MOO_ROM_LOADED_BIT) {
        moo_continue();
    }
}

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
    snprintf(file, sizeof(file), "%s.sav%i", sys.rompath, load_slot);
    if(state_load(file)) {
        if((~moo.state & MOO_ERROR_BIT)) {
            moo_begin();
        }
    }
}

static void save_state() {
    char file[256];
    snprintf(file, sizeof(file), "%s.sav%i", sys.rompath, save_slot);
    state_save(file);
}

static void resume() {
    moo_continue();
}

static void reset() {
    moo_load_rom(sys.rompath);
    moo_begin();
}

static void quit() {
    moo_quit();
}

static const char *filename(const char *filepath) {
    char *begin = strrchr(filepath, '/');
    if(begin == NULL) {
        return filepath;
    }
    return &begin[1];
}

static void setup() {
    int _have_last_rom = have_last_rom();
    int _have_last_state = have_last_state();

    menu_listentry_visible(list, LABEL_CONTINUE_LAST_ROM, (~moo.state & MOO_ROM_LOADED_BIT) && _have_last_rom && _have_last_state);
    menu_listentry_visible(list, LABEL_LOAD_LAST_ROM, (~moo.state & MOO_ROM_LOADED_BIT) && _have_last_rom);
    menu_listentry_visible(list, LABEL_RESUME, moo.state & MOO_ROM_LOADED_BIT);
    menu_listentry_visible(list, LABEL_RESET, moo.state & MOO_ROM_LOADED_BIT);
    menu_listentry_visible(list, LABEL_LOAD_STATE, moo.state & MOO_ROM_LOADED_BIT);
    menu_listentry_visible(list, LABEL_SAVE_STATE, moo.state & MOO_ROM_LOADED_BIT);

    menu_list_select_first(list);

    if(_have_last_rom) {
        char *_last_rom_path = last_rom_path();
        const char *rom_filename = filename(_last_rom_path);
        menu_listentry_textf(list, LABEL_LOAD_LAST_ROM, "Load %s", rom_filename);
        if(_have_last_state) {
            menu_listentry_textf(list, LABEL_CONTINUE_LAST_ROM, "Continue with %s", rom_filename);
        }
        free(_last_rom_path);
    }
}

static void draw() {
    SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);

    menu_draw_list(list);

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

static void mainmenu() {
    setup();
    while((~moo.state & MOO_ROM_RUNNING_BIT) && (moo.state & MOO_RUNNING_BIT) && (~moo.state & MOO_ERROR_BIT)) {
        draw();
        sys_handle_events(menu_input_event);
        menu_list_update(list);
    }
}


void menu_init() {
    menu_util_init();
    menu_rom_init();
    menu_options_init();
    menu_error_init();

    list = menu_new_list("Main Menu");
    list->back_func = back;

    menu_new_listentry_button(list, "", LABEL_CONTINUE_LAST_ROM, continue_last_rom);
    menu_new_listentry_button(list, "", LABEL_LOAD_LAST_ROM, load_last_rom);
    menu_new_listentry_button(list, "Resume", LABEL_RESUME, resume);
    menu_new_listentry_button(list, "Load ROM", LABEL_LOAD_ROM, menu_rom);
    menu_new_listentry_button(list, "Reset", LABEL_RESET, reset);
    menu_new_listentry_button(list, "Load state", LABEL_LOAD_STATE, load_state);
    menu_new_listentry_button(list, "Save state", LABEL_SAVE_STATE, save_state);
    menu_new_listentry_button(list, "Options", LABEL_OPTIONS, menu_options);
    //menu_new_listentry_button(list, "Connect to mooLounge", LABEL_CONNECT);
    menu_new_listentry_button(list, "Quit", LABEL_QUIT, quit);

    set_slot(LABEL_LOAD_STATE, 0);
    set_slot(LABEL_SAVE_STATE, 0);
}


void menu_close() {
    menu_free_list(list);

    menu_rom_close();
    menu_options_close();
    menu_error_close();
    menu_util_close();
}

void menu_run() {
    while((~moo.state & MOO_ROM_RUNNING_BIT) && (moo.state & MOO_RUNNING_BIT)) {
        if(moo.state & MOO_ERROR_BIT) {
            menu_error();
        }
        else {
            mainmenu();
        }
    }
}

