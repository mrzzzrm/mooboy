#ifndef CORE_OBJ_H
#define CORE_OBJ_H

#include "defines.h"
#include "lcd.h"

#define OBJ_PALETTE_MASK 0x07
#define OBJ_PALETTE_SHIFT 2
#define OBJ_DATA_MASK 0x03
#define OBJ_PRIORITY_BIT 0x20

#define OBJ_DATA(o) ((o) & OBJ_DATA_MASK)
#define OBJ_PALETTE(o) (((o) >> OBJ_PALETTE_SHIFT) & OBJ_PALETTE_MASK)
#define OBJ_PRIORITY(o) ((o) & OBJ_PRIORITY_BIT)

typedef struct {
    int diff;
    int end;
} obj_range_t;

void lcd_scan_obj(u16 *scan, pixel_meta_t *meta, obj_range_t *ranges, int *num_obj_ranges);

#endif
