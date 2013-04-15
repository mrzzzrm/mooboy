#include "mbc.h"
#include <assert.h>
#include <stdio.h>
#include "rtc.h"
#include "mem.h"

#define MBC3_MAP_RAM 0x00
#define MBC3_MAP_RTC 0x01


mbc_t mbc;
mbc1_t mbc1;
mbc3_t mbc3;
mbc5_t mbc5;


static void mbc0_lower_write(u16 adr, u8 val) {
    // Nothing to do here
}

static void mbc1_lower_write(u16 adr, u8 val) {
    switch(adr >> 12) {
        case 0x0: case 0x1: // Enable/Disable external RAM
        break;
        case 0x2: case 0x3: // Select lower ROM bank bits
            mbc1.rombank &= 0xE0;
            mbc1.rombank |= (val & 0x1F) == 0x00 ? 0x01 : (val & 0x1F);
            if(mbc1.rombank >= card.romsize) {
                printf("Ignored setting of ROM-bank %i\n", mbc1.rombank);
                mbc1.rombank = 1;
            }
            mbc.rombank = card.rombanks[mbc1.rombank];
        break;
        case 0x4: case 0x5:
            if(mbc1.mode == 0) { // Upper ROM bank bits
                mbc1.mode &= 0x1F;
                mbc1.mode |= (val & 0x03) << 5;
                if(mbc1.rombank >= card.romsize) {
                printf("Ignored setting of ROM-bank %i\n", mbc1.rombank);
                    mbc1.rombank = 1;
                }
                mbc.rombank = card.rombanks[mbc1.rombank];
            }
            else { // Cartridge RAM bits
                mbc.srambank = card.srambanks[val & 0x03];
            }
        break;
        case 0x6: case 0x7: // RAM or ROM banking mode
            mbc1.mode = val & 0x01;
        break;
        default:
            assert(0);
    }
}

static void mbc2_lower_write(u16 adr, u8 val) {
    switch(adr >> 12) {
        case 0: case 1: // RAM en/disable
        break;
        case 2: case 3:
            if(adr & 0x10) { // Least significant bit of upper address byte has to be zero.
                mbc.rombank = card.rombanks[val & 0x0F];
            }
        break;
        default:
            assert(0);
    }
}

static void mbc3_lower_write(u16 adr, u8 val) {
    switch(adr >> 12) {
        case 0: case 1: // RAM and RTC en/disable
            //exit(3);
        break;
        case 2: case 3: // Select ROM bank
            mbc.rombank = card.rombanks[(val & 0x7F) == 0 ? 0x01 : (val & 0x7F)];
        break;
        case 4: case 5: // Select RAM bank or RTC register
            switch(val) {
                case 0x00: case 0x01: case 0x02: case 0x03:
                    mbc.srambank = card.srambanks[val];
                    mbc3.mode = MBC3_MAP_RAM;
                break;
                case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C:
                    rtc_map_register(val);
                    mbc3.mode = MBC3_MAP_RTC;
                break;
                default:;
            }
        break;
        case 6: case 7: // Latch Clock Data (First write 0x00, then 0x01)
            rtc_latch(val);
        break;
        default:
            assert(0);
    }
}

static void mbc5_lower_write(u16 adr, u8 val) {
    switch(adr >> 12) {
        case 0: case 1: // En/disable RAM
        break;
        case 2:  // Select lower 8 bit of ROM bank
            mbc5.rombank &= 0xFF00;
            mbc5.rombank |= val;
            mbc.rombank = card.rombanks[mbc5.rombank];
        break;
        case 3: // Select upper 1 bit of ROM bank
            mbc5.rombank &= 0xFEFF;
            mbc5.rombank |= ((u16)val)<<8;
            mbc.rombank = card.rombanks[mbc5.rombank];
        break;
        case 4: case 5:
            assert(val < 0x04);
            mbc.srambank = card.srambanks[val & 0x0F];
        break;
        default:
            printf("Suspecious write %.2X to %.4X\n", val, adr);
    }

    assert(mbc5.rombank < 0x1E0);
}

void mbc_set_type(u8 type) {
    switch(type) {
        case 0: mbc.lower_write_func = mbc0_lower_write; break;
        case 1: mbc.lower_write_func = mbc1_lower_write; break;
        case 2: mbc.lower_write_func = mbc2_lower_write; break;
        case 3: mbc.lower_write_func = mbc3_lower_write; break;
        case 5: mbc.lower_write_func = mbc5_lower_write; break;
        default:
            assert(0);
    }
    fprintf(stderr, "MBC-type is %i\n", (int)type);
}

u8 mbc_upper_read(u16 adr) {
    if(mbc.type == 3 && mbc3.mode == MBC3_MAP_RTC) {
        return rtc.latched[rtc.mapped];
    }
    else {
        adr -= 0xA000;
        return mbc.srambank[adr];
    }
}

void mbc_lower_write(u16 adr, u8 val) {
    mbc.lower_write_func(adr, val);
}

void mbc_upper_write(u16 adr, u8 val) {
    if(mbc.type == 3 && mbc3.mode == MBC3_MAP_RTC) {
        rtc_write(val);
    }
    else {
        adr -= 0xA000;
        mbc.srambank[adr] = val;
    }
}

