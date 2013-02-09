#include "loader.h"
#include "_assert.h"
#include "mem.h"
#include "mem/mbc.h"

static u8 rom_bankcount(u8 ref) {
    if(ref <= 8) {
        return 2 << ref;
    }
    else if(ref >= 0x52 && ref <= 0x54) {
        return ((u8[]){72, 80, 96})[ref - 0x52];
    }
    else {
        fprintf(stderr, "ROM-bankcount ref: %.2X\n", ref);
        return 0;
    }
}

static void init_mbc(u8 ref) {
    u8 ln = ref & 0x0F;
    u8 hn = ref >> 4;

    if(hn == 0x00) {
        switch(ln) {
            case 0x0:
                mbc_set_type(0);
            break;
            case 0x1: case 0x2: case 0x3:
                mbc_set_type(1);
            break;
            case 0x5: case 0x6:
                mbc_set_type(2);
            break;
            case 0x8: case 0x9:
                // TODO: Implement
                assert_unsupported(1, "ROM + RAM (+ Battery) MBC not yet supported");
            break;
            case 0xB: case 0xC: case 0xD:
                // TODO: Implement
                assert_unsupported(1, "MMM01 MBC not yet supported");
            break;
            case 0xF:
                mbc_set_type(3);
            break;
            default:
                assert_corrupt(1, "No such MBC - type");
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
                assert_corrupt(1, "No such MBC - type");
        }
    }
    else {
        mbc_set_type(0);
    }
}

static void init_rombanks(u8 ref, u8 *data, u32 datasize) {
    mbc.romsize = rom_bankcount(ref);
        assert_corrupt(mbc.romsize != 0, "Unknown romsize ref");
        assert_corrupt(mbc.romsize * 0x4000 == datasize, "Datasize doesn't match rom-internally specified size");
    rom.banks = realloc(rom.banks, mbc.romsize * sizeof(*rom.banks));
    memcpy(rom.banks, data, datasize);
}

static void init_xrambanks(u8 ref) {
    switch(ref) {
        case 0x00:
            ram.xbanks = realloc(ram.xbanks, sizeof(*ram.xbanks) * 1);
        break;
        case 0x01: case 0x02:
            mbc.ramsize = 1;
            ram.xbanks = realloc(ram.xbanks, sizeof(*ram.xbanks) * mbc.ramsize);
        break;
        case 0x03:
            mbc.ramsize = 4;
            ram.xbanks = realloc(ram.xbanks, sizeof(*ram.xbanks) * mbc.ramsize);
        break;
        default:
            assert_corrupt(1, "No such RAM size");
    }
}

bool load_rom(u8 *data, uint datasize) {
    assert_corrupt(datasize > 0x014F, "ROM too small for header");

    init_mbc(data[0x0147]);
    init_rombanks(data[0x0148], data, datasize);
    init_xrambanks(data[0x0149]);

    return true;
}


