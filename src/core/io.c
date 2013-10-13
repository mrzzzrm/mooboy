#include "io.h"
#include <stdio.h>
#include "lcd.h"
#include "maps.h"
#include "obj.h"
#include "cpu.h"
#include "mem.h"
#include "timers.h"
#include "sound.h"
#include "defines.h"
#include "joy.h"
#include "serial.h"

u8 io_read(u16 adr) {
    u8 reg = adr & 0x00FF;

    switch(reg) {
        case 0x00: return joy_read(); break;

        case 0x01: /* return serial.sb;*/ break;
        case 0x02: /* return serial.sc;*/ break;

        case 0x04: return timers.div; break;
        case 0x05: return timers.tima; break;
        case 0x06: return timers.tma; break;
        case 0x07: return timers.tac; break;
        case 0x0F: return cpu.irq; break;

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
            return sound_read(reg);
        break;

        case 0x15: case 0x1F: return 0x00; break;

        case 0x40: return lcd.c; break;
        case 0x41: return lcd.stat; break;
        case 0x42: return lcd.scy; break;
        case 0x43: return lcd.scx; break;
        case 0x44: return lcd.ly; break;
        case 0x45: return lcd.lyc; break;
        case 0x46: return 0xFF; break;
        case 0x47: return lcd.bgp.b[0]; break;
        case 0x48: return lcd.obp.b[0]; break;
        case 0x49: return lcd.obp.b[1]; break;
        case 0x4A: return lcd.wy; break;
        case 0x4B: return lcd.wx; break;

        case 0x4D: return cpu.freq_switch | (cpu.freq == DOUBLE_CPU_FREQ ? 0x80 : 0x00); break;

        case 0x4F: return ram.selected_vrambank; break;

        case 0x51: return lcd.hdma_source >> 8; break;
        case 0x52: return lcd.hdma_source & 0xFF; break;
        case 0x53: return lcd.hdma_dest >> 8; break;
        case 0x54: return lcd.hdma_dest & 0xFF; break;
        case 0x55: return lcd.hdma_length | lcd.hdma_inactive; break;

        case 0x56: return 0x40; break;

        case 0x68: return lcd.bgp.s | lcd.bgp.i; break;
        case 0x69: return lcd.bgp.d[lcd.bgp.s]; break;
        case 0x6A: return lcd.obp.s | lcd.obp.i; break;
        case 0x6B: return lcd.obp.d[lcd.obp.s]; break;

        case 0x70: return ram.rambank_index | 0xF8; break;

        default:;
#ifdef DEBUG
            printf("Unknown IO read: %.2X\n", reg);
#endif
    }

    return 0xFF; // Avoids nasty warnings, precious
}

void io_write(u16 adr, u8 val) {
    u8 reg = adr & 0x00FF;

    switch(reg) {
        case 0x00: joy_select_col(val); break;

        case 0x01: /*serial.sb = val;*/  break;
        case 0x02: /*serial_sc_write(val);*/ break;

        case 0x04: timers.div = 0x00; break;
        case 0x05: timers.tima = val; break;
        case 0x06: timers.tma = val; break;
        case 0x07: timers_tac(val & 0x07); break;
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
            sound_write(reg, val);
        break;

        case 0x15: case 0x1F: break;

        case 0x40: lcd_c_write(val); break;
        case 0x41: lcd.stat = (lcd.stat & 0x87) | (val & 0x78); break;
        case 0x42: lcd.scy = val; break;
        case 0x43: lcd.scx = val; break;
        case 0x44: lcd_reset_ly(); break;
        case 0x45: lcd_set_lyc(val); break;
        case 0x46: lcd_dma(val); break;
        case 0x47: lcd_dmg_palette_data(&lcd.bgp, val, 0); break;
        case 0x48: lcd_dmg_palette_data(&lcd.obp, val, 0); break;
        case 0x49: lcd_dmg_palette_data(&lcd.obp, val, 1); break;
        case 0x4A: lcd.wy = val; break;
        case 0x4B: lcd.wx = val; break;

        case 0x4D: cpu.freq_switch = val & 0x01;  break;

        case 0x4F: ram.selected_vrambank = val & 0x01; break;

        case 0x51: lcd.hdma_source = (lcd.hdma_source & 0x00FF) | (val << 8); break;
        case 0x52: lcd.hdma_source = (lcd.hdma_source & 0xFF00) | (val & 0xF0); break;
        case 0x53: lcd.hdma_dest = (lcd.hdma_dest & 0x80FF) | ((val & 0x1F) << 8); break;
        case 0x54: lcd.hdma_dest = (lcd.hdma_dest & 0xFF00) | (val & 0xF0); break;
        case 0x55: lcd_hdma_control(val); break;

        case 0x56: break;

        case 0x68: lcd_palette_control(&lcd.bgp, val); break;
        case 0x69: lcd_cgb_palette_data(&lcd.bgp, val); break;
        case 0x6A: lcd_palette_control(&lcd.obp, val); break;
        case 0x6B: lcd_cgb_palette_data(&lcd.obp, val); break;

        case 0x70:
            ram.rambank_index = (val & 0x07) != 0 ? val & 0x07 : 0x01;
            ram.rambank = ram.rambanks[ram.rambank_index];
        break;

        default:;
#ifdef DEBUG
            printf("Unknown IO write: %.2X=%.2X\n", reg, val);
#endif
    }
}


