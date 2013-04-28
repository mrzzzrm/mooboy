#include "loader.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "mem.h"
#include "emu.h"
#include "mbc.h"

static u8 rom_bankcount(u8 ref) {
    if(ref <= 8) {
        return 2 << ref;
    }
    else if(ref >= 0x52 && ref <= 0x54) {
        return ((u8[]){72, 80, 96})[ref - 0x52];
    }
    else {
        fprintf(stderr, "ROM-bankcount ref unknown: %.2X\n", ref);
        return 0;
    }
}

static void init_mode(u8 ref) {
    if(ref & 0x80) {
        emu.mode = CGB_MODE;
    }
    else {
        emu.mode = NON_CGB_MODE;
    }
}

static void init_mbc(u8 ref) {
    u8 ln = ref & 0x0F;
    u8 hn = ref >> 4;

    mbc.has_battery = 0;
    mbc.has_rtc = 0;

    if(hn == 0x00) {
        switch(ln) {
            case 0x0:
                mbc_set_type(0);
            break;
            case 0x1: case 0x2: case 0x3:
                mbc_set_type(1);
                if(ln == 0x03) {
                    mbc.has_battery = 1;
                }
            break;
            case 0x5: case 0x6:
                mbc_set_type(2);
                if(ln == 0x06) {
                    mbc.has_battery = 1;
                }
            break;
            case 0x8: case 0x9:
                assert(0); // TODO
            break;
            case 0xB: case 0xC: case 0xD:
                assert(0); // TODO
            break;
            case 0xF:
                mbc_set_type(3);
            break;
            default:
                assert(0);
        }
    }
    else if(hn == 0x01) {
        switch(ln) {
            case 0x0: case 0x1: case 0x2:
            case 0x3:
                mbc_set_type(3);
            break;
            case 0x5: case 0x6: case 0x7:
                mbc_set_type(4);
            break;
            case 0x9: case 0xA: case 0xB:
            case 0xC: case 0xD: case 0xE:
                mbc_set_type(5);
            break;
            default:
                assert(0);
        }
    }
    else {
        mbc_set_type(0);
    }
}

static void init_rombanks(u8 ref, u8 *data, u32 datasize) {
    card.romsize = rom_bankcount(ref);
        assert(card.romsize != 0);
        assert(card.romsize * 0x4000 == datasize);
    card.rombanks = realloc(card.rombanks, card.romsize * sizeof(*card.rombanks));
    memcpy(card.rombanks, data, datasize);

    fprintf(stderr, "ROM-size set to %d banks\n", card.romsize);
}

static void init_xrambanks(u8 ref) {
    switch(ref) {
        case 0x00:
            card.sramsize = 1;
            card.srambanks = realloc(card.srambanks, sizeof(*card.srambanks) * card.sramsize);
        break;
        case 0x01: case 0x02:
            card.sramsize = 1;
            card.srambanks = realloc(card.srambanks, sizeof(*card.srambanks) * card.sramsize);
        break;
        case 0x03:
            card.sramsize = 4;
            card.srambanks = realloc(card.srambanks, sizeof(*card.srambanks) * card.sramsize);
        break;
        default:
            assert(0);
    }

    fprintf(stderr, "Cardridge-RAM set to %d banks\n", card.sramsize);
}

void load_rom(u8 *data, uint datasize) {
    assert(datasize > 0x014F);

    init_mode(data[0x0143]);
    init_mbc(data[0x0147]);
    init_rombanks(data[0x0148], data, datasize);
    init_xrambanks(data[0x0149]);

    mbc.rombank = card.rombanks[1];
    mbc.srambank = card.srambanks[0];
}


