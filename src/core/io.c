#include "io.h"
#include "lcd.h"
#include "lcd/maps.h"
#include "lcd/obj.h"
#include "cpu.h"
#include "timers.h"
#include "defines.h"
#include "util/defines.h"
#include "debug/debug.h"
#include "joy.h"

u8 io_read(u16 adr) {
    u8 r = adr &  0x00FF;

    switch(r) {
        /* TODO: Joypad */
        case 0x00: return joy_read(); break;

        /* TODO: Serial */
        case 0x01: break;
        case 0x02: break;

        case 0x04: return timers.div; break;
        case 0x05: return timers.tima; break;
        case 0x06: return timers.tma; break;
        case 0x07: return timers.tac; break;
        case 0x0F: return cpu.irq; break;

        /* TODO: Sound */
        case 0x10: break;
        case 0x11: break;
        case 0x12: break;
        case 0x13: break;
        case 0x14: break;

        case 0x16: break;
        case 0x17: break;
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
        case 0x31: break;
        case 0x32: break;
        case 0x33: break;
        case 0x34: break;
        case 0x35: break;
        case 0x36: break;
        case 0x37: break;
        case 0x38: break;
        case 0x39: break;
        case 0x3A: break;
        case 0x3B: break;
        case 0x3C: break;
        case 0x3D: break;
        case 0x3E: break;
        case 0x3F: break;


        break;

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

        case 0x04: timers.div = 0x00; break;
        case 0x05: timers.tima = 0x00; break;
        case 0x06: timers.tma = val; break;
        case 0x07: timers.tac = val; break;
        case 0x0F: cpu.irq = val; break;

        case 0x10: sound_write_nr10(val); break;
        case 0x11: sound_write_nr11(val); break;
        case 0x12: sound_write_nr12(val); break;
        case 0x13: sound_write_nr13(val); break;
        case 0x14: sound_write_nr14(val); break;

        case 0x16: sound_write_nr21(val); break;
        case 0x17: sound_write_nr22(val); break;
        case 0x18: sound_write_nr23(val); break;
        case 0x19: sound_write_nr24(val); break;

        case 0x1A: sound_write_nr30(val); break;
        case 0x1B: sound_write_nr31(val); break;
        case 0x1C: sound_write_nr32(val); break;
        case 0x1D: sound_write_nr33(val); break;
        case 0x1E: sound_write_nr34(val); break;

        case 0x20: sound_write_nr41(val); break;
        case 0x21: sound_write_nr42(val); break;
        case 0x22: sound_write_nr43(val); break;
        case 0x23: sound_write_nr44(val); break;

        case 0x24: sound_write_nr50(val); break;
        case 0x25: sound_write_nr51(val); break;
        case 0x26: sound_write_nr52(val); break;

        case 0x30: case 0x31: case 0x32: case 0x33:
        case 0x34: case 0x35: case 0x36: case 0x37:
        case 0x38: case 0x39: case 0x3A: case 0x3B:
        case 0x3C: case 0x3D: case 0x3E: case 0x3F:
            sound_write_wave(r - 0x30, val);
        break;

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
