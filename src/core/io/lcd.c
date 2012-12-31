#include "lcd.h"
#include "cpu.h"
#include "cpu/defines.h"
#include "util/defines.h"

#define DUR_FULL_REFRESH 17556
#define DUR_MODE_0 51
#define DUR_MODE_1 1140
#define DUR_MODE_2 20
#define DUR_MODE_3 43
#define DUR_SCANLINE 114
#define DUR_VBLANK 1140

#define STAT_MODE       (lcd.stat & 0x03)
#define STAT_CFLAG      (lcd.stat & 0x04)
#define STAT_HBLANK_IRQ (lcd.stat & 0x08)
#define STAT_VBLANK_IRQ (lcd.stat & 0x10)
#define STAT_OAM_IRQ    (lcd.stat & 0x20)
#define STAT_LYC_IRQ    (lcd.stat & 0x40)

#define STAT_SET_MODE(m)  {lcd.stat ^= lcd.stat&0x03; lcd.stat |= (m);}
#define STAT_SET_CFLAG(c) (lcd.stat ^= (~lcd.stat)&(c<<2))

lcd_t lcd;


static u8 step_mode(u8 m1) {
    u16 fc = cpu.cc % DUR_FULL_REFRESH;
    lcd.ly = fc / DUR_SCANLINE;

    if(lcd.ly < 144) {
        u16 lc = fc % DUR_SCANLINE;
        if(lc < DUR_MODE_2)
            return 0x02;
        else if(lc < DUR_MODE_2 + DUR_MODE_3)
            return 0x02;
        else
            return 0x00;
    }
    else {
        return 0x01;
    }
}

static void refresh_line() {

}

void lcd_reset() {
    lcd.lcdc = 0x91;
    lcd.stat = 0;
    lcd.scx = 0;
    lcd.scy = 0;
    lcd.ly = 0;
    lcd.lyc = 0;
    lcd.wx = 0;
    lcd.wy = 0;
    lcd.bgp = 0;
    lcd.obp0 = 0;
    lcd.obp1 = 0;
}

void lcd_step() {
    u16 m1, m2;

    m1 = lcd.stat & 0x03;
    m2 = step_mode(m1);
    STAT_SET_MODE(m2);

    if(m1 != m2) {
        switch(m1) {
            case 0x00:
                if(m2 == 0x01) { // VBlank IRQ

                }
                else { // OAM IRQ

                }
            break;
            case 0x01: // OAM IRQ
            break;
            case 0x02: // Nothing?
            break;
            case 0x03: // HBlank IRQ
            break;
        }

        if(m2 == 0x02) {
            refresh_line();
        }
    }
}

void lcd_dma(u8 v) {

}

void lcd_control(u8 v) {

}

void lcd_stat(u8 v) {

}

