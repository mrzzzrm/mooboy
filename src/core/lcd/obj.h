#ifndef IO_LCD_OBJ_H
#define IO_LCD_OBJ_H

    #include "util/defines.h"

    typedef struct {
        u8 data;
        u8 priority;
        u8 palette;
    } dmg_obj_scan_t;

    void lcd_render_obj_line();
    void lcd_dmg_scan_obj(dmg_obj_scan_t *scan);

#endif
