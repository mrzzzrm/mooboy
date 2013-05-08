#include "state.h"
#include <stdio.h>
#include <assert.h>
#include "core/cpu.h"
#include "core/joy.h"
#include "core/lcd.h"
#include "core/defines.h"
#include "util/defines.h"

#define S(val) save((val), sizeof((val)))
#define SV(a) {int i; for(i = 0; i < sizeof((a))/sizeof(*(a)); i++) {S((a)[i]);} }

FILE *f;

void save(u32 val, u8 size) {
    int b;
    for(b = 0; b < size; b++) {
        u8 byte = val & 0xFF;
        fwrite(&byte, 1, 1, f);
        val >> 8;
    }
}

void save_cpu() {
    S(A); S(F); S(B); S(C); S(D); S(E);
    S(SP); S(PC);
    S(cpu.ime); S(cpu.irq); S(cpu.ie);
    S(cpu.cc); S(cpu.dfcc); S(cpu.nfcc);
    S(cpu.freq);
    S(cpu.freq_switch);
    S(cpu.halted);
}

void save_joy() {
    S(joy.state); S(joy.col);
}

void save_fb() {
    int f, p;
    for(f = 0; f < 2; f++) {
        for(p = 0; p < 144*166; p++) {
            S(lcd.fb[f][p]);
        }
    }

    S(lcd.clean_fb == lcd.fb[0] ? 0x00 : 0x01);
    S(lcd.working_fb == lcd.fb[0] ? 0x00 : 0x01);
}

void save_lcd_maps() {
    int i;

    S(lcd.bg_map == &ram.vrambanks[0][0x1C00] ? 0x00 : 0x01);
    S(lcd.wnd_map == &ram.vrambanks[0][0x1C00] ? 0x00 : 0x01);
    S(lcd.bg_attr_map == &ram.vrambanks[1][0x1C00] ? 0x00 : 0x01);
    S(lcd.wnd_attr_map == &ram.vrambanks[1][0x1C00] ? 0x00 : 0x01);
    SV(lcd.bgp_map);
    SV(lcd.obp_map[0] : )
    i = 0;
    for(i = 0; i < 2)
}

void save_lcd() {
    S(lcd.c);
    S(lcd.stat);
    S(lcd.scx); S(lcd.scy);
    S(lcd.ly); S(lcd.lyc);
    S(lcd.wx); S(lcd.wy);
    S(lcd.bgp); SV(lcd.obp);
    SV(lcd.bgpd); SV(lcd.obpd);
    S(lcd.bgps); S(lcd.bgpi);
    S(lcd.obps); S(lcd.obpi);
    save_fb();
    S(lcd.dma_source); S(lcd.dma_dest);
    S(lcd.dma_length); S(lcd.dma_hblank_inactive);
    save_lcd_maps();
}

void save_mbc() {

}

void save_mem() {

}

void save_rtc() {

}

void save_sound() {

}

void save_timers() {

}


void flush() {

}

void save_state() {
    f = fopen("state.sav", "w");
    assert(f);

    save_cpu();
    save_joy();
    save_lcd();
    save_mbc();
    save_mem();
    save_rtc();
    save_sound();
    save_timers();

    flush();
    fclose(f);
}

void load_state() {

}

