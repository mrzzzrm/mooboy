#include "maps.h"
#include "mem.h"
#include <assert.h>
#include "mem/io/lcd.h"

#define FB_WIDTH 160

#define MAP_WIDTH 256
#define TILE_WIDTH 8
#define TILE_HEIGHT 8
#define MAP_COLUMNS 32
#define MAP_ROWS 32
#define LCDC_TILE_DATA_BIT 0x10

#define PIXEL_PER_BYTE 4

#define min(a, b) ((a) < (b) ? (a) : (b))


static void tile_line(u8 *line, u8 tx, u8 ty, u8 fbx) {
    for(; tx < 8 && fbx < FB_WIDTH; tx++, fbx++) {
        u8 pbm = 0x80 >> tx;
        u8 rcol = ((line[0] & pbm) >> (8-tx)) | ((line[1] & pbm) >> ((8-tx)-1));
        u8 pcol = (lcd.bgp & (0x3 << (rcol<<1))) >> (rcol<<1);

        lcd.working_fb[lcd.ly*FB_WIDTH + fbx] = pcol;
    }
}

static void render_map_line_signed_tdt(u8 *map, u8 mx, u8 my, u8 sx) {
    u8 fbx;
    u8 *tdt = &mbc.vrambank[0x1000];
    u8 tx = mx % TILE_WIDTH;
    u8 ty = my % TILE_HEIGHT;
    u8 tc = mx / TILE_WIDTH;
    u8 tr = my / TILE_HEIGHT;

    tile_line(&tdt[(s8)(map[tr * MAP_COLUMNS + tc])*0x10 + ty*2], tx, ty, 0);
    tc++;
    for(fbx = TILE_WIDTH - tx; fbx <= FB_WIDTH; fbx += 8, tc++) {
        tile_line(&tdt[(s8)(map[tr * MAP_COLUMNS + tc])*0x10 + ty*2], 0, ty, fbx);
    }
}

static void render_map_line_unsigned_tdt(u8 *map, u8 mx, u8 my, s8 sx) {
    u8 fbx;
    u8 *tdt = &mbc.vrambank[0x0000];
    u8 tx = mx % TILE_WIDTH;
    u8 ty = my % TILE_HEIGHT;
    u8 tc = mx / TILE_WIDTH;
    u8 tr = my / TILE_HEIGHT;

    tile_line(&tdt[map[tr * MAP_COLUMNS + tc]*0x10 + ty*2], tx, ty, 0);
    tc++;
    for(fbx = TILE_WIDTH - tx; fbx <= FB_WIDTH; fbx += 8, tc++) {
        tile_line(&tdt[map[tr * MAP_COLUMNS + tc]*0x10 + ty*2], 0, ty, fbx);
    }
}

void lcd_render_bg_line() {
    u8 mx = lcd.scx;
    u8 my = lcd.scy + lcd.ly;

    if(lcd.c & LCDC_TILE_DATA_BIT)
        render_map_line_unsigned_tdt(lcd.bgmap, mx, my, 0);
    else
        render_map_line_signed_tdt(lcd.bgmap, mx, my, 0);
}

void lcd_render_wnd_line() {
    u8 sx, mx;

    if(lcd.wy > lcd.ly) {
        return;
    }

    if(lcd.wx < 7) {
        sx = 0;
        mx = 7 - lcd.wx;
    }
    else {
        sx = lcd.wx - 7;
        mx = 0;
    }

    u8 my = lcd.ly - lcd.wy;

    if(lcd.c & LCDC_TILE_DATA_BIT)
        render_map_line_unsigned_tdt(lcd.wndmap, mx, my, sx);
    else
        render_map_line_signed_tdt(lcd.wndmap, mx, my, sx);
}

