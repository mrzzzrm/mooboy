#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include "sys/sys.h"
#include "util/framerate.h"
#include "core/cpu.h"
#include "core/joy.h"
#include "core/rtc.h"
#include "core/mbc.h"
#include "core/moo.h"
#include "core/mem.h"
#include "core/timers.h"
#include "core/sound.h"
#include "core/lcd.h"
#include "core/defines.h"

#define S(val) save((val), sizeof((val)))
#define SV(a) {int i; for(i = 0; i < sizeof((a))/sizeof(*(a)); i++) {S((a)[i]);} }

#define R(val) val = load(sizeof(val))
#define RV(a) {int i; for(i = 0; i < sizeof((a))/sizeof(*(a)); i++) {R((a)[i]);} }

#define BYTE(val) ((u8)(val))

#define CHECKPOINTS 40

#define set_checkpoint() _set_checkpoint(__LINE__)
#define assert_checkpoint() _assert_checkpoint(__LINE__)

#define LCD_MODE_0_EVENT_ID 0
#define LCD_MODE_1_EVENT_ID 1
#define LCD_MODE_2_EVENT_ID 2
#define LCD_MODE_3_EVENT_ID 3
#define LCD_VBLANK_LINE_EVENT_ID 4
#define SOUND_MIX_EVENT_ID 5
#define SOUND_SWEEP_EVENT_ID 6
#define SOUND_ENVELOPES_EVENT_ID 7
#define SOUND_LENGTH_COUNTERS_EVENT_ID 8
#define TIMERS_TIMA_EVENT_ID 9
#define TIMER_DIV_EVENT_ID 10
#define RTC_EVENT_ID 11
#define NUM_HW_EVENTS 12

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
        val >>= 8;
        bytes_handled++;
    }
}

static u32 load(u8 size) {
    u32 re = 0;
    int b;
    for(b = 0; b < size; b++) {
        if(fread(&byte, 1, 1, f) != 1) {
            moo_errorf("Savestate corrupt #1");
        }
        re |= byte << (8*b);
        bytes_handled++;
    }

    return re;
}

static void _set_checkpoint(int line) {
    assert(current_checkpoint < CHECKPOINTS);
    S(checkpoints[current_checkpoint]);
    current_checkpoint++;
}

static void _assert_checkpoint(int line) {
    if(current_checkpoint >= CHECKPOINTS) {
        moo_errorf("Savestate corrupt in line %i: too many checkpoints", line);
        return;
    }
    R(byte);
    if(byte != checkpoints[current_checkpoint]) {
        moo_errorf("Savestate corrupt in line %i: wrong byte, expected %.2X got %.2X", line, checkpoints[current_checkpoint], byte);
        return;
    }
    current_checkpoint++;
}

void save_cpu() {
    S(A); S(F); S(B); S(C); S(D); S(E); S(HL);
    S(SP); S(PC);
    S(cpu.op); S(cpu.cb);
    S(cpu.ime); S(cpu.irq); S(cpu.ie);
    S(cpu.remainder);
    S(cpu.freq);
    S(cpu.freq_factor);
    S(cpu.freq_switch);
    S(cpu.halted);

    set_checkpoint();
}

