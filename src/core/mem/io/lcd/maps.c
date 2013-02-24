#include "maps.h"
#include "mem.h"
#include <assert.h>
#include "tile.h"
#include "mem/io/lcd.h"


#define MAP_WIDTH 256
#define TILE_WIDTH 8
#define TILE_HEIGHT 8
#define MAP_COLUMNS 32
#define MAP_ROWS 32
#define LCDC_TILE_DATA_BIT 0x10

#define min(a, b) ((a) < (b) ? (a) : (b))

static u8 tx, ty, tc, tr;
static u8 bgpmap[4];


static inline void set_map_cursor(u8 mx, u8 my) {
    tx = mx % TILE_WIDTH;
    ty = my % TILE_HEIGHT;
    tc = mx / TILE_WIDTH;
    tr = my / TILE_HEIGHT;
}

static inline u8 *get_signed_tile_line(u8 *tdt, u8 *tdp, u8 ty) {
    return &tdt[((s8)(*tdp)*0x10) + ty*2];
}

static inline u8 *get_unsigned_tile_line(u8 *tdt, u8 *tdp, u8 ty) {
    return &tdt[(*tdp)*0x10 + ty*2];
}

static void render_map_line_signed_tdt(u8 *map, u8 mx, u8 my, u8 fbx, int crap) {
    u8 *tdt = &mbc.vrambank[0x1000];
    u8 *tdp = &map[tr * MAP_COLUMNS + tc];

    lcd_render_tile_line(get_signed_tile_line(tdt, tdp, ty), tx, ty, fbx, bgpmap, crap);
    for(fbx += (TILE_WIDTH-tx); fbx < LCD_WIDTH; fbx += TILE_WIDTH) {
        tdp = &map[tr * MAP_COLUMNS + tc];
        lcd_render_tile_line(get_signed_tile_line(tdt, tdp, ty), 0, ty, fbx, bgpmap, crap);
        tc++;
    }
}

static void render_map_line_unsigned_tdt(u8 *map, u8 mx, u8 my, u8 fbx, int crap) {
    u8 *tdt = &mbc.vrambank[0x0000];
    u8 *tdp = &map[tr * MAP_COLUMNS + tc];

    lcd_render_tile_line(get_unsigned_tile_line(tdt, tdp, ty), tx, ty, fbx, bgpmap, crap);
    for(fbx += (TILE_WIDTH-tx); fbx < LCD_WIDTH; fbx += TILE_WIDTH) {
        tdp = &map[tr * MAP_COLUMNS + tc];
        lcd_render_tile_line(get_unsigned_tile_line(tdt, tdp, ty), 0, ty, fbx, bgpmap, crap);
        tc++;
    }
}

void lcd_render_bg_line() {
    u8 mx = lcd.scx;
    u8 my = lcd.scy + lcd.ly;
    set_map_cursor(mx, my);

    if(lcd.c & LCDC_TILE_DATA_BIT)
        render_map_line_unsigned_tdt(lcd.bgmap, mx, my, 0, 0);
    else
        render_map_line_signed_tdt(lcd.bgmap, mx, my, 0, 0);
}

void lcd_render_wnd_line() {
    u8 fbx, mx, my;

    if(lcd.wy > lcd.ly) {
        return;
    }
    if(lcd.wx <= 7) {
        fbx = 0;
        mx = lcd.wx;
    }
    else {
        fbx = lcd.wx - 7;
        mx = 0;
    }

    my = lcd.ly - lcd.wy;
    set_map_cursor(mx, my);

    if(lcd.c & LCDC_TILE_DATA_BIT)
        render_map_line_unsigned_tdt(lcd.wndmap, mx, my, fbx, 1);
    else
        render_map_line_signed_tdt(lcd.wndmap, mx, my, fbx, 1);
}

void lcd_bgpmap_dirty() {
    u8 rc;

    for(rc = 0; rc < 4; rc++) {
        bgpmap[rc] = (lcd.bgp & (0x3 << (rc<<1))) >> (rc<<1);
    }
}

