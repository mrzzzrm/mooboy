#include "maps.h"
#include "defines.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "mem.h"
#include "moo.h"
#include "lcd.h"

static u8 priority, palette, tile_index, attr, bank;
static u8 *linedata;

static inline u8 render(u8 rshift) {
    u8 lsb = (linedata[0] >> rshift) & 0x01;
    u8 msb = (linedata[1] >> rshift) & 0x01;
    return (lsb | (msb << 1));
}

#define draw_tile_line(scan, meta, rshift_init, op) \
    int tx; \
    u8 rshift = rshift_init; \
    \
    for(tx = 0; tx < 8; tx++, rshift op) { \
        (meta)[tx].color_id = render(rshift); \
        (meta)[tx].priority = priority; \
        (scan)[tx] = lcd.bgp.map[palette][(meta)[tx].color_id]; \
    }

#define iterate_tile(line_init, op) \
    u8 line = line_init; \
    for(ty = 0; ty < 8; ty++, line op, cy++) { \
        linedata = &tdt[tile*0x10 + line*2]; \
        \
        if(attr & 0x20) { \
            draw_tile_line(&map->scan_cache[cy][cx], &map->cache_meta[cy][cx], 0, ++); \
        } \
        else { \
            draw_tile_line(&map->scan_cache[cy][cx], &map->cache_meta[cy][cx], 7, --); \
        } \
    }

static inline void draw_tile(lcd_map_t *map, int tx, int ty) {
    u8 index_offset = lcd.c & 0x10 ? 0x00 : 0x80;
    u8 tile = tile_index + index_offset;

    priority = attr & 0x80;
    u8 *tdt = &ram.vrambanks[bank][lcd.c & 0x10 ? 0x0000 : 0x0800];

    u8 cx = tx*8;
    u8 cy = ty*8;

    if(attr & 0x40) {
        iterate_tile(7, --);
    }
    else {
        iterate_tile(0, ++);
    }
}

static inline void mark_index_refs_dirty_on_map(lcd_map_t *map, int x, int y, u8 index) {
    if(map->tiles[y*32 + x] == index) {
       map->tile_dirty[y][x] = 1;
    }
}

static inline void mark_index_refs_dirty(u8 index) {
    int x, y;

    for(y = 0; y < 32; y++) {
        for(x = 0; x < 32; x++) {
            mark_index_refs_dirty_on_map(&lcd.maps[0], x, y, index);
            mark_index_refs_dirty_on_map(&lcd.maps[1], x, y, index);
        }
    }
}

#define cached_dirty(dword) map->cached_palette[ty][x][dword] != *(u32*)&lcd.bgp.d[palette*8 + dword*4]
#define cache_palette(dword) map->cached_palette[ty][x][dword] = *(u32*)&lcd.bgp.d[palette*8 + dword*4]

static inline void redraw_dirty(lcd_map_t *map, int tx, int ty) {
    int c;

    for(c = 0; c < 21; c++) {
        int x = (tx + c) % 32;

        attr = map->attr[ty * 32 + x];
        bank = attr & 0x08 ? 1 : 0;
        palette = attr & 0x07;
        tile_index = map->tiles[ty*32 + x];

        if(lcd.index_dirty[bank][tile_index]) {
            mark_index_refs_dirty(tile_index);
            lcd.index_dirty[bank][tile_index] = 0;
        }

        if(cached_dirty(0) || cached_dirty(1)) {
            map->tile_dirty[ty][x] = 1;
            cache_palette(0);
            cache_palette(1);
        }

        if(map->tile_dirty[ty][x]) {
            draw_tile(map, x, ty);
            map->tile_dirty[ty][x] = 0;
        }
    }
}

static inline void scan_bg(u16 *scan, pixel_meta_t *meta) {
    lcd_map_t *map =  &lcd.maps[lcd.c & 0x08 ? 1 : 0];

    int my = (lcd.ly + lcd.scy) % 256;
    int ex = min(256 - lcd.scx, 160);

    redraw_dirty(map, lcd.scx/8, my/8);

    memcpy(scan, &map->scan_cache[my][lcd.scx], ex * sizeof(*scan));
    memcpy(meta, &map->cache_meta[my][lcd.scx], ex * sizeof(*meta));
    memcpy(&scan[ex], &map->scan_cache[my][0], (160 - ex) * sizeof(*scan));
    memcpy(&meta[ex], &map->cache_meta[my][0], (160 - ex) * sizeof(*meta));
}


static inline void scan_wnd(u16 *scan, pixel_meta_t *meta) {
    if(lcd.wy > lcd.ly || lcd.wx > 166) {
        return;
    }
    lcd_map_t *map =  &lcd.maps[lcd.c & 0x40 ? 1 :0];

    u8 mx = -min(lcd.wx - 7, 0);
    u8 my = lcd.ly - lcd.wy;
    u8 sx = max(lcd.wx - 7, 0);

    redraw_dirty(map, mx/8, my/8);

    memcpy(&scan[sx], &map->scan_cache[my][mx], (160 - sx) * sizeof(*scan));
    memcpy(&meta[sx], &map->cache_meta[my][mx], (160 - sx) * sizeof(*meta));
}

void lcd_scan_maps(u16 *scan, pixel_meta_t *meta) {
    scan_bg(scan, meta);

    if(lcd.c & 0x20) {
        scan_wnd(scan, meta);
    }
}

void maps_tiledata_dirty(int absolute_index) {
    u8 tile;
    if(lcd.c & 0x10) {
        if(absolute_index > 255) {
            return;
        }
        tile = absolute_index;
    }
    else {
        if(absolute_index <= 128) {
            return;
        }
        tile = absolute_index - 256;
    }
    lcd.index_dirty[ram.selected_vrambank][tile] = 1;
}

void maps_tile_dirty(lcd_map_t *map, int tile) {
    map->tile_dirty[tile/32][tile%32] = 1;
}

void maps_dirty() {
    memset(lcd.maps[0].tile_dirty, 0xFF, sizeof(lcd.maps[0].tile_dirty));
    memset(lcd.maps[1].tile_dirty, 0xFF, sizeof(lcd.maps[1].tile_dirty));
}

