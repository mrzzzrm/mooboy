#ifndef MENU_DIALOG_H
#define MENU_DIALOG_H

#include "util.h"

typedef struct {
    menu_word_string_t *prompt;
    menu_label_t *selections[2];
    void (*selection_funcs[2])();
    int active_selections;
} menu_dialog_t;

menu_dialog_t *menu_dialog_new(const char *question, void (*yes_func)(), void (*no_func)());
menu_dialog_t *menu_dialog_new_message(const char *message);
void menu_dialog_free(menu_dialog_t *dialog);
void menu_dialog_run(menu_dialog_t *dialog);

#endif
