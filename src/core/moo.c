#include "moo.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
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
#include "load.h"
#include "serial.h"
#include "sys/sys.h"
#include "menu/menu.h"
#include "util/performance.h"
#include "util/framerate.h"
#include "util/config.h"
#include "util/card.h"
#include "sound.h"


moo_t moo;

static void store_rompath() {
    FILE *f = fopen("lastrom.txt", "w");
    fwrite(sys.rompath, 1, strlen(sys.rompath), f);
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
}

void moo_begin() {
    moo.state |= MOO_ROM_RUNNING_BIT;
    rtc_begin();
    sound_begin();
    lcd_begin();
    timers_begin();
    sys_begin();
    framerate_begin();
}

void moo_continue() {
    moo.state |= MOO_ROM_RUNNING_BIT;
    sys_continue();
}

void moo_pause() {
    moo.state ^= MOO_ROM_RUNNING_BIT;
    sys_pause();
}

void moo_quit() {
    moo.state &= ~MOO_RUNNING_BIT;

    if(moo.state & MOO_ROM_LOADED_BIT) {
        card_save();
    }
}

void moo_load_rom(const char *path) {
    if(moo.state & MOO_ROM_LOADED_BIT) {
        card_save();
    }
    if(path != sys.rompath) {
        strcpy(sys.rompath, path);
    }

    printf("Loading ROM '%s'\n", sys.rompath);

    moo_reset();
    load_rom();

    if(moo.state & MOO_ROM_LOADED_BIT) {
        moo_load_rom_config();
        store_rompath();
    }
}

void moo_load_rom_config() {
    char configpath[sizeof(sys.rompath) + 5];
    sprintf(configpath, "%s.conf", sys.rompath);

    if(config_load(configpath)) {
        config_load("global.conf");
    }
}

void moo_set_hw(int hw) {
    moo.hw = hw;
    if(!moo.hw == CGB_HW) {
        moo.mode = NON_CGB_MODE;
    }
    //serial_update_internal_period();
}

void moo_step_hw(int mcs) {
    assert(mcs <= 10);

    int nfcs;
    if(mcs == 0) {
        return;
    }
    if(cpu.freq == DOUBLE_CPU_FREQ) {
        nfcs = (mcs + cpu.remainder) / 2;
        cpu.remainder = (mcs + cpu.remainder) % 2;
    }
    else {
        nfcs = mcs;
    }

    cpu.dbg_mcs += mcs;
    cpu.dbg_nfcs += nfcs;


    hw_step(mcs);


  //  timers_step(nfcs, mcs);
    //lcd_step(nfcs);
    //rtc_step(nfcs);
    sound_step(nfcs);
    //serial_step();


    sys.invoke_cc += mcs;
}

static void moo_cycle(int num) {
    unsigned int t;

    sys.invoke_cc = 0;
    for(t = 0; t < num; t++) {
        if(cpu.halted) {
            if(ints_handle_standby()) {
                cpu.halted = 0;
            }
            moo_step_hw(1);
        }
        else {
            u8 mcs = cpu_step();
            moo_step_hw(mcs);
        }
    }
}

void moo_run() {
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

    va_end(args);
}

void moo_fatalf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    fprintf(stderr, "FATAL: "); vfprintf(stderr, format, args); fprintf(stderr, "\n");

    moo_quit();

    va_end(args);
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

