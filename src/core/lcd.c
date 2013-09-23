#include "lcd.h"
#include <string.h>
#include <stdio.h>
#include "cpu.h"
#include "sys/sys.h"
#include "moo.h"
#include "mem.h"
#include "defines.h"
#include "obj.h"
#include "maps.h"

#define DUR_FULL_REFRESH 17556
#define DUR_MODE_0 51
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

hw_event_t lcd_mode_0_event;
hw_event_t lcd_mode_1_event;
hw_event_t lcd_mode_2_event;
hw_event_t lcd_mode_3_event;
hw_event_t lcd_vblank_line_event;

static void mode_2(int mcs);

static void unschedule() {
    hw_unschedule(&lcd_mode_0_event);
    hw_unschedule(&lcd_mode_1_event);
    hw_unschedule(&lcd_mode_2_event);
    hw_unschedule(&lcd_mode_3_event);
    hw_unschedule(&lcd_vblank_line_event);
}

static void swap_fb() {
    u16 *tmp = lcd.clean_fb;
    lcd.clean_fb = lcd.working_fb;
    lcd.working_fb = tmp;
}

static void draw_line_dmg(u8 *maps_scan, u8 *obj_scan) {
    u16 *pixel = &lcd.working_fb[lcd.ly * LCD_WIDTH];
    u8 x;

    for(x = 0; x < LCD_WIDTH; x++, pixel++) {
        if(OBJ_PRIORITY(obj_scan[x])) {
            if(maps_scan[x] != 0) {
                *pixel = lcd.bgp_map[maps_scan[x]];
            }
            else {
                *pixel = lcd.obp_map[OBJ_PALETTE(obj_scan[x])][OBJ_DATA(obj_scan[x])];
            }
        }
        else {
            if(OBJ_DATA(obj_scan[x]) != 0) {
                *pixel = lcd.obp_map[OBJ_PALETTE(obj_scan[x])][OBJ_DATA(obj_scan[x])];
            }
            else {
                *pixel = lcd.bgp_map[maps_scan[x]];
            }
        }
    }
}

static void draw_line_cgb_mode(u8 *maps_scan, u8 *obj_scan) {
    u16 *pixel = &lcd.working_fb[lcd.ly * LCD_WIDTH];
    u8 x;

    for(x = 0; x < LCD_WIDTH; x++, pixel++) {
        u8 bg_priority;

        if(lcd.c & LCDC_BG_ENABLE_BIT) {
            if(maps_scan[x] & MAPS_PRIORITY_BIT) {
                bg_priority = 1;
            }
            else {
                bg_priority = obj_scan[x] & OBJ_PRIORITY_BIT ? 1 : 0;
            }
        }
        else {
            bg_priority = 0;
        }

        if(bg_priority) {
            if(MAPS_DATA(maps_scan[x]) != 0 || OBJ_DATA(obj_scan[x]) == 0) {
                *pixel = lcd.bgpd_map[MAPS_PALETTE(maps_scan[x])][MAPS_DATA(maps_scan[x])];
            }
            else {
                *pixel = lcd.obpd_map[OBJ_PALETTE(obj_scan[x])][OBJ_DATA(obj_scan[x])];
            }
        }
        else {
            if(OBJ_DATA(obj_scan[x]) != 0) {
                *pixel = lcd.obpd_map[OBJ_PALETTE(obj_scan[x])][OBJ_DATA(obj_scan[x])];
            }
            else {
                *pixel = lcd.bgpd_map[MAPS_PALETTE(maps_scan[x])][MAPS_DATA(maps_scan[x])];
            }
        }
    }
}

static void draw_line_non_cgb_mode(u8 *maps_scan, u8 *obj_scan) {
    u16 *pixel = &lcd.working_fb[lcd.ly * LCD_WIDTH];
    u8 x;

    for(x = 0; x < LCD_WIDTH; x++, pixel++) {
        if(OBJ_PRIORITY(obj_scan[x])) {
            if(maps_scan[x] != 0) {
                *pixel = lcd.bgp_map[maps_scan[x]];
            }
            else {
                *pixel = lcd.obp_map[OBJ_PALETTE(obj_scan[x])][OBJ_DATA(obj_scan[x])];
            }
        }
        else {
            if(OBJ_DATA(obj_scan[x]) != 0) {
                *pixel = lcd.obp_map[OBJ_PALETTE(obj_scan[x])][OBJ_DATA(obj_scan[x])];
            }
            else {
                *pixel = lcd.bgp_map[maps_scan[x]];
            }
        }
    }
}

