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

#define DUR_MODE_0 (51 * cpu.freq_factor)
#define DUR_MODE_2 (20 * cpu.freq_factor)
#define DUR_MODE_3 (43 * cpu.freq_factor)
#define DUR_SCANLINE (114 * cpu.freq_factor)

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


static void unschedule() {
    int e;
    for(e = 0; e < 4; e++) {
        hw_unschedule(&lcd.mode_event[e]);
    }
    hw_unschedule(&lcd.vblank_line_event);
}

static inline void stat_irq(u8 flag) {
    if((lcd.c & LCDC_DISPLAY_ENABLE_BIT) && (lcd.stat & flag)) {
        cpu.irq |= IF_LCDSTAT;
    }
}

static void check_coincidence() {
    if(lcd.ly == lcd.lyc) {
        stat_irq(SIF_LYC);
        STAT_SET_CFLAG(1);
    }
    else {
        STAT_SET_CFLAG(0);
    }
}

static void swap_fb() {
    u16 *tmp = lcd.clean_fb;
    lcd.clean_fb = lcd.working_fb;
    lcd.working_fb = tmp;
}

static void draw_line_cgb_mode(scan_pixel_t *obj_scan, scan_pixel_t *maps_scan) {
    u16 *pixel = &lcd.working_fb[lcd.ly * LCD_WIDTH];
    int x, bg_priority, draw_bg;

    for(x = 0; x < LCD_WIDTH; x++, pixel++) {
       bg_priority = (lcd.c & LCDC_BG_ENABLE_BIT) &&
                      (maps_scan[x].priority || obj_scan[x].priority);
       draw_bg = (bg_priority && maps_scan[x].color_id != 0) || obj_scan[x].color_id == 0;

       *pixel = draw_bg ? maps_scan[x].color : obj_scan[x].color;
    }
}

static void draw_line_non_cgb_mode(scan_pixel_t *obj_scan, scan_pixel_t *maps_scan) {
    u16 *pixel = &lcd.working_fb[lcd.ly * LCD_WIDTH];
    u8 x;
    int bg_priority;

    for(x = 0; x < LCD_WIDTH; x++, pixel++) {
        bg_priority = (obj_scan[x].priority && maps_scan[x].color_id != 0) || obj_scan[x].color_id == 0;
        *pixel = bg_priority ? maps_scan[x].color : obj_scan[x].color;
    }
}


static void draw_line() {
    scan_pixel_t obj_scan[160];
    scan_pixel_t maps_scan[160];

    memset(obj_scan, 0x00, sizeof(obj_scan));

    if(lcd.c & LCDC_OBJ_ENABLE_BIT) {
        lcd_scan_obj(obj_scan);
    }
    lcd_scan_maps(maps_scan);

    if(moo.mode == CGB_MODE) {
        draw_line_cgb_mode(obj_scan, maps_scan);
    }
    else {
        draw_line_non_cgb_mode(obj_scan, maps_scan);
    }
}

inline void hdma() {
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
   // printf("%i/%i Next line\n", cpu.dbg_mcs, lcd.ly);

    lcd.ly++;
    lcd.ly %= 154;

    check_coincidence();
}

static void vblank_line(int mcs) {
    next_line();
    hw_schedule(lcd.ly == 153 ? &lcd.mode_event[2] : &lcd.vblank_line_event, DUR_SCANLINE - mcs);
}

static void mode_0(int mcs) {
    STAT_SET_MODE(0);
    stat_irq(SIF_HBLANK);

    if(lcd.c & LCDC_DISPLAY_ENABLE_BIT) {
        draw_line();
    }
    if(!lcd.hdma_inactive) {
        hdma();
    }

    hw_schedule(lcd.ly == 143 ? &lcd.mode_event[1] : &lcd.mode_event[2], DUR_MODE_0 - mcs);
}

static void mode_1(int mcs) {
    next_line();
    STAT_SET_MODE(1);

    cpu.irq |= IF_VBLANK;
    stat_irq(SIF_VBLANK);
    sys_fb_ready();
    swap_fb();

    hw_schedule(&lcd.vblank_line_event, DUR_SCANLINE - mcs);
}

static void mode_2(int mcs) {
    STAT_SET_MODE(2);
    next_line();

    stat_irq(SIF_OAM);

    hw_schedule(&lcd.mode_event[3], DUR_MODE_2 - mcs);
}

static void mode_3(int mcs) {
    STAT_SET_MODE(3);
    hw_schedule(&lcd.mode_event[0], DUR_MODE_3 - mcs);
}

