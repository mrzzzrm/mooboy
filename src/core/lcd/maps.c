#include "maps.h"
#include <assert.h>
#include <stdio.h>
#include "mem.h"
#include "emu.h"
#include "lcd.h"


#define MAP_WIDTH 256
#define TILE_WIDTH 8
#define TILE_HEIGHT 8
#define MAP_COLUMNS 32
#define MAP_ROWS 32
#define LCDC_TILE_DATA_BIT 0x10

#define PRIORITY_SHIFT 7
#define PALETTE_MASK 0x07
#define BANK_SHIFT 3
#define HFLIP_BIT 0x20
#define VFLIP_BIT 0x40

static u8 sx, tx, ty, tc, tr, plus;
static u8 *tdt;
static u8 priority, palette, bank;

static inline void init_scan(u8 mx, u8 my) {
    tx = mx % TILE_WIDTH;
    ty = my % TILE_HEIGHT;
    tc = mx / TILE_WIDTH;
    tr = my / TILE_HEIGHT;
    plus = lcd.c & LCDC_TILE_DATA_BIT ? 0x00 : 0x80;
    tdt = lcd.c & LCDC_TILE_DATA_BIT ? &ram.vrambank[0x0000] : &ram.vrambank[0x0800];
}

static inline void scan_tile_line(u8 *scan, u8 *line) {
    u8 rshift = 7 - tx;

    for(; tx < TILE_WIDTH && sx < LCD_WIDTH; tx++, sx++) {
        u8 lsb = (line[0] >> rshift) & 0x01;
        u8 msb = (line[1] >> rshift) & 0x01;
        u8 render = lsb + (msb << 1);

        scan[sx] = render | palette | priority;
        rshift--;
    }
    tx = 0;
}

static void inline scan_tile_line_flipped(u8 *scan, u8 *line) {
    u8 rshift = tx;

    for(; tx < TILE_WIDTH && sx < LCD_WIDTH; tx++, sx++) {
        u8 lsb = (line[0] >> rshift) & 0x01;
        u8 msb = (line[1] >> rshift) & 0x01;
        u8 render = lsb + (msb << 1);

        scan[sx] = render | palette | priority;
        rshift++;
    }
    tx = 0;
}

static inline scan_line(u8 *map, u8 *scan) {
    u8 *tile_index;
    do {
        tile_index = &map[tr * MAP_COLUMNS + tc];
        scan_tile_line(scan, &tdt[((*tile_index + plus)%256)*0x10 + ty*2]);
        tc = (tc+1)%32;
    } while (sx < LCD_WIDTH);
}

static inline cgb_scan_line(u8 *map, u8 *attr_map, u8 *scan) {
    u8 tile_index, attributes, line;
    do {
        tile_index = map[tr * MAP_COLUMNS + tc];
        attributes = attr_map[tr * MAP_COLUMNS + tc];

        priority = attributes >> PRIORITY_SHIFT ? MAPS_PRIORITY_BIT : 0;
        palette = (attributes & PALETTE_MASK) << MAPS_PALETTE_SHIFT;
        bank = (attributes >> BANK_SHIFT) & 0x01;

        line = attributes & VFLIP_BIT ? 7-ty : ty;

        if(attributes & HFLIP_BIT) {
            scan_tile_line_flipped(scan, &tdt[((tile_index + plus)%256)*0x10 + line*2]);
        }
        else {
            scan_tile_line(scan, &tdt[((tile_index + plus)%256)*0x10 + line*2]);
        }
        tc = (tc+1)%32;
    } while (sx < LCD_WIDTH);
}

static inline void dmg_scan_bg(u8 *scan) {
    sx = 0;
    init_scan(lcd.scx, lcd.scy + lcd.ly);
    if(hw.type == CGB_HW) {
        cgb_scan_line(lcd.bg_map, lcd.bg_attr_map, scan);
    }
    else {
        scan_line(lcd.bg_map, scan);
    }
}

static inline void dmg_scan_wnd(u8 *scan) {
    u8 mx;

    if(lcd.wy > lcd.ly) {
        return;
    }
    if(lcd.wx <= 7) {
        sx = 0;
        mx = lcd.wx - 7;
    }
    else {
        sx = lcd.wx - 7;
        mx = 0;
    }

    init_scan(mx, lcd.ly - lcd.wy);
    if(hw.type == CGB_HW) {
        cgb_scan_line(lcd.wnd_map, lcd.wnd_attr_map, scan);
    }
    else {
        scan_line(lcd.wnd_map, scan);
    }
}

void lcd_dmg_scan_maps(u8 *scan) {
    if(lcd.c & LCDC_BG_ENABLE_BIT) {
        dmg_scan_bg(scan);
    }
    if(lcd.c & LCDC_WND_ENABLE_BIT) {
        dmg_scan_wnd(scan);
    }
}

void lcd_scan_maps(u8 *scan) {
    dmg_scan_bg(scan);
    if(lcd.c & LCDC_WND_ENABLE_BIT) {
        dmg_scan_wnd(scan);
    }
}