static void draw_line() {
    u8 maps_scan[LCD_WIDTH];
    u8 obj_scan[LCD_WIDTH];

    memset(maps_scan, 0x00, sizeof(maps_scan));
    memset(obj_scan, 0x00, sizeof(obj_scan));

    if(lcd.c & LCDC_OBJ_ENABLE_BIT) {
        lcd_scan_obj(obj_scan);
    }
    lcd_scan_maps(maps_scan);

    if(moo.hw == DMG_HW) {
        draw_line_dmg(maps_scan, obj_scan);
    }
    else {
        if(moo.mode == CGB_MODE)
            draw_line_cgb_mode(maps_scan, obj_scan);
        else
            draw_line_non_cgb_mode(maps_scan, obj_scan);
    }
}

static inline void stat_irq(u8 flag) {
    if((lcd.c & LCDC_DISPLAY_ENABLE_BIT) && (lcd.stat & flag)) {
        cpu.irq |= IF_LCDSTAT;
    }
}

inline void lcd_hdma() {
    u16 end;

    hw_defer(8);

    for(end = lcd.hdma_source + 0x10; lcd.hdma_source < end; lcd.hdma_source++, lcd.hdma_dest++) {
        mem_write_byte(lcd.hdma_dest, mem_read_byte(lcd.hdma_source));
    }

    if(lcd.hdma_length == 0x00) {
        lcd.hdma_length = 0x7F;
        lcd.hdma_inactive = 0x80;
    }
    else {
        lcd.hdma_length--;
    }
}

static void next_line() {
    lcd.ly++;
    lcd.ly %= 154;

    if(lcd.ly == lcd.lyc) {
        lcd.stat |= 0x04;
        stat_irq(SIF_LYC);
    }
}

static void vblank_line(int mcs) {
    next_line();
    if(lcd.ly == 153) {
        hw_schedule(&lcd_mode_2_event, DUR_SCANLINE * cpu.freq_factor - mcs);
    }
    else {
        hw_schedule(&lcd_vblank_line_event, DUR_SCANLINE * cpu.freq_factor - mcs);
    }
}

static void mode_0(int mcs) {
    STAT_SET_MODE(0);
    stat_irq(SIF_HBLANK);

    if(lcd.c & LCDC_DISPLAY_ENABLE_BIT) {
        draw_line();
    }
    if(!lcd.hdma_inactive) {
        lcd_hdma();
    }

    if(lcd.ly == 143) {
        hw_schedule(&lcd_mode_1_event, DUR_MODE_0 * cpu.freq_factor - mcs);
    }
    else {
       hw_schedule(&lcd_mode_2_event, DUR_MODE_0 * cpu.freq_factor - mcs);
    }
}

static void mode_1(int mcs) {
    next_line();
    STAT_SET_MODE(1);

    cpu.irq |= IF_VBLANK;
    stat_irq(SIF_VBLANK);
    sys_fb_ready();
    swap_fb();

    hw_schedule(&lcd_vblank_line_event, DUR_SCANLINE * cpu.freq_factor - mcs);
}

static void mode_2(int mcs) {
    next_line();
    STAT_SET_MODE(2);
    stat_irq(SIF_OAM);

    hw_schedule(&lcd_mode_3_event, DUR_MODE_2 * cpu.freq_factor - mcs);
}

static void mode_3(int mcs) {
    STAT_SET_MODE(3);
    hw_schedule(&lcd_mode_0_event, DUR_MODE_3 * cpu.freq_factor - mcs);
}

