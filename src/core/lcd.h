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
    u8 bgp, obp[2];
    u8 bgpd[0x40], obpd[0x40];
    u8 bgps, bgpi;
    u8 obps, obpi;

    u16 fb[2][144*160];
    u16 *clean_fb;
    u16 *working_fb;

    u16 hdma_source, hdma_dest;
    u8 hdma_length, hdma_inactive;

    u8 *bg_map, *bg_attr_map;
    u8 *wnd_map, *wnd_attr_map;

    u8 bgp_map[4];
    u8 obp_map[2][4];

    u16 bgpd_map[8][4];
    u16 obpd_map[8][4];

    u32 cc;
} lcd_t;

extern lcd_t lcd;

void lcd_reset();

void lcd_step();
void lcd_dma(u8 v);
void lcd_gdma();
void lcd_hdma();

void lcd_c_dirty();
void lcd_obp0_dirty();
void lcd_obp1_dirty();
void lcd_bgp_dirty();
void lcd_bgpd_dirty(u8 bgps);
void lcd_obpd_dirty(u8 obps);

#endif
