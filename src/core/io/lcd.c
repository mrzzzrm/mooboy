#include "lcd.h"
#include "fb.h"
#include "cpu.h"
#include "mem.h"
#include "cpu/defines.h"
#include "util/defines.h"
#include "lcd/maps.h"
#include "debug.h"

#define DUR_FULL_REFRESH 17556
#define DUR_MODE_0 51
#define DUR_MODE_1 1140
#define DUR_MODE_2 20
#define DUR_MODE_3 43
#define DUR_SCANLINE 114
#define DUR_VBLANK 1140

#define SIF_HBLANK 0x08
#define SIF_VBLANK 0x10
#define SIF_OAM    0x20
#define SIF_LYC    0x40

#define STAT_MODE       (lcd.stat & 0x03)
#define STAT_CFLAG      (lcd.stat & 0x04)
#define STAT_HBLANK_IRQ (lcd.stat & 0x08)
#define STAT_VBLANK_IRQ (lcd.stat & 0x10)
#define STAT_OAM_IRQ    (lcd.stat & 0x20)
#define STAT_LYC_IRQ    (lcd.stat & 0x40)

#define STAT_SET_MODE(m)  {lcd.stat ^= lcd.stat&0x03; lcd.stat |= (m);}
#define STAT_SET_CFLAG(c) (lcd.stat ^= (~lcd.stat)&(c<<2))

#define LCDC_DISPLAY_ENABLE_BIT 0x80
#define LCDC_BG_ENABLE_BIT 0x00
#define LCDC_WND_ENABLE_BIT 0x20

#define LCD_WIDTH 160
#define LCD_HEIGHT 144

#define TILE_WIDTH 8
#define TILE_HEIGHT 8
#define TILE_BYTES 16
#define TILE_LINE_BYTES 2

#define TILES_INDEX_SIGNED 0
#define TILES_INDEX_UNSIGNED 1

#define MAP_WIDTH 256
#define MAP_HEIGHT 256
#define MAP_COLUMNS 32
#define MAP_ROWS 32


lcd_t lcd;

static void swap_fb() {
    u8 *tmp = lcd.clean_fb;
    lcd.clean_fb = lcd.working_fb;
    lcd.working_fb = tmp;
}


static void draw_line() {
    if(lcd.c & LCDC_BG_ENABLE_BIT)
        lcd_render_bg_line();
    if(lcd.c & LCDC_WND_ENABLE_BIT)
        lcd_render_wnd_line();
}

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

static inline void stat_irq(u8 flag) {
    if(lcd.stat & flag)
        cpu.irq |= IF_LCDSTAT;
}

void lcd_reset() {
    lcd.c = 0x91;
    lcd.stat = 0x82;
    lcd.scx = 0;
    lcd.scy = 0;
    lcd.ly = 0;
    lcd.lyc = 0;
    lcd.wx = 0;
    lcd.wy = 0;
    lcd.bgp = 0;
    lcd.obp0 = 0;
    lcd.obp1 = 0;

    memset(lcd.fb, 0x00, sizeof(lcd.fb));
    lcd.clean_fb = lcd.fb[0];
    lcd.working_fb = lcd.fb[1];
}

void lcd_step() {
    u16 m1, m2;

    m1 = lcd.stat & 0x03;
    m2 = step_mode(m1);
    STAT_SET_MODE(m2);

    if(m1 != m2) {

        if(dbg.verbose >= DBG_VLVL_MIN) fprintf(stderr, "  LCD Mode %i => %i\n", m1, m2);
        switch(m1) {
            case 0x00:
                if(m2 == 0x01) { // VBlank IRQ
                    cpu.irq |= IF_VBLANK;
                    stat_irq(SIF_VBLANK);
                }
                else  {// OAM IRQ
                    stat_irq(SIF_OAM);
                }
            break;
            case 0x01: // OAM IRQ
                stat_irq(SIF_OAM);
                swap_fb();
            break;
            case 0x02: // Nothing?
            break;
            case 0x03: // HBlank IRQ
                stat_irq(SIF_HBLANK);
            break;
        }

        if(m2 == 0x02) {
            if(lcd.c & LCDC_DISPLAY_ENABLE_BIT) {
                draw_line();
            }
        }
    }

    if(lcd.ly == lcd.lyc) {
        stat_irq(SIF_LYC);
        STAT_SET_CFLAG(1);
    }
    else {
        STAT_SET_CFLAG(0);
    }
}

void lcd_dma(u8 v) {
    u8 b;
    u16 src;

    src = v<<8;
    for(src = ((u16)v)<<8, b = 0; b < 0x9F; b++, src++) {
        ram.oam[b] = mem_readb(src);
    }
}

