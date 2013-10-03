#include "dialogs.h"
#include "dialog.h"
#include "menu/menu.h"
#include "sys/sys.h"
#include "core/moo.h"
#include "sys/sdl/input.h"
#include "util.h"

static int finished;
static int selection;
static menu_dialog_t *dialog;


menu_dialog_t *new_dialog(const char *prompt) {
    menu_dialog_t *dialog = malloc(sizeof(*dialog));
    memset(dialog, 0x00, sizeof(*dialog));
    dialog->prompt = menu_word_string(prompt);

    return dialog;
}

menu_dialog_t *menu_dialog_new(const char *question, void (*yes_func)(), void (*no_func)()) {
    menu_dialog_t *dialog = new_dialog(question);
    dialog->selections[0] = menu_label("Yes");
    dialog->selections[1] = menu_label("No");
    dialog->selection_funcs[0] = yes_func;
    dialog->selection_funcs[1] = no_func;
    dialog->active_selections = 2;

    return dialog;
}

menu_dialog_t *menu_dialog_new_message(const char *message) {
    menu_dialog_t *dialog = new_dialog(message);
    dialog->selections[0] = menu_label("Ok");
    dialog->active_selections = 1;

    return dialog;
}

void menu_dialog_free(menu_dialog_t *dialog) {
    int s;

    menu_free_word_string(dialog->prompt);

    for(s = 0; s < dialog->active_selections; s++) {
        if(dialog->selections[s]) {
             menu_free_label(dialog->selections[s]);
        }
    }

    free(dialog);
}

static void draw() {
    int sel;

    SDL_Surface *screen = SDL_GetVideoSurface();

    SDL_FillRect(screen, NULL, 0);
    menu_blit_word_string(dialog->prompt, screen->w/2, screen->h/3);

    for(sel = 0; sel < dialog->active_selections; sel++) {
        menu_blit_label(dialog->selections[sel], ALIGN_CENTER, selection == sel ? 1 : 0, screen->w/(dialog->active_selections+1) * (sel+1), screen->h*2/3);
    }

    SDL_Flip(screen);
}

static void dialog_input_event(int type, int key) {
    if(type == SDL_KEYDOWN) {
        if(key == SDLK_LEFT) {
            selection--;
            if(selection < 0) {
                selection += dialog->active_selections;
            }
        }
        if(key == SDLK_RIGHT) {
            selection++;
            selection %= dialog->active_selections;
        }
        if(key == input.keys.accept) {
            if(dialog->selection_funcs[selection] != NULL) {
                dialog->selection_funcs[selection]();
            }

            finished = 1;
        }
    }
}

void menu_dialog_run(menu_dialog_t *_dialog) {
    selection = 0;
    finished = 0;
    dialog = _dialog;

    while(!finished && (moo.state & MOO_RUNNING_BIT)) {
        draw();
        sys_handle_events(dialog_input_event);
    }
}
