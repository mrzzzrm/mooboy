#include "emu.h"
#include <stdio.h>
#include "defines.h"
#include "lcd.h"
#include "rtc.h"
#include "timers.h"
#include "joy.h"
#include "ints.h"
#include "debug/debug.h"
#include "loader.h"
#include "sys/sys.h"
#include "sound.h"

#define QUANTUM 1000

void emu_init() {
    mem_init();
    cpu_init();
    joy_init();
    sound_init();
}

void emu_close() {
    mem_close();
    sound_close();
}

void emu_reset() {
    mem_reset();
    cpu_reset();
    lcd_reset();
    timers_reset();
    rtc_reset();
    sound_reset();
}

void emu_load_rom(u8 *data, size_t size) {
    emu_reset();
    load_rom(data, size);
}

void emu_run_standby() {
    int c = 0;
    for(;;) {
        unsigned int t;
        for(t = 0; t < QUANTUM; t++) {
            debug_update();
            debug_before();

            if(ints_handle_standby()) {
                //printf("  > Been halted for %i\n", c);
                debug_after();
                return;
            }

            lcd_step();
            rtc_step(1);
            timers_step(1);
            sound_step();
            cpu.cc++;
            c++;

            debug_after();
        }
        sys_invoke();
    }
}

void emu_run() {
    debug_init();
    for(;;) {
        unsigned int t;
        for(t = 0; t < QUANTUM; t++) {
            debug_update();
            debug_before();

            ints_handle();
            u8 mcs = cpu_step();
            lcd_step();
            rtc_step(mcs);
            timers_step(mcs);
            sound_step();

            debug_after();
        }
        sys_invoke();
    }
}

