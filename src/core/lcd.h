#ifndef IO_LCD_H
#define IO_LCD_H

#include "util/defines.h"

#define TILE_WIDTH 8
#define TILE_HEIGHT 8

#define LCDC_BG_ENABLE_BIT      0x01
#define LCDC_OBJ_ENABLE_BIT     0x02
#define LCDC_OBJ_SIZE_BIT       0x04
#define LCDC_BG_MAP_BIT         0x08
#define LCDC_TILE_DATA_BIT      0x10
#define LCDC_WND_ENABLE_BIT     0x20
#define LCDC_WND_MAP_BIT        0x40
#define LCDC_DISPLAY_ENABLE_BIT 0x80

#define LCD_WIDTH 160
#define LCD_HEIGHT 144

typedef struct {
    u8 c;
    u8 stat;
    u8 scx, scy;
    u8 ly;
    u8 lyc;
    u8 wx, wy;
    u8 bgp, obp0, obp1;
    /* TODO: CGB registers */

    u8 fb[2][144*160];
    u8 *clean_fb;
    u8 *working_fb;

    u8 *bgmap;
    u8 *wndmap;
} lcd_t;

extern lcd_t lcd;

void lcd_reset();

void lcd_step();
void lcd_dma(u8 v);

void lcd_c_dirty();

#endif
