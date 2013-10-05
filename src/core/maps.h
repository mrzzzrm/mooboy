#ifndef CORE_MAPS_H
#define CORE_MAPS_H

    #include "defines.h"
    #include "lcd.h"

    #define MAPS_DATA_MASK 0x03
    #define MAPS_PALETTE_MASK 0x1C
    #define MAPS_PALETTE_SHIFT 2
    #define MAPS_PRIORITY_BIT 0x20

    #define MAPS_PALETTE(p) (((p) & MAPS_PALETTE_MASK) >> MAPS_PALETTE_SHIFT)
    #define MAPS_DATA(p) ((p) & MAPS_DATA_MASK)
    #define MAPS_PRIORITY(p) ((p) & MAPS_PRIORITY_BIT)

   // void lcd_dmg_scan_maps(u8 *scan);
    void lcd_scan_maps(scan_pixel_t *scan);

#endif
