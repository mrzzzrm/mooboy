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
#define LCDC_BG_MAP_BIT 0x04

#define PIXEL_PER_BYTE 4

#define min(a, b) ((a) < (b) ? (a) : (b))

/*
static u8 render_tile_line_seg(u8 tile, u8 pixel_x, u8 pixel_y, u8 lcd_x) {
    u8 *tile_pixels;
    u8 p, tx;

    if(lcd.c & LCDC_TILE_DATA_BIT)
        tile_pixels = &mbc.vrambank[0x1000 + ((s16)((s8)tile) << 8)];
    else
        tile_pixels = &mbc.vrambank[0x0000 + ((u16)tile << 8)];

    for(p = pixel_y * TILE_WIDTH + pixel_x, tx = pixel_x; tx < TILE_WIDTH && lcd_x < LCD_WIDTH; tx++, p++, lcd_x++) {
        lcd.working_fb[lcd.ly*LCD_WIDTH + lcd_x] = tile_pixels[p / PIXEL_PER_BYTE] & (0x03 << (p % PIXEL_PER_BYTE));
    }

    return lcd_x;
}

static void render_map_line_seg(u8 *map, u8 map_x, u8 map_y, u8 lcd_x, u8 count) {
    u8 col, row;
    u8 pixel_x, pixel_y;

    col = map_x / MAP_COLUMNS;
    row = map_y / MAP_ROWS;
    pixel_x = map_x % MAP_COLUMNS;
    pixel_y = map_y % MAP_ROWS;

    lcd_x = render_tile_line_seg(map[row * TILE_COLUMNS + col], pixel_x, pixel_y, lcd_x);

    for(; lcd_x < LCD_WIDTH; col++) {
        lcd_x = render_tile_line_seg(map[row * TILE_COLUMNS + col], 0, pixel_y, lcd_x);
    }
}

void lcd_render_bg_line() {
    u8 *map;
    u8 map_x, map_y;
    u8 count, count_wrap;

    map = &mbc.vrambank[lcd.c & LCDC_BG_MAP_BIT ? 0x1C00 : 0x1800];

    map_x = lcd.scx;
    map_y = lcd.scy + lcd.ly;

    count = min(LCD_WIDTH, MAP_WIDTH - map_x);
    count_wrap = MAP_WIDTH - count;

    render_map_line_seg(map, map_x, map_y, 0, count);

    if(count_wrap > 0) {
        render_map_line_seg(map, 0, map_y, count, count_wrap);
    }
}
*/
//
//static u8 render_tile_line_seg(u8 tile, u8 pixel_x, u8 pixel_y, u8 lcd_x) {
//    u8 *tile_pixels;
//    u8 p, tx;
//
//    if(lcd.c & LCDC_TILE_DATA_BIT)
//        tile_pixels = &mbc.vrambank[0x1000 + ((s16)((s8)tile) << 8)];
//    else
//        tile_pixels = &mbc.vrambank[0x0000 + ((u16)tile << 8)];
//
//    for(p = pixel_y * TILE_WIDTH + pixel_x, tx = pixel_x; tx < TILE_WIDTH && lcd_x < LCD_WIDTH; tx++, p++, lcd_x++) {
//        lcd.working_fb[lcd.ly*LCD_WIDTH + lcd_x] = tile_pixels[p / PIXEL_PER_BYTE] & (0x03 << (p % PIXEL_PER_BYTE));
//    }
//
//    return lcd_x;
//}
//
//static void render_map_line_seg(u8 *map, u8 map_x, u8 map_y, u8 lcd_x, u8 count) {
//    u8 col, row;
//    u8 pixel_x, pixel_y;
//
//    col = map_x / MAP_COLUMNS;
//    row = map_y / MAP_ROWS;
//    pixel_x = map_x % MAP_COLUMNS;
//    pixel_y = map_y % MAP_ROWS;
//
//    lcd_x = render_tile_line_seg(map[row * TILE_COLUMNS + col], pixel_x, pixel_y, lcd_x);
//
//    for(; lcd_x < LCD_WIDTH; col++) {
//        lcd_x = render_tile_line_seg(map[row * TILE_COLUMNS + col], 0, pixel_y, lcd_x);
//    }
//}

static void tile_line(u8 *line, u8 tx, u8 ty, u8 fbx) {
        fprintf(stderr, "%.2X%.2X/%i ", line[0], line[1], lcd.ly);
    for(; tx < 8 && fbx < FB_WIDTH; tx++, fbx++) {
        u8 pbm = 0x80 >> tx;
        lcd.working_fb[lcd.ly*FB_WIDTH + fbx] = ((line[0] & pbm) >> (8-tx)) | ((line[1] & pbm) >> ((8-tx)-1));
    }
}

static void render_map_line_signed_tdt(u8 *map, u8 palette, u8 mx, u8 my) {

}

static void render_map_line_unsigned_tdt(u8 *map, u8 palette, u8 mx, u8 my) {
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
    fprintf(stderr, "\n");
}

void lcd_render_bg_line() {
    u8 *map = &mbc.vrambank[lcd.c & LCDC_BG_MAP_BIT ? 0x1C00 : 0x1800];
    u8 mx = lcd.scx;
    u8 my = lcd.scy + lcd.ly;

    if(lcd.c & LCDC_TILE_DATA_BIT)
        render_map_line_unsigned_tdt(map, lcd.bgp, mx, my);
    else
        render_map_line_signed_tdt(map, lcd.bgp, mx, my);
}

void lcd_render_wnd_line() {

}

