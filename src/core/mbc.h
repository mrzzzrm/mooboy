#ifndef CORE_MBC_H
#define CORE_MBC_H

#include "util/defines.h"

typedef struct mbc_s {
    u16 type;

    void (*lower_write_func)(u16, u8);

    u8 *rombank;
    u8 *srambank;

    u8 ram_selected;

    u8 has_rtc;
    u8 has_ram;
    u8 has_battery;
} mbc_t;


typedef struct {
    u8 mode;
    u8 rombank;
} mbc1_t;

typedef struct {
    u8 mode;
} mbc3_t;

typedef struct {
    u16 rombank;
} mbc5_t;

extern mbc_t mbc;
extern mbc1_t mbc1;
extern mbc3_t mbc3;
extern mbc5_t mbc5;

void mbc_set_type(u8 type);

u8 mbc_upper_read(u16 adr);
void mbc_lower_write(u16 adr, u8 val);
void mbc_upper_write(u16 adr, u8 val);

#endif
