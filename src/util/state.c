#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <string.h>
#include "sys/sys.h"
#include "util/framerate.h"
#include "util/speed.h"
#include "core/cpu.h"
#include "core/joy.h"
#include "core/rtc.h"
#include "core/mbc.h"
#include "core/moo.h"
#include "core/mem.h"
#include "core/maps.h"
#include "core/timers.h"
#include "core/sound.h"
#include "core/lcd.h"
#include "core/defines.h"

#define BYTE(val) ((u8)(val))

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

static hw_event_t *id2event[] = {
    &lcd.mode_event[0], &lcd.mode_event[1], &lcd.mode_event[2], &lcd.mode_event[3],
    &lcd.vblank_line_event, &sound_mix_event, &sound_sweep_event, &sound_envelopes_event,
    &sound_length_counters_event, &timers_tima_event, &timers_div_event, &rtc_event
};

#define STATE_PREFIX "mbs"
static const u8 STATE_REVISION = 0x01;

static FILE *f;
static u8 byte;
static u8 loading_revision;

typedef struct {
    void *ptr;
    int size;
} value_t;

#define V(v) {&(v), sizeof(v)}
#define VA(v) {(v), sizeof(v)}

#define S(v) fwrite(&v, sizeof(v), 1, f)
#define R(v) fread(&v, sizeof(v), 1, f)

#define _sqw(c) \
    V((c).on), \
    V((c).l), V((c).r), \
    V((c).freq), V((c).duty), V((c).volume), \
    V((c).cc), V((c).cc_reset),\
    V((c).counter.length), V((c).counter.expires)

#define _env(e) V((e).sweep), V((e).tick), V((e).dir)

#define NUM_VALUES (sizeof(values)/sizeof(*values))

static value_t values[] = {
    V(cpu.af), V(cpu.bc), V(cpu.de), V(cpu.hl),
    V(SP), V(PC),
    V(cpu.op), V(cpu.cb),
    V(cpu.ime), V(cpu.irq), V(cpu.ie),
    V(cpu.remainder),
    V(cpu.freq),
    V(cpu.freq_factor),
    V(cpu.freq_switch),
    V(cpu.halted),
    V(joy.col),
    VA(lcd.fb),
    V(lcd.c),
    V(lcd.stat),
    V(lcd.scx), V(lcd.scy),
    V(lcd.ly), V(lcd.lyc),
    V(lcd.wx), V(lcd.wy),
    VA(lcd.bgp.b), VA(lcd.obp.b),
    VA(lcd.bgp.d), VA(lcd.obp.d),
    V(lcd.bgp.s), V(lcd.bgp.i),
    V(lcd.obp.s), V(lcd.obp.i),
    V(lcd.hdma_source), V(lcd.hdma_dest),
    V(lcd.hdma_length), V(lcd.hdma_inactive),
    V(mbc.type),
    V(mbc.has_rtc),
    V(mbc.has_battery),
    V(mbc1.mode),
    V(mbc1.rombank),
    V(mbc3.mode),
    V(mbc5.rombank),
    V(card.romsize),
    V(card.sramsize),
    VA(card.srambanks),
    VA(ram.rambanks),
    VA(ram.vrambanks),
    VA(ram.hram),
    VA(ram.oam),
    V(ram.rambank_index),
    VA(rtc.latched),
    VA(rtc.ticking),
    V(rtc.mapped),
    V(rtc.prelatched),
    V(sound.on),
    V(sound.so1_volume), V(sound.so2_volume),
    V(sound.mix_threshold),
    V(sound.cc_reset),
    V(sound.remainder),
    _sqw(sqw[0]), _sqw(sqw[1]),
    _env(env[0]), _env(env[1]), _env(env[2]),
    V(sweep.period), V(sweep.dir), V(sweep.shift), V(sweep.tick),
    V(wave.on),
    V(wave.cc), V(wave.cc_reset),
    V(wave.l), V(wave.r),
    V(wave.freq),
    V(wave.shift),
    VA(wave.data),
    V(wave.counter.length), V(wave.counter.expires),
    V(noise.on),
    V(noise.cc), V(noise.cc_reset),
    V(noise.l), V(noise.r),
    V(noise.volume),
    V(noise.shift),
    V(noise.width),
    V(noise.divr),
    V(noise.lsfr),
    V(noise.counter.length), V(noise.counter.expires),
    V(timers.div), V(timers.tima), V(timers.tma), V(timers.tac),
    V(timers.div_cc), V(timers.tima_cc),
    V(sys.ticks),
    V(sys.invoke_cc),
    V(framerate.skipped),
    V(framerate.delay_threshold),
    V(framerate.first_frame_ticks),
    V(framerate.framecount),
    V(speed.cc_ahead),
    V(speed.last_limit_check),
    V(hw.cc)
};

