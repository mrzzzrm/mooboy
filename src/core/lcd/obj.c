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
#define PALETTEMAP(obj)  (((obj)[OBJ_FLAGS_OFFSET] & OBJ_PALETTE_BIT) ? obp1map : obp0map)

#define OBJPTR(index) (&ram.oam[(index)*OBJ_SIZE])


static u8 obp0map[4];
static u8 obp1map[4];
static u8 obj_height;
static u8 obj_size_mode;

static void render_obj_line(u8 *line, u8 tx, u8 fbx, u8 *palette, u8 bgpriority) {
    u16 fb_cursor = (lcd.ly * LCD_WIDTH) + fbx;
    u16 line_end = (lcd.ly + 1) * LCD_WIDTH;
    u8 rshift = 7 - tx;
    bgpriority = bgpriority ? 0 : 1;

    for(; tx < TILE_WIDTH && fb_cursor < line_end; tx++, fb_cursor++) {
        u8 lsb = (line[0] >> rshift) & 0x01;
        u8 msb = (line[1] >> rshift) & 0x01;

        if(lsb || msb) {
            if(bgpriority || lcd.working_fb[fb_cursor] == 0) {
                lcd.working_fb[fb_cursor] = palette[lsb + (msb << 1)];
            }
        }
        rshift--;
    }
}

static void render_obj_line_reversed(u8 *line, u8 tx, u8 fbx, u8 *palette, u8 bgpriority) {
    u16 fb_cursor = (lcd.ly * LCD_WIDTH) + fbx;
    u16 line_end = (lcd.ly + 1) * LCD_WIDTH;
    u8 rshift = tx;
    bgpriority = bgpriority ? 0 : 1;

    for(; tx < TILE_WIDTH && fb_cursor < line_end; tx++, fb_cursor++) {
        u8 lsb = (line[0] >> rshift) & 0x01;
        u8 msb = (line[1] >> rshift) & 0x01;

        if(lsb || msb) {
            if(bgpriority || lcd.working_fb[fb_cursor] == 0) {
                lcd.working_fb[fb_cursor] = palette[lsb + (msb << 1)];
            }
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

static void render_obj(u8 *obj) {
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

    line_data = &ram.vrambank[tile_index*0x10 + obj_line*0x02];
    fbx = POSX(obj) - 8;

    if(fbx < 0) {
        tx = -fbx;
        fbx = 0;
    }
    else {
        tx = 0;
    }

    if(XFLIP(obj)) {
        render_obj_line_reversed(line_data, tx, fbx, PALETTEMAP(obj), BGPRIORITY(obj));
    }
    else {
        render_obj_line(line_data, tx, fbx, PALETTEMAP(obj), BGPRIORITY(obj));
    }
}

void lcd_render_obj_line() {
    unsigned int o;
    unsigned int obj_count;
    u8 *obj_indexes[OAM_OBJ_COUNT];

    obj_size_mode = lcd.c & LCDC_OBJ_SIZE_BIT;
    obj_height = (obj_size_mode ? 15 : 7);

    obj_count = select_obj_indexes(obj_indexes);
    obj_count = establish_render_priority(obj_indexes, obj_count);

    for(o = 0; o < obj_count; o++) {
        render_obj(obj_indexes[o]);
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


