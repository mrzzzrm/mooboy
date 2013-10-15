#include "dialogs.h"
#include "dialog.h"
#include "core/moo.h"
#include "util/pathes.h"
#include "util/state.h"

static menu_dialog_t *continue_dialog;
static menu_dialog_t *warn_rtc_sav_conflict_dialog;
static menu_dialog_t *error_dialog;

static void load_continue_state() {
    state_load(pathes.continue_state);
}

void menu_dialogs_init() {
    continue_dialog = menu_dialog_new("Continue right where you left?", load_continue_state, moo_continue);
    warn_rtc_sav_conflict_dialog = menu_dialog_new_message("This ROM uses a cardrige clock. mooBoy can emulate this clock even when it is not running. Don't use savestates if you want to take advantage of this, or disable auto-RTC in the options.");
    error_dialog = NULL;
}

void menu_dialogs_close() {
    menu_dialog_free(continue_dialog);
    menu_dialog_free(warn_rtc_sav_conflict_dialog);
}

void menu_continue() {
    menu_dialog_run(continue_dialog);
}

void menu_warn_rtc_sav_conflict() {
    menu_dialog_run(warn_rtc_sav_conflict_dialog);
}

void menu_error() {
    error_dialog = menu_dialog_new_message(moo.error->text);
    menu_dialog_run(error_dialog);
    menu_dialog_free(error_dialog);
    moo_clear_error();
}

