#include "lcd.h"

lcd_t lcd;

void lcd_reset() {
    lcd.lcdc = 0;
    lcd.stat = 0;
    lcd.scx = 0;
    lcd.scy = 0;
    lcd.ly = 0;
    lcd.lyc = 0;
    lcd.wx = 0;
    lcd.wy = 0;
    lcd.bgp = 0;
    lcd.obp0 = 0;
    lcd.obp1 = 0;
}

void lcd_dma(u8 v) {

}

void lcd_control(u8 v) {

}

void lcd_stat(u8 v) {

}

void lcd_drawl() {
    for
}
