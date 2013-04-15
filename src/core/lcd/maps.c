#include "maps.h"
#include <assert.h>
#include "mem.h"
#include "lcd.h"


#define MAP_WIDTH 256
#define TILE_WIDTH 8
#define TILE_HEIGHT 8
#define MAP_COLUMNS 32
#define MAP_ROWS 32
#define LCDC_TILE_DATA_BIT 0x10

#define min(a, b) ((a) < (b) ? (a) : (b))

static u8 tx, ty, tc, tr;
static u8 *dmg_scan;


static inline void set_map_cursor(u8 mx, u8 my) {
    tx = mx % TILE_WIDTH;
    ty = my % TILE_HEIGHT;
    tc = mx / TILE_WIDTH;
    tr = my / TILE_HEIGHT;
}

static inline u8 *get_signed_tile_line(u8 *tdt, s8 tdp, u8 ty) {
    return &tdt[tdp*0x10 + ty*2];
}

static inline u8 *get_unsigned_tile_line(u8 *tdt, u8 tdp, u8 ty) {
    return &tdt[tdp*0x10 + ty*2];
}

static void scan_tile_line(u8 *line, u8 tx, u8 ty, u8 sx) {
    u16 scan_cursor = sx;
    u16 line_end = (lcd.ly + 1) * LCD_WIDTH;
    u8 rshift = 7 - tx;

    for(; tx < TILE_WIDTH && scan_cursor < line_end; tx++, scan_cursor++) {
        u8 lsb = (line[0] >> rshift) & 0x01;
        u8 msb = (line[1] >> rshift) & 0x01;

        dmg_scan[scan_cursor] = lsb + (msb << 1);
        rshift--;
    }
}

static void dmg_scan_signed_tdt(u8 *map, u8 mx, u8 my, u8 sx) {
    u8 *tdt = &ram.vrambank[0x1000];
    u8 *tdp = &map[tr * MAP_COLUMNS + tc];

    scan_tile_line(get_signed_tile_line(tdt, *(s8*)tdp, ty), tx, ty, sx);
    tc++;
    tc %= 32;
    for(sx += (TILE_WIDTH-tx); sx < LCD_WIDTH; sx += TILE_WIDTH) {
        tdp = &map[tr * MAP_COLUMNS + tc];
        scan_tile_line(get_signed_tile_line(tdt, *(s8*)tdp, ty), 0, ty, sx);
        tc++;
        tc %= 32;
    }
}

static void dmg_scan_unsigned_tdt(u8 *map, u8 mx, u8 my, u8 sx) {
    u8 *tdt = &ram.vrambank[0x0000];
    u8 *tdp = &map[tr * MAP_COLUMNS + tc];

    scan_tile_line(get_unsigned_tile_line(tdt, *tdp, ty), tx, ty, sx);
    tc++;
    tc %= 32;
    for(sx += (TILE_WIDTH-tx); sx < LCD_WIDTH; sx += TILE_WIDTH) {
        tdp = &map[tr * MAP_COLUMNS + tc];
        scan_tile_line(get_unsigned_tile_line(tdt, *tdp, ty), 0, ty, sx);
        tc++;
        tc %= 32;
    }
}

void dmg_scan_bg() {
    u8 mx = lcd.scx;
    u8 my = lcd.scy + lcd.ly;
    set_map_cursor(mx, my);

    if(lcd.c & LCDC_TILE_DATA_BIT)
        dmg_scan_unsigned_tdt(lcd.bg_map, mx, my, 0);
    else
        dmg_scan_signed_tdt(lcd.bg_map, mx, my, 0);
}

void dmg_scan_wnd() {
    u8 sx, mx, my;

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

    my = lcd.ly - lcd.wy;
    set_map_cursor(mx, my);

    if(lcd.c & LCDC_TILE_DATA_BIT)
        dmg_scan_unsigned_tdt(lcd.wnd_map, mx, my, sx);
    else
        dmg_scan_signed_tdt(lcd.wnd_map, mx, my, sx);
}

void lcd_dmg_scan_bg(u8 *_scan) {
    dmg_scan = _scan;

    if(lcd.c & LCDC_BG_ENABLE_BIT) {
        dmg_scan_bg();
    }
    if(lcd.c & LCDC_WND_ENABLE_BIT) {
        dmg_scan_wnd();
    }
}


