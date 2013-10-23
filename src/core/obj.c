#include "obj.h"
#include "moo.h"
#include "mem.h"
#include "lcd.h"
#include "defines.h"
#include <string.h>
#include <stdio.h>

#define MAX_PER_LINE 10
#define OBJ_SIZE 4
#define OAM_SIZE 0xA0
#define OAM_OBJ_COUNT 40

#define POSY_OFFSET 0
#define POSX_OFFSET 1
#define INDEX_OFFSET 2
#define FLAGS_OFFSET 3

#define YFLIP_BIT 0x40
#define XFLIP_BIT 0x20
#define BANK_MASK 0x08
#define BANK_SHIFT 3

#define POSX(obj) (obj[POSX_OFFSET])
#define POSY(obj) (obj[POSY_OFFSET])
#define TILE(obj) (obj[INDEX_OFFSET])
#define XFLIP(obj)  ((obj)[FLAGS_OFFSET] & XFLIP_BIT)
#define YFLIP(obj)  ((obj)[FLAGS_OFFSET] & YFLIP_BIT)
#define BANK(obj) (((obj)[FLAGS_OFFSET] & BANK_MASK) >> BANK_SHIFT)


static u8 obj_height;
static u8 obj_size_mode;
static u8 priority;
static u8 palette;
static u16 *scan;
static pixel_meta_t *meta;
static obj_range_t *ranges;
static int obj_count;
static u8 *objs[OAM_OBJ_COUNT];

static inline void render_pixel(u8 *line, u8 rshift, u8 sx) {
    u8 lsb = (line[0] >> rshift) & 0x01;
    u8 msb = (line[1] >> rshift) & 0x01;
    u8 color_id = lsb + (msb << 1);

    if(color_id != 0) {
        meta[sx].color_id = color_id;
        meta[sx].priority = priority;
        scan[sx] = lcd.obp.map[palette][color_id];
    }
}

static inline void render_obj_line(u8 *line, u8 tx, u8 sx) {
    s8 rshift;
    for(rshift = 7 - tx; rshift >= 0 && sx < LCD_WIDTH; rshift--, sx++) {
        render_pixel(line, rshift, sx);
    }
}

static inline void render_obj_line_flipped(u8 *line, u8 tx, u8 sx) {
    u8 rshift;
    for(rshift = tx; rshift < 8 && sx < LCD_WIDTH; rshift++, sx++) {
        render_pixel(line, rshift, sx);
    }
}

static void select_obj_indexes() {
    u8 buf_index;
    u8 *obj, *oam_end;

    buf_index = 0;
    oam_end = ram.oam + OAM_SIZE;
    for(obj = ram.oam; obj < oam_end; obj += OBJ_SIZE) {
        s16 top_line = (s16)POSY(obj) - 16;
        s16 bottom_line = top_line + obj_height;

        if((s16)lcd.ly >= top_line && (s16)lcd.ly <= bottom_line) {
            objs[buf_index++] = obj;
        }
    }

    obj_count = buf_index;
}

#define sort(objs, cmp) { \
    u8 switched; \
    do { \
        u8 o; \
        switched = 0; \
        for(o = 0; o + 1 < obj_count; o++) { \
            if(POSX(objs[o]) cmp POSX(objs[o+1])) { \
                u8 *tmp = objs[o]; \
                objs[o] = objs[o+1]; \
                objs[o+1] = tmp; \
                switched = 1; \
            } \
        } \
    } while(switched); \
}

static int compute_ranges() {
    u8 *sorted_objs[OAM_OBJ_COUNT];
    int num = 0;

    memcpy(sorted_objs, objs, sizeof(*sorted_objs) * obj_count);
    sort(sorted_objs, >);

    int r, o;
    int last_range_end = 0;

    for(o = 0, r = 0; o < obj_count; o++, r++) {
        if(POSX(sorted_objs[o]) >= 168) {
            break;
        }

        ranges[r].diff = max(POSX(sorted_objs[o]) - 8 - last_range_end, 0);
        ranges[r].end = min(POSX(sorted_objs[o]), 160);

        for(; o < obj_count-1; o++) {
            if(POSX(sorted_objs[o+1]) - (ranges[r].end + 8) < 8) {
                ranges[r].end = min(POSX(sorted_objs[o+1]), 160);
            }
            else {
                break;
            }
        }
        num++;
        last_range_end = ranges[r].end;
    }

    return num;
}

static void render_obj(u8 *obj) {
    u8 *line_data;
    u8 obj_line, tile_index;
    s16 sx;
    u8 tx;

    obj_line = lcd.ly - (POSY(obj) - 16);
    if(YFLIP(obj)) {
        obj_line = obj_height - obj_line;
    }

    tile_index = TILE(obj);
    if(obj_size_mode) {
        tile_index &= 0xFE;
    }

    line_data = &ram.vrambanks[moo.mode == CGB_MODE ? BANK(obj) : 0][tile_index*0x10 + obj_line*0x02];
    sx = POSX(obj) - 8;

    if(sx < 0) {
        tx = -sx;
        sx = 0;
    }
    else {
        tx = 0;
    }

    priority = obj[FLAGS_OFFSET] & 0x80;
    palette = moo.mode == CGB_MODE ? obj[FLAGS_OFFSET] & 0x07 : (obj[FLAGS_OFFSET] >> 4) & 0x01;

    if(XFLIP(obj)) {
        render_obj_line_flipped(line_data, tx, sx);
    }
    else {
        render_obj_line(line_data, tx, sx);
    }
}


void lcd_scan_obj(u16 *_scan, pixel_meta_t *_meta, obj_range_t *_ranges, int *num_obj_ranges) {
    scan = _scan;
    meta = _meta;
    ranges = _ranges;

    obj_size_mode = lcd.c & LCDC_OBJ_SIZE_BIT;
    obj_height = (obj_size_mode ? 15 : 7);

    select_obj_indexes();
    *num_obj_ranges = compute_ranges();

    if(moo.mode == NON_CGB_MODE) {
        sort(objs, <);
    }

    obj_count = obj_count > MAX_PER_LINE ? MAX_PER_LINE : obj_count;

    int o;
    for(o = obj_count-1; o >= 0; o--) {
        render_obj(objs[o]);
    }
}


