#include "mem/io/lcd.h"
#include "tile.h"

void lcd_render_tile_line(u8 *line, u8 tx, u8 ty, u8 fbx, u8 *palette, int crap) {
    u16 fb_cursor = (lcd.ly * LCD_WIDTH) + fbx;
    u16 line_end = (lcd.ly + 1) * LCD_WIDTH;
    u8 rshift = 7 - tx;

    for(; tx < TILE_WIDTH && fb_cursor < line_end; tx++, fb_cursor++) {
        u8 lsb = (line[0] >> rshift) & 0x01;
        u8 msb = (line[1] >> rshift) & 0x01;

        lcd.working_fb[fb_cursor] = palette[lsb + (msb << 1)];// + crap*4;
        rshift--;
    }
}
