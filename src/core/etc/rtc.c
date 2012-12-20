#include "rtc.h"
#include "cpu.h"

rtc_t rtc;



static void rtc_next_day() {
    u16 d = rtc.dl | ((rtc.dh & 0x01) << 8);
    d++;
    if(d >= 0x200) { // rtc day overflow
        d = 0;
        rtc.dh |= 0x80;
    }
}

static void rtc_tick(u8 r) {
    static u8 *regs[] = {&rtc.s, &rtc.m, &rtc.h};
    if(r < 3) {
        *regs[r]++;
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
    rtc.s = 0;
    rtc.m = 0;
    rtc.h = 0;
    rtc.dl = 0;
    rtc.dh = 0;
    rtc.mapped = 0;
    rtc.last_tick = 0;
}

void rtc_step() {
    u32 mcs = cpu.cc - rtc.last_tick;
    if(rtc.dh & 0x40 || mcs < cpu.mcs_per_second) { // Halt bit set or next tick not yet reached
        return;
    }

    rtc_tick(0);
}
