#include "include.h"

static u8 rom_bankcount(u8 ref) {
    if(ref <= 6) {
        return ((u8[]){2, 4, 8, 16, 32, 64, 128})[ref];
    }
    else if(ref >= 0x52 && ref <= 0x54) {
        return ((u8[]){72, 80, 96})[ref - 0x52];
    }
    else {
        return 0;
    }
}

static uint mbc_type(u8 ref) {

}

bool load_rom(u8 *data, uint datasize) {
    assert_corrupt(datasize > 0x014F, "ROM too small for header");

    mbc.romsize = rom_bankcount(data[0x0148]);
    assert_corrupt(mbc.romsize == 0, "Unknown romsize ref");
    assert_corrupt(mbc.romsize * 0x4000 == datasize, "Datasize doesn't match rom-internally specified size");
    mbc.type = mbc_type(data[0x0148]);

    rom.banks = realloc(rom.banks, mbc.romsize * sizeof(*rom.banks));
    memcpy(rom.banks, data, datasize);

    return true;
}


