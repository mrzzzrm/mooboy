#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <SDL/SDL.h>
#include "sys/sys.h"
#include "sys/sdl/framerate.h"
#include "core/cpu.h"
#include "core/joy.h"
#include "core/rtc.h"
#include "core/mbc.h"
#include "core/moo.h"
#include "core/mem.h"
#include "core/timers.h"
#include "core/lcd.h"
#include "core/defines.h"

#define S(val) save((val), sizeof((val)))
#define SV(a) {int i; for(i = 0; i < sizeof((a))/sizeof(*(a)); i++) {S((a)[i]);} }

#define R(val) val = load(sizeof(val))
#define RV(a) {int i; for(i = 0; i < sizeof((a))/sizeof(*(a)); i++) {R((a)[i]);} }

#define BYTE(val) ((u8)(val))

#define CHECKPOINTS 32

#define set_checkpoint() _set_checkpoint(__LINE__)
#define assert_checkpoint() _assert_checkpoint(__LINE__)

FILE *f;
static u8 byte;
static u8 checkpoints[CHECKPOINTS];
static int current_checkpoint;
static int bytes_handled;

static void save(u32 val, u8 size) {
    int b;
    for(b = 0; b < size; b++) {
        u8 byte = val & 0xFF;
        fwrite(&byte, 1, 1, f);
        //printf(".");
        val >>= 8;
        bytes_handled++;
    }
}

static u32 load(u8 size) {
    u32 re = 0;
    int b;
    for(b = 0; b < size; b++) {
        assert(fread(&byte, 1, 1, f) == 1);
       // printf(".", byte);
        re |= byte << (8*b);
        bytes_handled++;
    }

    return re;
}

static void _set_checkpoint(int line) {
    assert(current_checkpoint < CHECKPOINTS);
    //fprintf(stderr, "SET CHCEKPOINT %i@%i - %i bytes written", current_checkpoint, line, bytes_handled);
    S(checkpoints[current_checkpoint]);
    //printf("\n");
    current_checkpoint++;
}

static void _assert_checkpoint(int line) {
    //fprintf(stderr, "ASSERT CHCEKPOINT %i@%i - %i bytes read\n", current_checkpoint, line, bytes_handled);
    if(current_checkpoint >= CHECKPOINTS) {
        fprintf(stderr,  "\nSavestate corrupt in line %i: too many checkpoints\n", line);
        fclose(f);
        assert(0);
    }
    R(byte);
    if(byte != checkpoints[current_checkpoint]) {
        fprintf(stderr,  "\nSavestate corrupt in line %i: wrong byte\n", line);
        fclose(f);
        assert(0);
    }
    current_checkpoint++;
}

void save_cpu() {
    S(A); S(F); S(B); S(C); S(D); S(E); S(HL);
    S(SP); S(PC);
    S(cpu.ime); S(cpu.irq); S(cpu.ie);
    S(cpu.freq);
    S(cpu.freq_switch);
    S(cpu.halted);
    set_checkpoint();
}

static void load_cpu() {
    R(A); R(F); R(B); R(C); R(D); R(E); R(HL);
    R(SP); R(PC);
    R(cpu.ime); R(cpu.irq); R(cpu.ie);
    R(cpu.freq);
    R(cpu.freq_switch);
    R(cpu.halted);
    assert_checkpoint();
}

static void save_joy() {
    S(joy.col);
    set_checkpoint();
}

static void load_joy() {
    joy.state = 0xFF;
    R(joy.col);
    assert_checkpoint();
}

static void save_fb() {
    int f;
    for(f = 0; f < 2; f++) {
        SV(lcd.fb[f]);
    }
    set_checkpoint();

    S(BYTE(lcd.clean_fb == lcd.fb[0] ? 0 : 1));
    S(BYTE(lcd.working_fb == lcd.fb[0] ? 0 : 1));
    set_checkpoint();
}

static void load_fb() {
    int f;
    for(f = 0; f < 2; f++) {
        RV(lcd.fb[f]);
    }
    assert_checkpoint();

    R(byte); lcd.clean_fb = lcd.fb[byte];
    R(byte); lcd.working_fb = lcd.fb[byte];
    assert_checkpoint();
}

static void save_lcd_maps() {

}

static void load_lcd_maps() {
    int p;

    lcd_c_dirty();
    lcd_obp0_dirty();
    lcd_obp1_dirty();
    lcd_bgp_dirty();

    for(p = 0; p <= 0x3F; p++) {
        lcd_bgpd_dirty(p);
        lcd_obpd_dirty(p);
    }
}

static void save_lcd() {
    S(lcd.c);
    S(lcd.stat);
    S(lcd.scx); S(lcd.scy);
    S(lcd.ly); S(lcd.lyc);
    S(lcd.wx); S(lcd.wy);
    S(lcd.bgp); SV(lcd.obp);
    SV(lcd.bgpd); SV(lcd.obpd);
    S(lcd.bgps); S(lcd.bgpi);
    S(lcd.obps); S(lcd.obpi);
    set_checkpoint();
    save_fb();
    S(lcd.hdma_source); S(lcd.hdma_dest);
    S(lcd.hdma_length); S(lcd.hdma_inactive);
    save_lcd_maps();
    set_checkpoint();
}

static void load_lcd() {
    R(lcd.c);
    R(lcd.stat);
    R(lcd.scx); R(lcd.scy);
    R(lcd.ly);
    R(lcd.lyc);
    R(lcd.wx); R(lcd.wy);
    R(lcd.bgp); RV(lcd.obp);
    RV(lcd.bgpd); RV(lcd.obpd);
    R(lcd.bgps); R(lcd.bgpi);
    R(lcd.obps); R(lcd.obpi);
    assert_checkpoint();
    load_fb();
    R(lcd.hdma_source); R(lcd.hdma_dest);
    R(lcd.hdma_length); R(lcd.hdma_inactive);
    load_lcd_maps();
    assert_checkpoint();
}

