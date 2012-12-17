#ifndef ETC_RTC_H
#define ETC_RTC_H

    typedef struct {
        u8 s, m, h;
        u8 dl, dh;
        u8 mapped;
        u32 last_tick;
    } rtc_t;

    extern rtc_t rtc;

    void rtc_reset();
    void rtc_step();

#endif
