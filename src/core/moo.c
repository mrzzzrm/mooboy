#include "moo.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <unistd.h>
#include "defines.h"
#include "lcd.h"
#include "rtc.h"
#include "serial.h"
#include "cpu.h"
#include "hw.h"
#include "mem.h"
#include "timers.h"
#include "joy.h"
#include "ints.h"
#include "mbc.h"
#include "load.h"
#include "serial.h"
#include "sys/sys.h"
#include "menu/menu.h"
#include "util/performance.h"
#include "util/framerate.h"
#include "util/config.h"
#include "util/card.h"
#include "util/state.h"
#include "util/pathes.h"
#include "util/speed.h"
#include "sound.h"


moo_t moo;


static void on_rom_over() {
    state_save(pathes.continue_state);
    card_save();
}

static void store_rompath() {
    FILE *f = fopen("lastrom.txt", "w");
    fprintf(f, "%s", pathes.rom);
    fclose(f);
}

void moo_init() {
    moo_set_hw(CGB_HW);

    sound_init();
    //serial_init();

    config_default();
}

void moo_close() {
    sound_close();
    pathes_close();
    //serial_close();
}

void moo_reset() {
    sys_reset();
    mem_reset();
    hw_reset();
    cpu_reset();
    lcd_reset();
    timers_reset();
    rtc_reset();
    sound_reset();
    joy_reset();
    //serial_reset();

    performance_reset();
    framerate_reset();
    speed_reset();
}

void moo_begin() {
    moo.state |= MOO_ROM_RUNNING_BIT;

    rtc_begin();
    sound_begin();
    lcd_begin();
    timers_begin();
    sys_begin();
    framerate_begin();
    speed_begin();
}

void moo_continue() {
    if((moo.state & MOO_ROM_LOADED_BIT) && (~moo.state & MOO_ERROR_BIT)) {
        moo.state |= MOO_ROM_RUNNING_BIT;
        sys_continue();
    }
}

void moo_restart_rom() {
    card_save();
    moo_reset();
    moo_load_rom_config();
    card_load();
    moo_begin();
}

void moo_pause() {
    moo.state ^= MOO_ROM_RUNNING_BIT;
    sys_pause();
}

void moo_quit() {
    moo.state &= ~MOO_RUNNING_BIT;

    if(moo.state & MOO_ROM_LOADED_BIT) {
        on_rom_over();
    }
}

void moo_paused_do(void (*func)()) {
    moo_pause();
    func();
    moo_continue();
}

static void warn_rtc_sav_conflict() {
    menu_warn_rtc_sav_conflict();
    sys.auto_continue = SYS_AUTO_CONTINUE_NO;
    sys.warned_rtc_sav_conflict = 1;
    config_save_local();
}

void moo_load_rom(const char *path) {
    if(moo.state & MOO_ROM_LOADED_BIT) {
        on_rom_over();
    }
    if(path != pathes.rom) {
        pathes_rompath(path);
    }

    printf("Loading ROM '%s'\n", pathes.rom);

    moo_reset();
    moo_load_rom_config();
    load_rom();

    if(~moo.state & MOO_ROM_LOADED_BIT) {
        printf("Failed to load ROM\n");
        return;
    }

    store_rompath();
    moo_begin();

    if(mbc.has_rtc && !sys.warned_rtc_sav_conflict) {
        moo_paused_do(warn_rtc_sav_conflict);
    }

    if(access(pathes.continue_state, R_OK) == 0) {
        switch(sys.auto_continue) {
            case SYS_AUTO_CONTINUE_YES:
                state_load(pathes.continue_state);
            break;
            case SYS_AUTO_CONTINUE_ASK:
                moo_paused_do(menu_continue);
            break;
        }
    }
}

void moo_load_rom_config() {
    if(!config_load_local()) {
        if(!config_load_global()) {
            config_default();
        }
    }
}

void moo_set_hw(int hw) {
    moo.hw = hw;
    if(moo.hw == DMG_HW) {
        moo.mode = NON_CGB_MODE;
    }
    //serial_update_internal_period();
}

static void moo_cycle(int num) {
    unsigned int t;

    sys.invoke_cc = 0;
    for(t = 0; t < num; t++) {
        if(cpu.halted) {
            if(ints_handle_standby()) {
                cpu.halted = 0;
            }
            hw_step(1);
        }
        else {
            u8 mcs = cpu_step();
            hw_step(mcs);
        }
    }
}

void moo_main() {
    while(moo.state & MOO_RUNNING_BIT) {
        if(moo.state & MOO_ERROR_BIT){
            menu_error();
        }
        else if(moo.state & MOO_ROM_RUNNING_BIT) {
            moo_cycle(sys.quantum_length);
            sys_invoke();
        }
        else {
            menu_run();
        }
    }
}

void moo_notifyf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    if(moo.error != NULL) {
        free(moo.error);
    }

    moo.error = malloc(sizeof(*moo.error));
    vsnprintf(moo.error->text, sizeof(moo.error->text), format, args);

    fprintf(stderr, "NOTIFICATION: "); vfprintf(stderr, format, args); fprintf(stderr, "\n");

    moo.state |= MOO_ERROR_BIT;
    moo_pause();

    va_end(args);
}

void moo_errorf(const char *format, ...) {
    va_list args;
    va_start(args, format);

    if(moo.error != NULL) {
        free(moo.error);
    }

    moo.error = malloc(sizeof(*moo.error));
    vsnprintf(moo.error->text, sizeof(moo.error->text), format, args);

    fprintf(stderr, "ERROR: "); vfprintf(stderr, format, args); fprintf(stderr, "\n");

    moo.state |= MOO_ERROR_BIT;
    moo.state &= ~MOO_ROM_RUNNING_BIT;
    moo.state &= ~MOO_ROM_LOADED_BIT;

    moo_reset();

    va_end(args);
}

void moo_fatalf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "FATAL: "); vfprintf(stderr, format, args); fprintf(stderr, "\n");

    va_end(args);

    exit(1);
}

void moo_clear_error() {
    if(moo.error != NULL) {
        free(moo.error);
        moo.error = NULL;
    }
    moo.state &= ~MOO_ERROR_BIT;

    if(moo.state & MOO_ROM_LOADED_BIT) {
        moo_continue();
    }
}