void lcd_reset() {
    lcd.c = 0x91;
    lcd.stat = 0x81;
    lcd.scx = 0;
    lcd.scy = 0;
    lcd.ly = 0x90;
    lcd.lyc = 0;
    lcd.wx = 0;
    lcd.wy = 0;
    lcd.bgp = 0xFC;
    lcd.obp[0] = 0xFF;
    lcd.obp[1] = 0xFF;
    lcd.cc = lcd.ly * DUR_SCANLINE + DUR_MODE_3;

    lcd.hdma_source = 0x0000;
    lcd.hdma_dest = 0x8000;
    lcd.hdma_length = 0x00;
    lcd.hdma_inactive = 0x80;

    memset(lcd.fb, 0x00, sizeof(lcd.fb));
    lcd.clean_fb = lcd.fb[0];
    lcd.working_fb = lcd.fb[1];

    memset(lcd.bgpd, 0x00, sizeof(lcd.bgpd));
    memset(lcd.obpd, 0x00, sizeof(lcd.obpd));
    memset(lcd.bgpd_map, 0x00, sizeof(lcd.bgpd_map));
    memset(lcd.obpd_map, 0x00, sizeof(lcd.obpd_map));

    lcd.non_cgb_bgp = 0;
    lcd.non_cgb_obp[0] = 0;
    lcd.non_cgb_obp[1] = 0;

    lcd_c_dirty();
    lcd_bgp_dirty();
    lcd_obp0_dirty();
    lcd_obp1_dirty();

    lcd_mode_0_event.callback = mode_0;
    lcd_mode_1_event.callback = mode_1;
    lcd_mode_2_event.callback = mode_2;
    lcd_mode_3_event.callback = mode_3;
    lcd_vblank_line_event.callback = vblank_line;

#ifdef DEBUG
    sprintf(lcd_mode_0_event.name, "lcd-mode-0");
    sprintf(lcd_mode_1_event.name, "lcd-mode-1");
    sprintf(lcd_mode_2_event.name, "lcd-mode-2");
    sprintf(lcd_mode_3_event.name, "lcd-mode-3");
    sprintf(lcd_vblank_line_event.name, "vblank_line");
#endif
}

void lcd_begin() {
    unschedule();
    hw_schedule(&lcd_vblank_line_event, (DUR_MODE_0 + DUR_MODE_2) * cpu.freq_factor);
}

void lcd_dma(u8 v) {
    u8 b;
    u16 src;

    for(src = ((u16)v)<<8, b = 0; b < 0xA0; b++, src++) {
        ram.oam[b] = mem_read_byte(src);
    }
}

void lcd_gdma() {
    int d;
    u16 length, source, dest, end;

    for(d = 0; d <= lcd.hdma_length; d++) {
        hw_step(8);
    }

    length = (lcd.hdma_length + 1) * 0x10;
    source = lcd.hdma_source;
    dest = lcd.hdma_dest;
    end = source + length;

    for(; source < end; source++, dest++) {
        mem_write_byte(dest, mem_read_byte(source));
    }

    lcd.hdma_length = 0x7F;
    lcd.hdma_inactive = 0x80;
}

void lcd_enable() {
    lcd.stat = (lcd.stat & 0xF8) | 0x04;
    unschedule();
    lcd.ly = -1;
    mode_2(1);
}

void lcd_disable() {
    lcd.ly = 0;
    lcd.stat = (lcd.stat & 0xF8) | 0x00;
    unschedule();
}

void lcd_set_lyc(u8 lyc) {
    lcd.lyc = lyc;
}

void lcd_reset_ly() {
    lcd.ly = 0x00;
    lcd.cc = 0;
}

void lcd_c_dirty() {
    lcd.bg_map = &ram.vrambanks[0][lcd.c & LCDC_BG_MAP_BIT ? 0x1C00 : 0x1800];
    lcd.wnd_map = &ram.vrambanks[0][lcd.c & LCDC_WND_MAP_BIT ? 0x1C00 : 0x1800];
    lcd.bg_attr_map = &ram.vrambanks[1][lcd.c & LCDC_BG_MAP_BIT ? 0x1C00 : 0x1800];
    lcd.wnd_attr_map = &ram.vrambanks[1][lcd.c & LCDC_WND_MAP_BIT ? 0x1C00 : 0x1800];
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

static void pd_dirty(u16 map[8][4], u8 d, u8 s) {
    u16 palette, color;

    palette = s/8;
    color = (s/2)%4;

    map[palette][color] = s % 2 == 0 ? (map[palette][color] & 0xFF00) | d : (map[palette][color] & 0x00FF) | (d << 8);
}

void lcd_bgpd_dirty(u8 bgps) {
    pd_dirty(lcd.bgpd_map, lcd.bgpd[bgps], bgps);
}

void lcd_obpd_dirty(u8 obps) {
    pd_dirty(lcd.obpd_map, lcd.obpd[obps], obps);
}

