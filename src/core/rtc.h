#ifndef CORE_RTC_H
#define CORE_RTC_H

#include "defines.h"

typedef struct {
    u8 latched[5];
    u8 ticking[5];
    u8 mapped;
    u8 prelatched;
    u32 cc;
} rtc_t;

extern rtc_t rtc;

void rtc_reset();
void rtc_begin();

void rtc_step(int nfcs);
void rtc_map_register(u8 val);
void rtc_latch(u8 val);
void rtc_write(u8 val);

#endif