void lcd_reset() {
    memset(&lcd, 0x00, sizeof(lcd));

    lcd.c = 0x91;
    lcd.stat = 0x81;
    lcd.ly = 0x90;

    lcd.hdma_dest = 0x8000;
    lcd.hdma_inactive = 0x80;

    lcd.clean_fb = lcd.fb[0];
    lcd.working_fb = lcd.fb[1];

    lcd.bgp.b[0] = 0xFC;
    lcd.obp.b[0] = 0xFF;
    lcd.obp.b[1] = 0xFF;
    lcd_rebuild_palette_maps();

    lcd.maps[0].tiles = &ram.vrambanks[0][0x1800];
    lcd.maps[0].attr = &ram.vrambanks[1][0x1800];
    lcd.maps[1].tiles = &ram.vrambanks[0][0x1C00];
    lcd.maps[1].attr = &ram.vrambanks[1][0x1C00];

    lcd.mode_event[0].callback = mode_0;
    lcd.mode_event[1].callback = mode_1;
    lcd.mode_event[2].callback = mode_2;
    lcd.mode_event[3].callback = mode_3;
    lcd.vblank_line_event.callback = vblank_line;

#ifdef DEBUG
    sprintf(lcd.mode_event[0].name, "lcd-mode-0");
    sprintf(lcd.mode_event[1].name, "lcd-mode-1");
    sprintf(lcd.mode_event[2].name, "lcd-mode-2");
    sprintf(lcd.mode_event[3].name, "lcd-mode-3");
    sprintf(lcd.vblank_line_event.name, "vblank_line");
#endif
}

void lcd_begin() {
    unschedule();
    hw_schedule(&lcd.vblank_line_event, DUR_MODE_0 + DUR_MODE_2);

    maps_dirty();
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

void lcd_hdma_control(u8 val) {
    lcd.hdma_length = val & 0x7F;
    if(val & 0x80) {
        lcd.hdma_inactive = 0x00;
    }
    else {
        if(lcd.hdma_inactive) {
            lcd_gdma();
            lcd.hdma_length = 0x7F;
        }
        lcd.hdma_inactive = 0x80;
    }
}

void lcd_enable() {
    lcd.stat = (lcd.stat & 0xF8) | 0x04;
    unschedule();
    lcd.ly = -1;
    mode_2(1);
}

void lcd_disable() {
    lcd.ly = 0;
    check_coincidence();
    lcd.stat = (lcd.stat & 0xF8) | 0x00;
    unschedule();
}

void lcd_set_lyc(u8 lyc) {
    lcd.lyc = lyc;
    check_coincidence();
}

void lcd_reset_ly() {
    lcd.ly = 0x00;
    check_coincidence();
}

void lcd_c_write(u8 val) {
    if(!(lcd.c & val & 0x80)) {
        if(val & 0x80) {
            lcd_enable();
        }
        else if(lcd.c & 0x80) {
            lcd_disable();
        }
    }

    if((lcd.c & 0x10) != (val & 0x10)) {
        maps_dirty();
    }

    lcd.c = val;
}

void lcd_vram_write(u16 adr, u8 val) {
    u16 vram_adr = adr - 0x8000;

    if(val == ram.vrambanks[ram.selected_vrambank][vram_adr]) {
        return;
    }

    ram.vrambanks[ram.selected_vrambank][vram_adr] = val;

    if(vram_adr >= 0x0000 && vram_adr < 0x1800) {
        maps_tiledata_dirty(vram_adr/16);
    }
    else if(vram_adr >= 0x1800 && vram_adr < 0x1C00) {
        maps_tile_dirty(&lcd.maps[0], vram_adr - 0x1800);
    }
    else if(vram_adr >= 0x1C00) {
        maps_tile_dirty(&lcd.maps[1], vram_adr - 0x1C00);
    }
}

static void update_cgb_palettes_map(lcd_palettes_t *palettes, u8 s) {
    u16 palette, color_id, d;

    palette = s/8;
    color_id = (s/2)%4;
    d = palettes->d[s];

    palettes->map[palette][color_id] = s & 1 ? (palettes->map[palette][color_id] & 0x00FF) | (d << 8) : (palettes->map[palette][color_id] & 0xFF00) | d;
}

static void update_dmg_palettes_map(lcd_palettes_t *palettes, u8 s) {
    u8 rc;

    for(rc = 0; rc < 4; rc++) {
        palettes->map[s][rc] = (palettes->b[s] & (0x3 << (rc<<1))) >> (rc<<1);
    }
    maps_dirty();
}

void lcd_palette_control(lcd_palettes_t *palettes, u8 val) {
    if(moo.mode == CGB_MODE) {
        palettes->s = val & 0x3F;
        palettes->i = val & 0x80;
    }
}

void lcd_cgb_palette_data(lcd_palettes_t *palettes, u8 val) {
    if(moo.mode == CGB_MODE) {
        palettes->d[palettes->s] = palettes->s & 0x01 ? val&0x7F : val;

        update_cgb_palettes_map(palettes, palettes->s);

        if(palettes->i) {
            palettes->s++;
            palettes->s &= 0x3F;
        }
    }
}

void lcd_dmg_palette_data(lcd_palettes_t *palettes, u8 val, u8 s) {
    palettes->b[s] = val;
    if(moo.mode == NON_CGB_MODE) {
        update_dmg_palettes_map(palettes, s);
    }
}

void lcd_rebuild_palette_maps() {
    if(moo.mode == CGB_MODE) {
        int i;
        for(i = 0; i < 0x40; i++) {
            update_cgb_palettes_map(&lcd.bgp, i);
            update_cgb_palettes_map(&lcd.obp, i);
        }
    }
    else {
        update_dmg_palettes_map(&lcd.bgp, 0);
        update_dmg_palettes_map(&lcd.obp, 0);
        update_dmg_palettes_map(&lcd.obp, 1);
    }
}

