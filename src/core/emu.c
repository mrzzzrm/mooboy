#include "emu.h"
#include <stdio.h>
#include "defines.h"
#include "lcd.h"
#include "rtc.h"
#include "cpu.h"
#include "mem.h"
#include "timers.h"
#include "joy.h"
#include "ints.h"
#include "loader.h"
#include "sys/sys.h"
#include "sound.h"

#define QUANTUM 1000

emu_t emu;

static unsigned int t;

void emu_init() {
    emu.hw = CGB_HW;
    emu.mode = CGB_MODE;

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

void emu_set_hw(int hw) {
    emu.hw = hw;
}

void emu_run() {
    debug_init();

    t = 0;
    for(;;) {
        for(; t < QUANTUM; t++) {
//            debug_update();
//            debug_before();

            u8 mcs = cpu_step();
            lcd_step(cpu.step_nf_cycles);
            rtc_step(mcs);
            timers_step(mcs);
            sound_step();

//            debug_after();
        }
        t = 0;
        sys_invoke();
    }
}

