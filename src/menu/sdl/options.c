#include "options.h"
#include "util/config.h"
#include "core/mbc.h"
#include "sys/sys.h"
#include "util.h"
#include "core/moo.h"
#include <SDL/SDL.h>

#define LABEL_SOUND 0
#define LABEL_SCALING 1
#define LABEL_STATUSBAR 2
#define LABEL_AUTO_CONTINUE 3
#define LABEL_AUTO_RTC 4
#define LABEL_SAVE_LOCAL 5
#define LABEL_LOAD_LOCAL 6
#define LABEL_SAVE_GLOBAL 7
#define LABEL_LOAD_GLOBAL 8
#define LABEL_RESET 9


static menu_list_t *list;
static int finished;
static char *rom_config_path = NULL;

static void back() {
    finished = 1;
}

static void change_sound(int dir) {
    sys.sound_on = dir ? !sys.sound_on : sys.sound_on;
    menu_listentry_val(list, LABEL_SOUND, sys.sound_on ? "on" : "off");
}

static void change_scaling(int dir) {
    sys_set_scalingmode((sys.scalingmode + dir + sys.num_scalingmodes) % sys.num_scalingmodes); // Since -1 % 5 != 4 this does (-1+5)%5
    menu_listentry_val(list, LABEL_SCALING,  sys.scalingmode_names[sys.scalingmode]);
}

static void change_statusbar(int dir) {
    sys.show_statusbar = dir ? !sys.show_statusbar : sys.show_statusbar;
    menu_listentry_val(list, LABEL_STATUSBAR, sys.show_statusbar ? "show" : "hide");
}

static void change_auto_continue(int dir) {
    sys.auto_continue += dir;
    sys.auto_continue = sys.auto_continue < 0 ? SYS_AUTO_CONTINUE_YES : sys.auto_continue % 3;
    menu_listentry_val(list, LABEL_AUTO_CONTINUE, sys.auto_continue == SYS_AUTO_CONTINUE_YES ? "yes" : sys.auto_continue == SYS_AUTO_CONTINUE_NO ? "no" : "ask me");
}

static void change_auto_rtc(int dir) {
    sys.auto_rtc = dir ? !sys.auto_rtc : sys.auto_rtc;
    menu_listentry_val(list, LABEL_AUTO_RTC, sys.auto_rtc ? "yes" : "no");
}

static void update_options() {
    change_sound(0);
    change_scaling(0);
    change_statusbar(0);
    change_auto_continue(0);
    change_auto_rtc(0);
}

static void reset() {
    config_default();
    update_options();
}

static void save_local() {
    config_save(rom_config_path);
    update_options();
}

static void load_local() {
    config_load(rom_config_path);
    update_options();
}

static void save_global() {
    config_save("mooboy.conf");
    update_options();
}

static void load_global() {
    if(!config_load("mooboy.conf")) {
        reset();
    }
    update_options();
}

static void draw() {
    SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
    menu_draw_list(list);
    SDL_Flip(SDL_GetVideoSurface());
}

void menu_options_init() {
    list = menu_new_list("Options");
    list->back_func = back;

    menu_new_listentry_selection(list, "Sound", LABEL_SOUND, change_sound);
    menu_new_listentry_selection(list, "Scaling", LABEL_SCALING, change_scaling);
    menu_new_listentry_selection(list, "Statusbar", LABEL_STATUSBAR, change_statusbar);
    menu_new_listentry_selection(list, "Auto-Continue", LABEL_AUTO_CONTINUE, change_auto_continue);
    menu_new_listentry_selection(list, "Tick RTC when ROM not loaded", LABEL_AUTO_RTC, change_auto_rtc);

    menu_new_listentry_spacer(list);

    menu_new_listentry_button(list, "Save gameconfig", LABEL_SAVE_LOCAL, save_local);
    menu_new_listentry_button(list, "Load gameconfig", LABEL_LOAD_LOCAL, load_local);
    menu_new_listentry_button(list, "Save global config", LABEL_SAVE_GLOBAL, save_global);
    menu_new_listentry_button(list, "Load global config", LABEL_LOAD_GLOBAL, load_global);
    menu_new_listentry_button(list, "Reset to default", LABEL_RESET, reset);

    update_options();
}

void menu_options_close() {
    menu_free_list(list);

    if(rom_config_path != NULL) {
        free(rom_config_path);
    }
}

static void options_input_event(int type, int key) {
    menu_list_input(list, type, key);
}

static void setup() {
    finished = 0;

    menu_listentry_visible(list, LABEL_AUTO_RTC, mbc.has_rtc);
    menu_listentry_visible(list, LABEL_SAVE_LOCAL, moo.state & MOO_ROM_LOADED_BIT);
    menu_listentry_visible(list, LABEL_LOAD_LOCAL, moo.state & MOO_ROM_LOADED_BIT);

    update_options();

    if(moo.state & MOO_ROM_LOADED_BIT) {
        rom_config_path = realloc(rom_config_path, strlen(sys.rompath) + strlen(".conf") + 1);
        sprintf(rom_config_path, "%s.conf", sys.rompath);
    }
}

void menu_options() {
    setup();

    while(!finished && (moo.state & MOO_RUNNING_BIT)) {
        draw();
        sys_handle_events(options_input_event);
        menu_list_update(list);
    }
}

