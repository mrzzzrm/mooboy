#ifndef IO_LCD_H
#define IO_LCD_H

    #include "util/defines.h"

    typedef struct {
        u8 lcdc;
        u8 stat;
        u8 scx, scy;
        u8 ly;
        u8 lyc;
        u8 wx, wy;
        u8 bgp, obp0, obp1;
        /* TODO: CGB registers */
    } lcd_t;

    extern lcd_t lcd;

    void lcd_reset();

    void lcd_dma(u8 v);
    void lcd_control(u8 v);
    void lcd_stat(u8 v);
    void lcd_drawl();

#endif
