#include "obj.h"
#include "mem.h"
#include "lcd.h"

#define MAX_PER_LINE 10
#define OBJ_SIZE 4
#define OAM_SIZE 0xA0
#define OAM_OBJ_COUNT 40

#define OBJ_POSY_OFFSET 0
#define OBJ_POSX_OFFSET 1
#define OBJ_INDEX_OFFSET 2
#define OBJ_FLAGS_OFFSET 3

#define OBJ_BG_PRIORITY_BIT 0x80
#define OBJ_YFLIP_BIT 0x40
#define OBJ_XFLIP_BIT 0x20
#define OBJ_PALETTE_BIT 0x10

#define POSX(obj) (obj[OBJ_POSX_OFFSET])
#define POSY(obj) (obj[OBJ_POSY_OFFSET])
#define TILE(obj) (obj[OBJ_INDEX_OFFSET])

#define XFLIP(obj)  ((obj)[OBJ_FLAGS_OFFSET] & OBJ_XFLIP_BIT)
#define YFLIP(obj)  ((obj)[OBJ_FLAGS_OFFSET] & OBJ_YFLIP_BIT)
#define BGPRIORITY(obj)  ((obj)[OBJ_FLAGS_OFFSET] & OBJ_BG_PRIORITY_BIT)
#define PALETTEMAP(obj)  (((obj)[OBJ_FLAGS_OFFSET] & OBJ_PALETTE_BIT) ? lcd.obp_map[1] : lcd.obp_map[0])

#define OBJPTR(index) (&ram.oam[(index)*OBJ_SIZE])

static u8 obj_height;
static u8 obj_size_mode;

static void render_obj_line(u8 *line, u8 tx, u8 fbx, dmg_obj_scan_t *scan, u8 priority, u8 palette) {
    u16 scan_cursor = fbx;
    u8 rshift = 7 - tx;

    for(; tx < TILE_WIDTH && scan_cursor < LCD_WIDTH; tx++, scan_cursor++) {
        u8 lsb = (line[0] >> rshift) & 0x01;
        u8 msb = (line[1] >> rshift) & 0x01;
        u8 pixel =  lsb + (msb << 1);
        if(pixel != 0) {
            scan[scan_cursor].data = lsb + (msb << 1);
            scan[scan_cursor].priority = priority;
            scan[scan_cursor].palette = palette;
        }
        rshift--;
    }
}

static void render_obj_line_flipped(u8 *line, u8 tx, u8 fbx, dmg_obj_scan_t *scan, u8 priority, u8 palette) {
    u16 scan_cursor =  fbx;
    u8 rshift = tx;

    for(; tx < TILE_WIDTH && scan_cursor < LCD_WIDTH; tx++, scan_cursor++) {
        u8 lsb = (line[0] >> rshift) & 0x01;
        u8 msb = (line[1] >> rshift) & 0x01;
        u8 pixel =  lsb + (msb << 1);
        if(pixel != 0) {
            scan[scan_cursor].data = lsb + (msb << 1);
            scan[scan_cursor].priority = priority;
            scan[scan_cursor].palette = palette;
        }
        rshift++;
    }
}


static unsigned int select_obj_indexes(u8 **buf) {
    u8 buf_index;
    u8 *obj, *oam_end;

    buf_index = 0;
    oam_end = ram.oam + OAM_SIZE;
    for(obj = ram.oam; obj < oam_end; obj += OBJ_SIZE) {
        s16 top_line = (s16)POSY(obj) - 16;
        s16 bottom_line = top_line + obj_height;

        if((s16)lcd.ly >= top_line && (s16)lcd.ly <= bottom_line) {
            buf[buf_index++] = obj;
        }
    }

    return buf_index;
}

static unsigned int establish_render_priority(u8 **objs, unsigned int count) {
    u8 switched;

    do {
        u8 o;
        switched = 0;
        for(o = 0; o + 1 < count; o++) {
            if(POSX(objs[o+1]) < POSX(objs[o])) {
                u8 *tmp = objs[o];
                objs[o] = objs[o+1];
                objs[o+1] = tmp;
                switched = 1;
            }
        }
    } while(switched);

    return count > MAX_PER_LINE ? MAX_PER_LINE : count;
}


static void render_obj(u8 *obj, dmg_obj_scan_t *scan) {
    u8 *line_data;
    u8 obj_line, tile_index;
    s16 fbx;
    u8 tx;

    obj_line = lcd.ly - (POSY(obj) - 16);
    if(YFLIP(obj)) {
        obj_line = obj_height - obj_line;
    }

    tile_index = TILE(obj);
    if(obj_size_mode) {
        tile_index &= 0xFE;
    }

    line_data = &ram.vrambanks[0][tile_index*0x10 + obj_line*0x02];
    fbx = POSX(obj) - 8;

    if(fbx < 0) {
        tx = -fbx;
        fbx = 0;
    }
    else {
        tx = 0;
    }

    if(XFLIP(obj)) {
        render_obj_line_flipped(line_data, tx, fbx, scan, BGPRIORITY(obj), (obj)[OBJ_FLAGS_OFFSET] & OBJ_PALETTE_BIT ? 1 : 0);
    }
    else {
        render_obj_line(line_data, tx, fbx, scan, BGPRIORITY(obj), (obj)[OBJ_FLAGS_OFFSET] & OBJ_PALETTE_BIT ? 1 : 0);
    }
}

void lcd_render_obj_line() {
//    unsigned int o;
//    unsigned int obj_count;
//    u8 *obj_indexes[OAM_OBJ_COUNT];
//
//    obj_size_mode = lcd.c & LCDC_OBJ_SIZE_BIT;
//    obj_height = (obj_size_mode ? 15 : 7);
//
//    obj_count = select_obj_indexes(obj_indexes);
//    obj_count = establish_render_priority(obj_indexes, obj_count);
//
//    for(o = 0; o < obj_count; o++) {
//        render_obj(obj_indexes[o]);
//    }
}

void lcd_dmg_scan_obj(dmg_obj_scan_t *scan) {
    unsigned int o;
    unsigned int obj_count;
    u8 *obj_indexes[OAM_OBJ_COUNT];

    obj_size_mode = lcd.c & LCDC_OBJ_SIZE_BIT;
    obj_height = (obj_size_mode ? 15 : 7);

    obj_count = select_obj_indexes(obj_indexes);
    obj_count = establish_render_priority(obj_indexes, obj_count);

    for(o = 0; o < obj_count; o++) {
        render_obj(obj_indexes[o], scan);
    }
}


