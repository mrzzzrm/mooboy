#ifndef IO_LCD_H
#define IO_LCD_H

    #include "util/defines.h"

    typedef struct {
        u8 c;
        u8 stat;
        u8 scx, scy;
        u8 ly;
        u8 lyc;
        u8 wx, wy;
        u8 bgp, obp0, obp1;
        /* TODO: CGB registers */

        u8 fb[2][144][160];
        u8 **clean_fb;
        u8 **working_fb;
    } lcd_t;

    extern lcd_t lcd;

    void lcd_reset();
    void lcd_step();

    void lcd_dma(u8 v);

#endif
