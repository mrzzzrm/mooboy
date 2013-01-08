#include "io.h"
#include "io/lcd.h"
#include "cpu.h"
#include "_assert.h"
#include "util/defines.h"
#include "debug.h"
#include "io/divt.h"
#include "io/tima.h"

u8 io_read(u16 adr) {
    u8 r = adr &  0x00FF;

    switch(r) {
        /* TODO: Joypad */
        case 0x00: break;
        case 0x01: break;
        case 0x02: break;

        case 0x04: return divt.ticks; break;
        case 0x05: return tima.ticks; break;
        case 0x06: return tima.mod; break;
        case 0x07: return tima.c; break;
        case 0x0F: return cpu.irq; break;

        /* TODO: Sound */
        case 0x10: break;
        case 0x11: break;
        case 0x12: break;
        case 0x13: break;
        case 0x14: break;
        case 0x16: break;
        case 0x17: break;
        case 0x18: break;
        case 0x19: break;
        case 0x1A: break;
        case 0x1B: break;
        case 0x1C: break;
        case 0x1D: break;
        case 0x1E: break;
        case 0x20: break;
        case 0x21: break;
        case 0x22: break;
        case 0x23: break;
        case 0x24: break;
        case 0x25: break;
        case 0x26: break;
        case 0x30: break;

        case 0x40: return lcd.c; break;
        case 0x41: return lcd.stat; break;
        case 0x42: return lcd.scy; break;
        case 0x43: return lcd.scx; break;
        case 0x44: return lcd.ly; break;
        case 0x45: return lcd.lyc; break;
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
    }

    return 0xFF; // Avoids warnings;
}

void io_write(u16 adr, u8 val) {
    u8 r = adr & 0x00FF;
    switch(r) {
        /* TODO: Joypad */
        case 0x00: break;
        case 0x01: break;
        case 0x02: break;

        case 0x04: divt.ticks = 0x00; break;
        case 0x05: tima.ticks = 0x00; break;
        case 0x06: tima.mod = val; break;
        case 0x07: tima.c = val; break;
        case 0x0F: cpu.irq = val; break;

        /* TODO: Sound */
        case 0x10: break;
        case 0x11: break;
        case 0x12: break;
        case 0x13: break;
        case 0x14: break;
        case 0x16: break;
        case 0x17: break;
        case 0x18: break;
        case 0x19: break;
        case 0x1A: break;
        case 0x1B: break;
        case 0x1C: break;
        case 0x1D: break;
        case 0x1E: break;
        case 0x20: break;
        case 0x21: break;
        case 0x22: break;
        case 0x23: break;
        case 0x24: break;
        case 0x25: break;
        case 0x26: break;
        case 0x30: break;

        case 0x40: lcd.c = val; break;
        case 0x41: lcd.stat = val; break;
        case 0x42: lcd.scy = val; break;
        case 0x43: lcd.scx = val; break;
        case 0x44: assert(0); break;
        case 0x45: lcd.lyc = val; /* TODO */  break;
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
    }
}
