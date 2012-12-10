#include "mbc.h"

mbc_t mbc;


static struct {
    u8 mode;
    u8 rombank;
} mbc1;

static struct {
    u16 rombank;
} mbc5;


static void mbc0_control(u16 adr, u8 val) {
    // Nothing to do here
}

static void mbc1_control(u16 adr, u8 val) {
    switch(adr>>12) {
        case 0x0: case 0x1: // Enable/Disable external RAM
        break;
        case 0x2: case 0x3: // Select lower ROM bank bits
            mbc1.rombank &= 0xE0;
            mbc1.rombank |= (val & 0x1F) == 0 ? 0x01 : (val & 0x1F);
            mbc.rombank = rom.banks[mbc1.rombank];
        break;
        case 0x4: case 0x5:
            if(mbc1.mode == 0) { // Upper ROM bank bits
                mbc1.mode &= 0x1F;
                mbc1.mode |= (val & 0x03) << 5;
                mbc.rombank = rom.banks[mbc1.rombank];
            }
            else { // Cartridge RAM bits
                mbc.xrambank = ram.xbank[val & 0x03];
            }
        break;
        case 0x6: case 0x7: // RAM or ROM banking mode
            mbc1.mode = val & 0x01;
        break;
    }
}

static void mbc2_control(u16 adr, u8 val) {
    switch(adr>>12) {
        case 0: case 1: // RAM en/disable
        break;
        case 2: case 3:
            if(adr & 0x10) { // Least significant bit of upper address byte has to be zero.
                mbc.rombank = rom.banks[val & 0x0F];
            }
        break;
    }
}

static void mbc3_control(u16 adr, u8 val) {
    switch(adr>>12) {
        case 0: case 1: // RAM and RTC en/disable
        break;
        case 2: case 3: // Select ROM bank
            mbc.rombank = rom.banks[(val & 0x7F) == 0 ? 0x01 : (val & 0x7F)];
        break;
        case 4: case 5: // TODO: Select RAM bank or RTC register
        break;
        case 6: case 7: // TODO: Latch Clock Data
        break
    }
}

static void mbc5_control(u16 adr, u8 val) {
    switch(adr>>12) {
        case 0: case 1: // En/disable RAM
        break;
        case 2: case 3: // Select lower 8 bit of ROM bank
            mbc5.rombank &= 0xFF00;
            mbc5.rombank |= val;
            mbc.rombank = rom.banks[mbc5.rombank];
        break;
        case 4: case 5: // Select upper 1 bit of ROM bank
            mbc5.rombank &= 0x00FF;
            mbc5.rombank |= (val&0x01) << 8;
            mbc.rombank = rom.banks[mbc5.rombank];
        break;
    }
}

