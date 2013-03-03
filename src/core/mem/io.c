#include "io.h"
#include "io/lcd.h"
#include "cpu.h"
#include "cpu/timers.h"
#include "cpu/defines.h"
#include "_assert.h"
#include "util/defines.h"
#include "debug/debug.h"
#include "io/joy.h"

u8 io_read(u16 adr) {
    u8 r = adr &  0x00FF;

    switch(r) {
        /* TODO: Joypad */
        case 0x00: return joy_read(); break;

        /* TODO: Serial */
        case 0x01: break;
        case 0x02: break;

        case 0x04: return cpu.div; break;
        case 0x05: return cpu.tima; break;
        case 0x06: return cpu.tma; break;
        case 0x07: return cpu.tac; break;
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
        case 0x46: return 0x00; break;
        case 0x47: return lcd.bgp; break;
        case 0x48: return lcd.obp0; break;
        case 0x49: return lcd.obp1; break;
        case 0x4A: return lcd.wy; break;
        case 0x4B: return lcd.wx; break;

        /* TODO: CGB Mode */
        case 0x51: break;
        case 0x52: break;
        case 0x53: break;
        case 0x55: break;
        case 0x54: break;

        /* TODO: CGB Mode */
        case 0x68: break;
        case 0x69: break;
        case 0x6A: break;
        case 0x6B: break;
    }

    return 0xFF; // Avoids nasty warnings, precious
}

void io_write(u16 adr, u8 val) {
    u8 r = adr & 0x00FF;
    switch(r) {
        /* TODO: Joypad */
        case 0x00: joy_select_col(val); break;
        case 0x01: break;
        case 0x02: break;

        case 0x04: cpu.div = 0x00; break;
        case 0x05: cpu.tima = 0x00; break;
        case 0x06: cpu.tma = val; break;
        case 0x07: cpu.tac = val; break;
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

        case 0x40:
            lcd.c = val;
            lcd_c_dirty();
        break;
        case 0x41: lcd.stat = val; break;
        case 0x42: lcd.scy = val; break;
        case 0x43: lcd.scx = val; break;
        case 0x44: lcd.ly = val; break;
        case 0x45:
            lcd.lyc = val;
        break;
        case 0x46: lcd_dma(val); break;
        case 0x47:
            lcd.bgp = val;
            lcd_bgpmap_dirty();
        break;
        case 0x48:
            lcd.obp0 = val;
            lcd_obp0map_dirty();
        break;
        case 0x49:
            lcd.obp1 = val;
            lcd_obp1map_dirty();
        break;
        case 0x4A: lcd.wy = val; break;
        case 0x4B: lcd.wx = val; break;

        /* TODO: CGB Mode */
        case 0x51: break;
        case 0x52: break;
        case 0x53: break;
        case 0x55: break;
        case 0x54: break;

        /* TODO: CGB Mode */
        case 0x68: break;
        case 0x69: break;
        case 0x6A: break;
        case 0x6B: break;
    }
}