static void load_cpu() {
    R(A); R(F); R(B); R(C); R(D); R(E); R(HL);
    R(SP); R(PC);
    R(cpu.op); R(cpu.cb);
    R(cpu.ime); R(cpu.irq); R(cpu.ie);
    R(cpu.remainder);
    R(cpu.freq);
    R(cpu.freq_factor);
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

static void save_sqw(sqw_t *c) {
    S(c->on);
    S(c->l); S(c->r);
    S(c->freq); S(c->duty); S(c->volume);
    S(c->cc); S(c->cc_reset);
    S(c->counter.length); S(c->counter.expires);
}

static void save_env(env_t *e) {
    S(e->sweep); S(e->tick); S(e->dir);
}

static void save_sweep() {
    S(sweep.period); S(sweep.dir); S(sweep.shift); S(sweep.tick);
}

static void save_wave() {
    S(wave.on);
    S(wave.cc); S(wave.cc_reset);
    S(wave.l); S(wave.r);
    S(wave.freq);
    S(wave.shift);
    SV(wave.data);
    S(wave.counter.length); S(wave.counter.expires);
}

static void save_noise() {
    S(noise.on);
    S(noise.cc); S(noise.cc_reset);
    S(noise.l); S(noise.r);
    S(noise.volume);
    S(noise.shift);
    S(noise.width);
    S(noise.divr);
    S(noise.lsfr);
    S(noise.counter.length); S(noise.counter.expires);
}

static void save_sound() {
    S(sound.on);
    S(sound.so1_volume); S(sound.so2_volume);
    S(sound.mix_threshold);
    S(sound.cc_reset);
    S(sound.remainder);
    set_checkpoint();

    save_sqw(&sqw[0]); save_sqw(&sqw[1]);
    set_checkpoint();
    save_env(&env[0]); save_env(&env[1]); save_env(&env[2]);
    set_checkpoint();
    save_sweep();
    save_wave();
    save_noise();
    set_checkpoint();
}

static void load_sqw(sqw_t *c) {
    R(c->on);
    R(c->l); R(c->r);
    R(c->freq); R(c->duty); R(c->volume);
    R(c->cc); R(c->cc_reset);
    R(c->counter.length); R(c->counter.expires);
}

static void load_env(env_t *e) {
    R(e->sweep); R(e->tick); R(e->dir);
}

static void load_sweep() {
    R(sweep.period); R(sweep.dir); R(sweep.shift); R(sweep.tick);
}

static void load_wave() {
    R(wave.on);
    R(wave.cc); R(wave.cc_reset);
    R(wave.l); R(wave.r);
    R(wave.freq);
    R(wave.shift);
    RV(wave.data);
    R(wave.counter.length); R(wave.counter.expires);
}

static void load_noise() {
    R(noise.on);
    R(noise.cc); R(noise.cc_reset);
    R(noise.l); R(noise.r);
    R(noise.volume);
    R(noise.shift);
    R(noise.width);
    R(noise.divr);
    R(noise.lsfr);
    R(noise.counter.length); R(noise.counter.expires);
}

static void load_sound() {
    R(sound.on);
    R(sound.so1_volume); R(sound.so2_volume);
    R(sound.mix_threshold);
    R(sound.cc_reset);
    R(sound.remainder);
    assert_checkpoint();

    load_sqw(&sqw[0]); load_sqw(&sqw[1]);
    assert_checkpoint();
    load_env(&env[0]); load_env(&env[1]); load_env(&env[2]);
    assert_checkpoint();
    load_sweep();
    load_wave();
    load_noise();
    assert_checkpoint();
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

static u8 hw_event_to_id(hw_event_t *event) {
    if(event == &lcd_mode_event[0]) return LCD_MODE_0_EVENT_ID;
    if(event == &lcd_mode_event[1]) return LCD_MODE_1_EVENT_ID;
    if(event == &lcd_mode_event[2]) return LCD_MODE_2_EVENT_ID;
    if(event == &lcd_mode_event[3]) return LCD_MODE_3_EVENT_ID;
    if(event == &lcd_vblank_line_event) return LCD_VBLANK_LINE_EVENT_ID;
    if(event == &sound_mix_event) return SOUND_MIX_EVENT_ID;
    if(event == &sound_sweep_event) return SOUND_SWEEP_EVENT_ID;
    if(event == &sound_envelopes_event) return SOUND_ENVELOPES_EVENT_ID;
    if(event == &sound_length_counters_event) return SOUND_LENGTH_COUNTERS_EVENT_ID;
    if(event == &timers_tima_event) return TIMERS_TIMA_EVENT_ID;
    if(event == &timers_div_event) return TIMER_DIV_EVENT_ID;
    if(event == &rtc_event) return RTC_EVENT_ID;
    assert(0);
}

static void save_hw_queue(hw_event_t *q) {
    hw_event_t *event;
    for(event = q; event != NULL; event = event->next) {
        S(hw_event_to_id(event));
        S(event->mcs);
    }
    S(BYTE(0xFF));
}

static void save_hw() {
    S(hw.cc);
    save_hw_queue(hw.queue);
    set_checkpoint();
    save_hw_queue(hw.sched);
    set_checkpoint();
}

static hw_event_t *hw_id_to_event(u8 id) {
    if(id == LCD_MODE_0_EVENT_ID) return &lcd_mode_event[0];
    if(id == LCD_MODE_1_EVENT_ID) return &lcd_mode_event[1];
    if(id == LCD_MODE_2_EVENT_ID) return &lcd_mode_event[2];
    if(id == LCD_MODE_3_EVENT_ID) return &lcd_mode_event[3];
    if(id == LCD_VBLANK_LINE_EVENT_ID) return &lcd_vblank_line_event;
    if(id == SOUND_MIX_EVENT_ID) return &sound_mix_event;
    if(id == SOUND_SWEEP_EVENT_ID) return &sound_sweep_event;
    if(id == SOUND_ENVELOPES_EVENT_ID) return &sound_envelopes_event;
    if(id == SOUND_LENGTH_COUNTERS_EVENT_ID) return &sound_length_counters_event;
    if(id == TIMERS_TIMA_EVENT_ID) return &timers_tima_event;
    if(id == TIMER_DIV_EVENT_ID) return &timers_div_event;
    if(id == RTC_EVENT_ID) return &rtc_event;

    moo_errorf("Savestate is corrupt #1");
    return NULL;
}

static void load_hw_queue() {
    u8 id;
    hw_cycle_t mcs;
    hw_event_t *event;
    int scheduled[NUM_HW_EVENTS] = {0};

    for(R(id); id != 0xFF; R(id)) {
        if(scheduled[id]) {
            moo_errorf("Savestate is corrupt #2");
        }
        scheduled[id] = 1;

        event = hw_id_to_event(id);
        if(event == NULL) {
            return;
        }
#ifdef DEBUG
        event->dbg_queued = 0;
#endif
        R(mcs);
        hw_schedule(event, mcs - hw.cc);
    }
}

static void load_hw() {
    hw_reset();

    R(hw.cc);
    load_hw_queue();
    assert_checkpoint();
    load_hw_queue();
    assert_checkpoint();
}

static void save_sys() {
    S(sys.ticks);
    S(sys.invoke_cc);
    set_checkpoint();
}

static void load_sys() {
    R(sys.ticks);
    R(sys.invoke_cc);
    assert_checkpoint();
}

static void save_util() {
    S(framerate.cc_ahead);
    S(framerate.skipped);
    S(framerate.delay_threshold);
    S(framerate.first_frame_ticks);
    S(framerate.framecount);
    S(framerate.last_curb_ticks);
    set_checkpoint();
}

static void load_util() {
    R(framerate.cc_ahead);
    R(framerate.skipped);
    R(framerate.delay_threshold);
    R(framerate.first_frame_ticks);
    R(framerate.framecount);
    R(framerate.last_curb_ticks);
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
    if(byte != CHECKPOINTS) {
        moo_errorf("Corrupt savestate #2");
    }
    for(cp = 0; cp < CHECKPOINTS; cp++) {
        R(byte);
        checkpoints[cp] = byte;
    }
    current_checkpoint = 0;
    bytes_handled = 0;
}

void state_save(const char *filename) {
    printf("Saving state to '%s'\n", filename);

    f = fopen(filename, "wb");
    if(f == NULL) {
        moo_errorf("Couln't write to .sav file '%s'", filename);
        return;
    }

    init_checkpoints();

    save_cpu();
    save_joy();
    save_lcd();
    save_mbc();
    save_mem();
    save_rtc();
    save_sound();
    save_timers();
    save_hw();

    save_sys();
    save_util();

    fclose(f);
}

int state_load(const char *filename) {
    printf("Loading state from '%s'\n", filename);

    f = fopen(filename, "rb");
    if(f == NULL) {
        return 0;
    }

    load_checkpoints();

    load_cpu();
    load_joy();
    load_lcd();
    load_mbc();
    load_mem();
    load_rtc();
    load_sound();
    load_timers();
    load_hw();

    load_sys();
    load_util();

#ifdef DEBUG
    if(fread(&byte, 1, 1, f) != 0) {
        moo_errorf("Savestate file is too big. Is this really an error?");
    }
#endif

    fclose(f);

    if(~moo.state & MOO_ERROR_BIT) {
        moo_continue();
    }

    return ~moo.state & MOO_ERROR_BIT;
}

