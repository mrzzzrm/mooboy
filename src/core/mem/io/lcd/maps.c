#include "maps.h"
#include "io/lcd.h"
#include "mem.h"

#define LCD_WIDTH 160

#define MAP_WIDTH 256
#define TILE_WIDTH 32
#define MAP_COLUMNS 32
#define MAP_ROWS 32
#define TILE_COLUMNS

#define LCDC_TILE_DATA_BIT 0x10
#define LCDC_BG_MAP_BIT 0x04

#define PIXEL_PER_BYTE 4

#define min(a, b) ((a) < (b) ? (a) : (b))

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

void lcd_render_wnd_line() {

}

