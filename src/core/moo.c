#include "moo.h"
#include <stdio.h>
#include <string.h>
#include "defines.h"
#include "lcd.h"
#include "rtc.h"
#include "serial.h"
#include "cpu.h"
#include "mem.h"
#include "timers.h"
#include "joy.h"
#include "ints.h"
#include "load.h"
#include "serial.h"
#include "sys/sys.h"
#include "menu/menu.h"
#include "util/config.h"
#include "util/card.h"
#include "sound.h"


moo_t moo;

void moo_init() {
    moo_set_hw(CGB_HW);
    moo.mode = CGB_MODE;

    mem_init();
    cpu_init();
    joy_init();
    sound_init();
    //serial_init();
}

void moo_close() {
    mem_close();
    sound_close();
}

void moo_reset() {
    sys.ticks = 0;

    sys_reset();
    mem_reset();
    cpu_reset();
    lcd_reset();
    timers_reset();
    rtc_reset();
    sound_reset();
    //serial_reset();
}

void moo_begin() {
    moo.state |= MOO_ROM_RUNNING_BIT;
    sys_play_audio(sys.sound_on);
}

void moo_continue() {
    moo.state |= MOO_ROM_RUNNING_BIT;
    sys_play_audio(sys.sound_on);
}

void moo_pause() {
    moo.state ^= MOO_ROM_RUNNING_BIT;
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
    strcpy(sys.rompath, path);

    moo_reset();
    load_rom();

    if(moo.state & MOO_ROM_LOADED_BIT) {
        moo_load_rom_config();
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
    //serial_update_internal_period();
}

void moo_step_hw(int mcs) {
    int nfcs;
    static int rest = 0;

    if(mcs == 0) {
        return;
    }

    if(cpu.freq == DOUBLE_CPU_FREQ) {
        nfcs = (mcs+rest)/2;
        rest = (mcs+rest)%2;
    }
    else {
        nfcs = mcs;
    }

    timers_step(nfcs, mcs);
    lcd_step(nfcs);
    rtc_step(nfcs);
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
        if(moo.state & MOO_ROM_RUNNING_BIT) {
            moo_cycle(sys.quantum_length);
            sys_invoke();
        }
        else {
            menu_run();
        }
    }
}

