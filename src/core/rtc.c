#include "rtc.h"
#include <string.h>
#include "cpu.h"
#include "mbc.h"

#define S 0x00
#define M 0x01
#define H 0x02
#define DL 0x03
#define DH 0x04

rtc_t rtc;

static void rtc_next_day() {
    u16 d = rtc.ticking[DL] | ((rtc.ticking[DH] & 0x01) << 8);
    d++;
    if(d >= 0x200) { // rtc day overflow
        d = 0;
        rtc.ticking[DH] |= 0x80;
    }
}

static void rtc_tick(u8 r) {
    static u8 *regs[] = {&rtc.ticking[S], &rtc.ticking[M], &rtc.ticking[H]};
    if(r < 3) {
        (*regs[r])++;
        if(*regs[r] > 59) {
            *regs[r] = 0;
            rtc_tick(*regs[r+1]);
        }
    }
    else {
        rtc_next_day();
    }
}

void rtc_reset() {
    memset(&rtc, 0x00, sizeof(rtc));
}

void rtc_step(u8 mcs) {
    if(mbc.type == 3) {
        rtc.cc += mcs;
        if(rtc.ticking[DH] & 0x40 || rtc.cc < cpu.mcs_per_second) { // Halt bit set or next tick not yet reached
            return;
        }
        else {
            rtc.cc -= cpu.mcs_per_second;
            rtc_tick(0);
        }
    }
}

void rtc_map_register(u8 val) {
    rtc.mapped = val - 0x08;
}

void rtc_latch(u8 val) {
    switch(val) {
        case 0x00: rtc.prelatched = 1; break;
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

