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

void emu_step_hw(u8 mcs) {
    u32 nfcc;

    nfcc = cpu.nfcc;
    cpu.cc += mcs;
    cpu.step_sf_cycles = mcs;

    if(cpu.freq == DOUBLE_CPU_FREQ) {
        cpu.dfcc += cpu.step_sf_cycles;
    }
    cpu.nfcc = cpu.cc - (cpu.dfcc >> 1);
    cpu.step_nf_cycles = cpu.nfcc - nfcc;

    timers_step();
    lcd_step();
    rtc_step();
    sound_step();
}

void emu_run() {
    debug_init();
    sys_begin();

    for(;;) {
        unsigned int t;
        for(t = 0; t < QUANTUM; t++) {
//            debug_update();
//            debug_before();
            if(cpu.halted) {
                if(ints_handle_standby()) {
                    cpu.halted = 0;
                }

                emu_step_hw(1);
            }
            else {
                u8 mcs = cpu_step();
                emu_step_hw(mcs);
            }
//            debug_after();
        }
        if(!sys_invoke()) {
            break;
        }
    }
}

