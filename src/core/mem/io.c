#include "io.h"
#include "io/lcd.h"
#include "_assert.h"
#include "util/defines.h"

u8 io_read(u16 adr) {
    u8 r = adr -  0xFF00;

    switch(r) {
        case 0x40: return lcd.c; break;
        case 0x41: return lcd.stat; break;
        case 0x42: return lcd.scy; break;
        case 0x43: return lcd.scx; break;
        case 0x44: return lcd.ly; break;
        case 0x45: assert(0); /* TODO: */ break;
        case 0x46: assert(0); break;
        case 0x47: return lcd.bgp; break;
        case 0x48: return lcd.obp0; break;
        case 0x49: return lcd.obp1; break;
        case 0x4A: return lcd.wy; break;
        case 0x4B: return lcd.wx; break;

        /* TODO: CGB Mode */
        case 0x51: assert(0); break;
        case 0x52: assert(0); break;
        case 0x53: assert(0); break;
        case 0x55: assert(0); break;
        case 0x54: assert(0); break;

        /* TODO: CGB Mode */
        case 0x68: assert(0); break;
        case 0x69: assert(0); break;
        case 0x6A: assert(0); break;
        case 0x6B: assert(0); break;

        default:
            assert_corrupt(0, "Illegal or unknown IO register write access");
    }

    return 0; // Avoids warnings;
}

void io_write(u16 adr, u8 val) {
    u8 r = adr -  0xFF00;

    switch(r) {
        case 0x40: lcd.c = val; break;
        case 0x41: lcd.stat = val; break;
        case 0x42: lcd.scy = val; break;
        case 0x43: lcd.scx = val; break;
        case 0x44: assert(0); break;
        case 0x45: assert(0); /* TODO */  break;
        case 0x46: lcd_dma(val); break;
        case 0x47: lcd.bgp = val; break;
        case 0x48: lcd.obp0 = val; break;
        case 0x49: lcd.obp1 = val; break;
        case 0x4A: lcd.wy = val; break;
        case 0x4B: lcd.wx = val; break;

        /* TODO: CGB Mode */
        case 0x51: assert(0); break;
        case 0x52: assert(0); break;
        case 0x53: assert(0); break;
        case 0x55: assert(0); break;
        case 0x54: assert(0); break;

        /* TODO: CGB Mode */
        case 0x68: assert(0); break;
        case 0x69: assert(0); break;
        case 0x6A: assert(0); break;
        case 0x6B: assert(0); break;

        default:
            assert_corrupt(0, "Illegal or unknown IO register write access");
    }
}
