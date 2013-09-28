#include "continue.h"
#include "core/moo.h"
#include "menu/menu.h"
#include "sys/sdl/input.h"
#include "util/continue.h"
#include "util.h"

#define NO 0
#define YES 1

static menu_label_t *question;
static menu_label_t *yes, *no;
static int selection;
static int finished;

void menu_continue_init() {
    question = menu_label("Continue right where you left?");
    yes = menu_label("Yes");
    no = menu_label("No");
}

void menu_continue_close() {
    menu_free_label(question);
    menu_free_label(yes);
    menu_free_label(no);
}

static void draw() {
    SDL_Surface *screen = SDL_GetVideoSurface();

    SDL_FillRect(screen, NULL, 0);
    menu_blit_label(question, ALIGN_CENTER, 0, screen->w/2, screen->h/3);
    menu_blit_label(yes, ALIGN_CENTER, selection == YES, screen->w/3, screen->h*2/3);
    menu_blit_label(no, ALIGN_CENTER, selection == NO, screen->w*2/3, screen->h*2/3);

    SDL_Flip(screen);
}

static void continue_input_event(int type, int key) {
    if(type == SDL_KEYDOWN) {
        if(key == SDLK_LEFT || key == SDLK_RIGHT) {
            selection = selection == YES ? NO : YES;
        }
        if(key == input.keys.accept) {
            switch(selection) {
                case YES: continue_state_load(); break;
                case NO: moo_continue(); break;
            }

            finished = 1;
        }
    }
}

void menu_continue() {
    selection = YES;
    finished = 0;

    while(!finished && (moo.state & MOO_RUNNING_BIT)) {
        draw();
        sys_handle_events(continue_input_event);
    }
}

