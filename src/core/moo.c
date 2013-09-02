#include "moo.h"
#include <stdio.h>
#include "defines.h"
#include "lcd.h"
#include "rtc.h"
#include "serial.h"
#include "cpu.h"
#include "mem.h"
#include "timers.h"
#include "joy.h"
#include "ints.h"
#include "loader.h"
#include "serial.h"
#include "sys/sys.h"
#include "sys/menu/menu.h"
#include "sound.h"

timingstats_t timing[2][256];

moo_t moo;
int core;
int cc = 0;

void moo_init() {
    moo.hw = CGB_HW;
    moo.mode = CGB_MODE;

    mem_init();
    cpu_init();
    joy_init();
    sound_init();

    memset(timing, 0x00, sizeof(timingstats_t));
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

}

void moo_continue() {

}

void moo_pause() {

}

void moo_load_rom(u8 *data, size_t size) {
    moo_reset();
    load_rom(data, size);
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
    while(sys.state & MOO_RUNNING_BIT) {
        if(sys.state & MOO_ROM_RUNNING_BIT) {
            moo_cycle(sys.quantum_length);
            sys_invoke();
            sys.invoke_cc = 0;
        }
        else {
            if(sys.state & MOO_ROM_LOADED_BIT) {
                moo_pause();
            }

            switch(menu_run()) {
                case MENU_CONTINUE:
                    moo_continue();
                break;
                case MENU_NEW_ROM:
                case MENU_STATE_LOADED:
                    moo_begin();
                break;
            }
        }
    }
}

