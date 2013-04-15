#include "lcd.h"
#include <string.h>
#include <stdio.h>
#include "cpu.h"
#include "sys/sys.h"
#include "emu.h"
#include "mem.h"
#include "defines.h"
#include "util/defines.h"
#include "lcd/obj.h"
#include "lcd/maps.h"
#include "debug/debug.h"

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

#define STAT_SET_MODE(m)  (lcd.stat = (lcd.stat & 0xFC) | (m))
#define STAT_SET_CFLAG(c) (lcd.stat = (lcd.stat & 0xFB) | ((c) << 2))

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
    memset(lcd.working_fb, 0x00, sizeof(lcd.fb)/2);
}

static void draw_line() {
    u8 x;

    if(hw.type == DMG_HW) {
        u16 *pixel = &lcd.working_fb[lcd.ly * LCD_WIDTH];
        u8 bg_scan[LCD_WIDTH];
        dmg_obj_scan_t obj_scan[LCD_WIDTH];

        memset(bg_scan, 0x00, sizeof(bg_scan));

        lcd_dmg_scan_bg(bg_scan);
        lcd_dmg_scan_obj(obj_scan);

        for(x = 0; x < LCD_WIDTH; x++, pixel++) {
            lcd.working_fb[lcd.ly * LCD_WIDTH + x] = lcd.bgp_map[bg_scan[x]];
//            if(obj_scan[x].priority) {
//                if(bg_scan[x] != 0 || obj_scan[x].data == 0) {
//                    *pixel = lcd.bgp_map[bg_scan[x]];
//                }
//                else {
//                    *pixel = lcd.obp_map[obj_scan[x].palette][obj_scan[x].data];
//                }
//            }
//            else {
//                if(obj_scan[x].data != 0) {
//                    *pixel = lcd.obp_map[obj_scan[x].palette][obj_scan[x].data];
//                }
//                else {
//                    *pixel = lcd.bgp_map[bg_scan[x]];
//                }
//            }
        }
    }
    else {

    }

//    if(lcd.c & LCDC_BG_ENABLE_BIT) {
//        lcd_render_bg_line();
//    }
//    if(lcd.c & LCDC_WND_ENABLE_BIT) {
//        lcd_render_wnd_line();
//    }
//    if(lcd.c & LCDC_OBJ_ENABLE_BIT) {
//        lcd_render_obj_line();
//    }
}

static inline void stat_irq(u8 flag) {
    if((lcd.c & LCDC_DISPLAY_ENABLE_BIT) && (lcd.stat & flag)) {
        cpu.irq |= IF_LCDSTAT;
        //printf("%.4X: LCDSTAT-INT %.2X %i %.2X %.2X\n", PC-1, flag, lcd.ly, cpu.ie, cpu.ime == IME_ON);
    }
}

static u8 step_mode(u8 m1) {
    u8 old = lcd.ly;

    u16 fc = cpu.cc % DUR_FULL_REFRESH;
    lcd.ly = fc / DUR_SCANLINE;
    STAT_SET_CFLAG(lcd.ly == lcd.lyc ? 1 : 0);


    if(lcd.ly < 144) {
        u16 lc = fc % DUR_SCANLINE;
        if(lc < DUR_MODE_2)
            return 0x02;
        else if(lc < DUR_MODE_2 + DUR_MODE_3)
            return 0x03;
        else
            return 0x00;
    }
    else {
        return 0x01;
    }
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
    lcd.bgp = 0xFC;
    lcd.obp[0] = 0xFF;
    lcd.obp[1] = 0xFF;

    memset(lcd.fb, 0x00, sizeof(lcd.fb));
    lcd.clean_fb = lcd.fb[0];
    lcd.working_fb = lcd.fb[1];

    memset(lcd.bgpd, 0x00, sizeof(lcd.bgpd));
    memset(lcd.obpd, 0x00, sizeof(lcd.obpd));

    lcd_c_dirty();
    lcd_bgp_dirty();
    lcd_obp0_dirty();
    lcd_obp1_dirty();
}

void lcd_step() {
    u16 m1, m2;

    m1 = lcd.stat & 0x03;
    m2 = step_mode(m1);
    STAT_SET_MODE(m2);


    if(m1 != m2) {
        //debug_sym_lcd_mode_change(m1, m2);
        switch(m2) {
            case 0x00:
                stat_irq(SIF_HBLANK);
                if(lcd.c & LCDC_DISPLAY_ENABLE_BIT) {
                    draw_line();
                }
            break;
            case 0x01:
                cpu.irq |= IF_VBLANK;
                stat_irq(SIF_VBLANK);
                sys_fb_ready();
                swap_fb();
            break;
            case 0x02:
                stat_irq(SIF_OAM);
                if(lcd.ly == lcd.lyc) {
                    stat_irq(SIF_LYC);
                }
            break;
            case 0x03:
            break;
        }
    }
}

void lcd_dma(u8 v) {
    u8 b;
    u16 src;

    for(src = ((u16)v)<<8, b = 0; b < 0xA0; b++, src++) {
        ram.oam[b] = mem_read_byte(src);
    }
}

void lcd_c_dirty() {
    lcd.bg_map = &ram.vrambank[lcd.c & LCDC_BG_MAP_BIT ? 0x1C00 : 0x1800];
    lcd.wnd_map = &ram.vrambank[lcd.c & LCDC_WND_MAP_BIT ? 0x1C00 : 0x1800];
}


void lcd_bgp_dirty() {
    u8 rc;

    for(rc = 0; rc < 4; rc++) {
        lcd.bgp_map[rc] = (lcd.bgp & (0x3 << (rc<<1))) >> (rc<<1);
    }
}

static void obp_dirty(u8 obp, u8 *obp_map) {
    u8 rc;
    for(rc = 0; rc < 4; rc++) {
        obp_map[rc] = (obp & (0x3 << (rc<<1))) >> (rc<<1);
    }
}

void lcd_obp0_dirty() {
    obp_dirty(lcd.obp[0], lcd.obp_map[0]);
}

void lcd_obp1_dirty() {
    obp_dirty(lcd.obp[1], lcd.obp_map[1]);
}


