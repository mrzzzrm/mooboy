#include "mbc.h"

mbc_t mbc;

static void mbc1_control(u16 adr, u8 val) {
    switch(adr>>12) {
        case 0x0: case 0x1: // Enable/Disable external RAM
        break;
        case 0x2: case 0x3: // Select lower ROM bank bits
            u8 lower = val & 0x1F;
        break;
        case 0x4: case 0x5:
            if(mbc1.mode == 0) {
                u8 upper = val & 0x03;
            }
            else {
                mbc.xrambank = ram.xbank[val & 0x03];
            }
        break;
        case 0x6: case 0x7:
            mbc1.mode = val & 0x01;
        break;
    }
}

static void mbc2_control(u16 adr, u8 val) {
    switch(adr>>12) {

    }
}

static void mbc3_control(u16 adr, u8 val) {

}

static void mbc5_control(u16 adr, u8 val) {

}

void mbc_control(u16 adr, u8 val) {
    switch(mbc.type) {
        case 0: break;
        case 1: mbc1_control(adr, val); break;
        case 2: mbc2_control(adr, val); break;
        case 3: mbc3_control(adr, val); break;
        case 5: mbc5_control(adr, val); break;

        default: assert(0); break;
    }
}
