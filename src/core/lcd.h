#ifndef LCD_H
#define LCD_H

    typedef struct lcd_s {
        u8 stat;
        u8 creg;
        u8 scx, scy;
        u8 wx, wy;
        u8 bgp;
        u8 ly;
        u8 obp0, obp1;
    } lcd_t;


#endif
