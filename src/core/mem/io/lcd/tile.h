#ifndef IO_LCD_TILE_H
#define IO_LCD_TILE_H

    #include "util/defines.h"

    void lcd_render_tile_line(u8 *line, u8 tx, u8 ty, u8 fbx, u8 *palette, int crap);
    void lcd_render_tile_line_reversed(u8 *line, u8 tx, u8 ty, u8 fbx, u8 *palette, int crap);

#endif
