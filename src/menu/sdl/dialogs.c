#include "dialogs.h"
#include "dialog.h"
#include "core/moo.h"
#include "util/continue.h"

static menu_dialog_t *continue_dialog;
static menu_dialog_t *warn_rtc_sav_conflict_dialog;

void menu_dialogs_init() {
    continue_dialog = menu_dialog_new("Continue right where you left?", continue_state_load, moo_continue);
    warn_rtc_sav_conflict_dialog = menu_dialog_new_message("This ROM uses a cardrige clock. mooBoy can emulate this clock even when it is not running. Don't use savestates if you want to take advantage of this, or disable auto-RTC in the options.");
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
