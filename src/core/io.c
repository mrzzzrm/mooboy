#include "io.h"
#include <stdio.h>
#include "lcd.h"
#include "lcd/maps.h"
#include "lcd/obj.h"
#include "cpu.h"
#include "mem.h"
#include "timers.h"
#include "sound.h"
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

        /* Sound */
        case 0x10: case 0x11: case 0x12: case 0x13:
        case 0x14: case 0x16: case 0x17: case 0x18:
        case 0x19: case 0x1A: case 0x1B: case 0x1C:
        case 0x1D: case 0x1E: case 0x20: case 0x21:
        case 0x22: case 0x23: case 0x24: case 0x25:
        case 0x26: case 0x30: case 0x31: case 0x32:
        case 0x33: case 0x34: case 0x35: case 0x36:
        case 0x37: case 0x38: case 0x39: case 0x3A:
        case 0x3B: case 0x3C: case 0x3D: case 0x3E:
        case 0x3F:
            return sound_read(r);
        break;

        case 0x40: return lcd.c; break;
        case 0x41: return lcd.stat; break;
        case 0x42: return lcd.scy; break;
        case 0x43: return lcd.scx; break;
        case 0x44: return lcd.ly; break;
        case 0x45: return lcd.lyc; break;
        case 0x46: return 0x00; break;
        case 0x47: return lcd.bgp; break;
        case 0x48: return lcd.obp[0]; break;
        case 0x49: return lcd.obp[1]; break;
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

        default:;
            printf("Unknown IO read: %.2X\n", r);
    }

    return 0xFF; // Avoids nasty warnings, precious
}

void io_write(u16 adr, u8 val) {
    u8 r = adr & 0x00FF;
    switch(r) {
        /* Joypad */
        case 0x00: joy_select_col(val); break;
        case 0x01: break;
        case 0x02: break;

        case 0x04: timers.div = 0x00; break;
        case 0x05: timers.tima = 0x00; break;
        case 0x06: timers.tma = val; break;
        case 0x07: timers.tac = val; break;
        case 0x0F: cpu.irq = val & 0x1F; break;

        case 0x10: case 0x11: case 0x12: case 0x13:
        case 0x14: case 0x16: case 0x17: case 0x18:
        case 0x19: case 0x1A: case 0x1B: case 0x1C:
        case 0x1D: case 0x1E: case 0x20: case 0x21:
        case 0x22: case 0x23: case 0x24: case 0x25:
        case 0x26: case 0x30: case 0x31: case 0x32:
        case 0x33: case 0x34: case 0x35: case 0x36:
        case 0x37: case 0x38: case 0x39: case 0x3A:
        case 0x3B: case 0x3C: case 0x3D: case 0x3E:
        case 0x3F:
            sound_write(r, val);
        break;

        case 0x40:
            lcd.c = val;
            lcd_c_dirty();
        break;
        case 0x41: lcd.stat = (lcd.stat & 0x03) | (val & 0x78); /*printf("%.4X: LCDSTAT-INTs enabled: %.2X\n", PC-1, val & 0x78);*/ break;
        case 0x42: lcd.scy = val; /*printf("%.4X: SCY=%.2X\n", PC-1, val);*/ break;
        case 0x43: lcd.scx = val; break;
        case 0x44: lcd.ly = 0x00; break;
        case 0x45: lcd.lyc = val; break;
        case 0x46: lcd_dma(val); break;
        case 0x47:
            lcd.bgp = val;
            lcd_bgp_dirty();
        break;
        case 0x48:
            lcd.obp[0] = val;
            lcd_obp0_dirty();
        break;
        case 0x49:
            lcd.obp[1] = val;
            lcd_obp1_dirty();
        break;
        case 0x4A: lcd.wy = val; break;
        case 0x4B: lcd.wx = val; break;

        /* TODO: CGB Mode */
        case 0x4F: ram.vrambank = ram.vrambanks[val & 0x01]; break;
        case 0x51: break;
        case 0x52: break;
        case 0x53: break;
        case 0x55: break;
        case 0x54: break;

        /* TODO: CGB Mode */
        case 0x68: lcd.bgps = val & 0x1F; lcd.bgpi = val & 0x80; break;
        case 0x69: lcd.bgpd[lcd.bgps] = val; lcd_bgpd_dirty(lcd.bgps); if(lcd.bgpi) lcd.bgps++; lcd.bgps &= 0x3F; break;
        case 0x6A: lcd.obps = val & 0x1F; lcd.obpi = val & 0x80; break;
        case 0x6B: lcd.obpd[lcd.obps] = val; lcd_obpd_dirty(lcd.obps); if(lcd.obpi) lcd.obps++; lcd.obps &= 0x3F; break;

        case 0x70: ram.wrambank = ram.wrambanks[val != 0 ? val & 0x07 : 0x01]; break;

        default:;
            printf("Unknown IO write: %.2X=%.2X\n", r, val);
    }
}