static void save_prefix() {
    fprintf(f, "%s", STATE_PREFIX);
    S(STATE_REVISION);
}

static void save_values() {
    int v;
    for(v = 0; v < NUM_VALUES; v++) {
        fwrite(values[v].ptr, 1, values[v].size, f);
    }
}

static u8 hw_event_to_id(hw_event_t *event) {
    if(event == &lcd.mode_event[0]) return LCD_MODE_0_EVENT_ID;
    if(event == &lcd.mode_event[1]) return LCD_MODE_1_EVENT_ID;
    if(event == &lcd.mode_event[2]) return LCD_MODE_2_EVENT_ID;
    if(event == &lcd.mode_event[3]) return LCD_MODE_3_EVENT_ID;
    if(event == &lcd.vblank_line_event) return LCD_VBLANK_LINE_EVENT_ID;
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
        byte = hw_event_to_id(event); S(byte);
        S(event->mcs);
    }
    byte = 0xFF; S(byte);
}

static void save_hw() {
    S(hw.cc);
    save_hw_queue(hw.queue);
    save_hw_queue(hw.sched);
}


static void save_misc() {
    byte = (u8(*)[0x4000])mbc.rombank - card.rombanks; S(byte);
    byte = (u8(*)[0x2000])mbc.srambank - card.srambanks; S(byte);
    byte = (u8(*)[0x1000])ram.rambank - ram.rambanks; S(byte);
    save_hw();
}

void state_save(const char *filename) {
    printf("Saving state to '%s'\n", filename);

    f = fopen(filename, "wb");
    if(f == NULL) {
        moo_errorf("Couln't write to .sav file '%s'", filename);
        return;
    }

    save_prefix();
    save_values();
    save_misc();

    fclose(f);
}

static int load_prefix() {
    char buf[sizeof(STATE_PREFIX)];

    int read = fread(buf, 1, sizeof(buf)-1, f);
    if(read != sizeof(buf)-1 || strcmp(buf, STATE_PREFIX)) {
        moo_errorf("File is no savestate or savestate is corrupt");
        return 1;
    }
    if(fread(&loading_revision, sizeof(loading_revision), 1, f) != sizeof(loading_revision)) {
        moo_errorf("Savestate too small");
        return 1;
    }

    return 0;
}

static int load_values() {
    int v;
    for(v = 0; v < NUM_VALUES; v++) {
        size_t read = fread(values[v].ptr, 1, values[v].size, f);
        if(read != values[v].size) {
            moo_errorf("Savestate corrupt at value %i", v);
            return 1;
        }
    }
    return 0;
}

static hw_event_t *hw_id_to_event(u8 id) {
    if(id < NUM_HW_EVENTS) {
        return id2event[id];
    }
    else {
        moo_errorf("Savestate is corrupt: No such event id %i", id);
        return NULL;
    }
}

static int load_hw_queue() {
    u8 id;
    hw_cycle_t mcs;
    hw_event_t *event;
    int scheduled[NUM_HW_EVENTS] = {0};

    for(R(id); id != 0xFF; R(id)) {
        if(scheduled[id]) {
            moo_errorf("Savestate is corrupt #2");
            return 1;
        }
        scheduled[id] = 1;

        event = hw_id_to_event(id);
        if(event == NULL) {
            return 1;
        }
#ifdef DEBUG
        event->dbg_queued = 0;
#endif
        R(mcs);
        hw_schedule(event, mcs - hw.cc);
    }

    return 0;
}

static int load_hw() {
    int error = 0;

    hw_reset();
    R(hw.cc);
    error |= load_hw_queue();
    error |= load_hw_queue();

    return error;
}

static int load_misc() {
    int error = 0;

    joy.state = 0xFF;

    error |= fread(&byte, 1, 1, f) != 1; mbc.rombank = card.rombanks[byte];
    error |= fread(&byte, 1, 1, f) != 1; mbc.srambank = card.srambanks[byte & 0x03];
    error |= fread(&byte, 1, 1, f) != 1; ram.rambank = ram.rambanks[byte & 0x07];
    error |= load_hw();

    maps_dirty();
    lcd_rebuild_palette_maps();

    return error;
}

int state_load(const char *filename) {
    printf("Loading state from '%s'\n", filename);

    f = fopen(filename, "rb");
    if(f == NULL) {
        return 0;
    }

    if( load_prefix() || load_values() || load_misc())  {
        fclose(f);
        return 0;
    }

    if(fread(&byte, 1, 1, f) != 0) {
        moo_errorf("Savestate file is too big. Is this really an error?");
    }
    fclose(f);

    if(~moo.state & MOO_ERROR_BIT) {
        moo_continue();
    }

    return ~moo.state & MOO_ERROR_BIT;
}