static void save_mbc() {
    S(mbc.type);
    // lower_write_func is set on rom-load
    S(BYTE(((u8(*)[0x4000])mbc.rombank - card.rombanks)));
    S(BYTE(((u8(*)[0x2000])mbc.srambank - card.srambanks)));
    S(mbc.has_rtc);
    S(mbc.has_battery);

    S(mbc1.mode);
    S(mbc1.rombank);

    S(mbc3.mode);

    S(mbc5.rombank);
    set_checkpoint();
}

static void load_mbc() {
    R(mbc.type);
    // lower_write_func is set on rom-load
    R(byte); mbc.rombank = card.rombanks[byte];
    R(byte); mbc.srambank = card.srambanks[byte];
    R(mbc.has_rtc);
    R(mbc.has_battery);

    R(mbc1.mode);
    R(mbc1.rombank);

    R(mbc3.mode);

    R(mbc5.rombank);
    assert_checkpoint();
}

static void save_mem() {
    int b;

    S(card.romsize);
    set_checkpoint();
    S(card.sramsize);
    set_checkpoint();
    for(b = 0; b < card.sramsize; b++) {
        SV(card.srambanks[b]);
    }
    set_checkpoint();
    for(b = 0; b < card.romsize; b++) {
        SV(card.rombanks[b]);
    }
    set_checkpoint();

    for(b = 0; b < 8; b++) {
        SV(ram.rambanks[b]);
    }
    set_checkpoint();
    for(b = 0; b < 2; b++) {
        SV(ram.vrambanks[b]);
    }
    set_checkpoint();

    SV(ram.hram);
    SV(ram.oam);
    S(BYTE(((u8(*)[0x1000])ram.rambank - ram.rambanks)));
    S(BYTE(((u8(*)[0x2000])ram.vrambank - ram.vrambanks)));
    S(ram.rambank_index);
    set_checkpoint();
}

static void load_mem() {
    int b;

    R(card.romsize);
    assert_checkpoint();
    R(card.sramsize);
    assert_checkpoint();
    for(b = 0; b < card.sramsize; b++) {
        RV(card.srambanks[b]);
    }
    assert_checkpoint();
    for(b = 0; b < card.romsize; b++) {
        RV(card.rombanks[b]);
    }
    assert_checkpoint();


    for(b = 0; b < 8; b++) {
        RV(ram.rambanks[b]);
    }
    assert_checkpoint();
    for(b = 0; b < 2; b++) {
        RV(ram.vrambanks[b]);
    }
    assert_checkpoint();

    RV(ram.hram);
    RV(ram.oam);
    R(byte); ram.rambank = ram.rambanks[byte];
    R(byte); ram.vrambank = ram.vrambanks[byte];
    R(ram.rambank_index);
    assert_checkpoint();
}

static void save_rtc() {
    SV(rtc.latched);
    SV(rtc.ticking);
    S(rtc.mapped);
    S(rtc.prelatched);
    S(rtc.cc);
    set_checkpoint();
}

static void load_rtc() {
    RV(rtc.latched);
    RV(rtc.ticking);
    R(rtc.mapped);
    R(rtc.prelatched);
    R(rtc.cc);
    assert_checkpoint();
}

static void save_sound() {

}

static void load_sound() {

}

static void save_timers() {
    S(timers.div); S(timers.tima); S(timers.tma); S(timers.tac);
    S(timers.div_cc); S(timers.tima_cc);
    set_checkpoint();
}

static void load_timers() {
    R(timers.div); R(timers.tima); R(timers.tma); R(timers.tac);
    R(timers.div_cc); R(timers.tima_cc);
    assert_checkpoint();
}

static void save_sys() {
    S(sys.ticks);
    set_checkpoint();
}

static void load_sys() {
    R(sys.ticks);
    sys.ticks_diff = sys.ticks - SDL_GetTicks();
    sys_pause();
    assert_checkpoint();
}

static void init_checkpoints() {
    srand(time(NULL));
    int cp;
    S(BYTE(CHECKPOINTS));
    for(cp = 0; cp < CHECKPOINTS; cp++) {
        checkpoints[cp] = rand() & 0xFF;
        S(checkpoints[cp]);
    }
    current_checkpoint = 0;
    bytes_handled = 0;
}

static void load_checkpoints() {
    int cp;
    R(byte);
    assert(byte == CHECKPOINTS);
    for(cp = 0; cp < CHECKPOINTS; cp++) {
        R(byte);
        checkpoints[cp] = byte;
    }
    current_checkpoint = 0;
    bytes_handled = 0;
}

void state_save(const char *filename) {
    printf("Saving %s\n", filename);
    f = fopen(filename, "wb");
    assert(f);

    init_checkpoints();

    save_cpu();
    save_joy();
    save_lcd();
    save_mbc();
    save_mem();
    save_rtc();
    save_sound();
    save_timers();
    save_sys();

    fclose(f);
}

int state_load(const char *filename) {
    printf("Loading %s\n", filename);
    f = fopen(filename, "rb");
    assert(f);

    load_checkpoints();

    load_cpu();
    load_joy();
    load_lcd();
    load_mbc();
    load_mem();
    load_rtc();
    load_sound();
    load_timers();
    load_sys();

    assert(fread(&byte, 1, 1, f) == 0);

    fclose(f);

    return 0;
}

