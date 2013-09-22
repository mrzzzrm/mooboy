#include "error.h"
#include "menu/menu.h"
#include "util.h"
#include "sys/sys.h"
#include "core/moo.h"
#include <SDL/SDL.h>

static int finished;
static SDL_Surface *heading_surface;
static SDL_Surface *footer_surface;
static menu_word_string_t *msg_string;


static void draw() {
    SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
    menu_blit(heading_surface, 0, 0);
    menu_blit_word_string(msg_string, 0, 100);
    menu_blit(footer_surface, 0, 400);
    SDL_Flip(SDL_GetVideoSurface());
}

static void setup() {
    finished = 0;

    msg_string = menu_word_string(moo.error->text);
}

static void error_input_event(int type, int key) {
    if(type == SDL_KEYDOWN) {
        finished = 1;
    }
}

void menu_error_init() {
    heading_surface = menu_text("Error!");
    footer_surface = menu_text("Press any button to continue");
}

void menu_error_close() {
    SDL_FreeSurface(heading_surface);
    SDL_FreeSurface(footer_surface);
}

void menu_error() {
    setup();
    while(!finished && (moo.state & MOO_RUNNING_BIT)) {
        draw();
        sys_handle_events(error_input_event);
    }

    menu_free_word_string(msg_string);
    moo_clear_error();
}

