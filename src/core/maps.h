#ifndef CORE_MAPS_H
#define CORE_MAPS_H

    #include "defines.h"
    #include "lcd.h"

    void lcd_scan_maps(u16 *scan, pixel_meta_t *meta);
    void maps_tiledata_dirty(int tileindex);
    void maps_tile_dirty(lcd_map_t *map, int tile);
    void maps_dirty();

#endif
