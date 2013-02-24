#include "obj.h"
#include "fb.h"
#include "mem.h"
#include "mem/io/lcd.h"

#define MAX_OBJS_PER_LINE 10
#define OAM_OBJ_COUNT 40

#define OBJ_POSY_OFFSET 0
#define OBJ_POSX_OFFSET 1
#define OBJ_INDEX_OFFSET 2
#define OBJ_FLAGS_OFFSET 3

#define OBJ_YFLIP_BIT 0x40
#define OBJ_XFLIP_BIT 0x20
#define OBJ_PALETTE_BIT 0x10

#define OBJ_BYTES_IN_OAM 4


static u8 obp0map[4];
static u8 obp1map[4];

static unsigned int select_obj_indexes(u8 *buf) {
    u8 oam_pos, buf_index;

    buf_index = 0;
    for(oam_pos = 0; oam_pos < OAM_OBJ_COUNT; oam_pos += OBJ_BYTES_IN_OAM) {
        s16 top_line = (s16)ram.oam[oam_pos + OBJ_POSY_OFFSET] - 16;
        s16 bottom_line = top_line + (lcd.c & LCDC_OBJ_SIZE_BIT ? 15 : 7);

        if(lcd.ly >= top_line && lcd.ly <= bottom_line) {
            buf[buf_index++] = ram.oam[oam_pos + OBJ_INDEX_OFFSET];
            if(buf_index >= MAX_OBJS_PER_LINE) {
                break;
            }
        }
    }

    return buf_index;
}

static void establish_draw_priority(u8 *obj_indexes, unsigned int count) {
    u8 switched;

    do {
        u8 o;

        switched = 0;
        for(o = 0; o + 1 < count; o++) {
            u8 x1 = ram.oam[obj_indexes[o]*OBJ_BYTES_IN_OAM + OBJ_POSX_OFFSET];
            u8 x2 = ram.oam[obj_indexes[o+1]*OBJ_BYTES_IN_OAM + OBJ_POSX_OFFSET];

            if(x2 < x1) {
                u8 tmp = obj_indexes[o];
                obj_indexes[o] = obj_indexes[o+1];
                obj_indexes[o+1] = tmp;
                switched = 1;
            }
        }
    } while(switched);
}

static void draw_obj(u8 index) {
    u8 obj_line;
    u8 *line_data;
    s16 fbx;
    u8 *palette_map;

    obj_line = lcd.ly - (ram.oam[index*OBJ_BYTES_IN_OAM + OBJ_POSY_OFFSET] - 16);
    if(ram.oam[index*OBJ_BYTES_IN_OAM + OBJ_FLAGS_OFFSET] & OBJ_YFLIP_BIT) {
        obj_line = (lcd.c & LCDC_OBJ_SIZE_BIT ? 15 : 7) - obj_line;
    }

    line_data = &mbc.vrambank[ram.oam[index*OBJ_BYTES_IN_OAM + OBJ_INDEX_OFFSET] * 16];
    fbx = ram.oam[index*OBJ_BYTES_IN_OAM + OBJ_POSX_OFFSET] - 8;
    palette_map = ram.oam[index*OBJ_BYTES_IN_OAM + OBJ_FLAGS_OFFSET] & OBJ_PALETTE_BIT ? obp1map : obp0map;

    if(ram.oam[index*OBJ_BYTES_IN_OAM + OBJ_FLAGS_OFFSET] & OBJ_XFLIP_BIT) {
        lcd_render_tile_line_reversed(line_data, 0, 0, fbx, palette_map, 1);
    }
    else {
        lcd_render_tile_line(line_data, 0, 0, fbx, palette_map, 1);
    }
}

void lcd_render_obj_line() {
    unsigned int o;
    unsigned int obj_count;
    u8 obj_indexes[MAX_OBJS_PER_LINE];

    obj_count = select_obj_indexes(obj_indexes);

    establish_draw_priority(obj_indexes, obj_count);

    for(o = 0; o < obj_count; o++) {
        draw_obj(obj_indexes[o]);
    }
}

static void obpmap_dirty(u8 obp, u8 *obpmap) {
    u8 rc;
    for(rc = 0; rc < 4; rc++) {
        obpmap[rc] = (obp & (0x3 << (rc<<1))) >> (rc<<1);
    }
}

void lcd_obp0map_dirty() {
    obpmap_dirty(lcd.obp0, obp0map);
}

void lcd_obp1map_dirty() {
    obpmap_dirty(lcd.obp1, obp1map);
}


