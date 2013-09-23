#include "rtc.h"
#include <string.h>
#include <stdio.h>
#include "cpu.h"
#include "mbc.h"

#undef H

#define S 0x00
#define M 0x01
#define H 0x02
#define DL 0x03
#define DH 0x04

rtc_t rtc;

hw_event_t rtc_event;

static void rtc_next_day() {
    u16 d = rtc.ticking[DL] | ((rtc.ticking[DH] & 0x01) << 8);
    d++;
    if(d >= 0x200) { // rtc day overflow
        d = 0;
        rtc.ticking[DH] |= 0x80;
    }
}

static inline void rtc_tick(u8 r) {
    u8 *regs[] = {&rtc.ticking[S], &rtc.ticking[M], &rtc.ticking[H]};
    switch(r) {
        case 0: case 1:
            (*regs[r])++;
            if(*regs[r] > 59) {
                *regs[r] = 0;
                rtc_tick(r+1);
            }
        break;
        case 2:
            (*regs[r])++;
            if(*regs[r] > 24) {
                *regs[r] = 0;
                rtc_tick(r+1);
            }
        break;
        case 3:
            rtc_next_day();
    }
}

static void step(int mcs) {
    if(~rtc.ticking[DH] & 0x40) {
        rtc_tick(0);
    }
    hw_unschedule(&rtc_event); hw_schedule(&rtc_event, cpu.freq);
}

void rtc_reset() {
    memset(&rtc, 0x00, sizeof(rtc));

    rtc_event.callback = step;

#ifdef DEBUG
    sprintf(rtc_event.name, "rtc");
#endif
}

void rtc_begin() {
    if(mbc.type == 3) {
        hw_schedule(&rtc_event, cpu.freq);
    }
}

void rtc_step(int nfcs) {
    if(mbc.type == 3) {
        rtc.cc += nfcs;
        if(rtc.ticking[DH] & 0x40 || rtc.cc < NORMAL_CPU_FREQ) { // Halt bit set or next tick not yet reached
            return;
        }
        else {
            rtc.cc -= NORMAL_CPU_FREQ;
            rtc_tick(0);
        }
    }
}

void rtc_map_register(u8 val) {
    rtc.mapped = val - 0x08;
}

void rtc_latch(u8 val) {
    switch(val) {
        case 0x00:
            rtc.prelatched = 1;
        break;
        case 0x01:
            if(rtc.prelatched) {
                memcpy(rtc.latched, rtc.ticking, sizeof(rtc.latched));
            }
            rtc.prelatched = 0;
        break;

        default:
            rtc.prelatched = 0;
    }
}

void rtc_write(u8 val) {
    rtc.ticking[rtc.mapped] = val;
}

