#ifndef MBC_H
#define MBC_H

#include "util/defines.h"

typedef struct mbc_s {
    u16 type;

    void (*lower_write_func)(u16, u8);

    u8 *rombank;
    u8 *srambank;

    u8 has_rtc;
    u8 has_battery;
} mbc_t;

extern mbc_t mbc;

void mbc_set_type(u8 type);

u8 mbc_upper_read(u16 adr);
void mbc_lower_write(u16 adr, u8 val);
void mbc_upper_write(u16 adr, u8 val);

#endif
