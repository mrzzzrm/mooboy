#ifndef IO_LCD_OBJ_H
#define IO_LCD_OBJ_H

#include "util/defines.h"

#define OBJ_PALETTE_MASK 0x07
#define OBJ_PALETTE_SHIFT 2
#define OBJ_DATA_MASK 0x03
#define OBJ_PRIORITY_BIT 0x08

#define OBJ_DATA(o) ((o) & OBJ_DATA_MASK)
#define OBJ_PALETTE(o) (((o) >> OBJ_PALETTE_SHIFT) & OBJ_PALETTE_MASK)
#define OBJ_PRIORITY(o) ((o) & OBJ_PRIORITY_BIT)

void lcd_scan_obj(u8 *scan);

#endif
