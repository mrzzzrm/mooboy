#include "sys/sys.h"
#include "video.h"
#include <SDL/SDL.h>
#include <assert.h>
#include "util/cmd.h"
#include "core/cpu.h"
#include "core/rtc.h"
#include "core/mbc.h"
#include "core/lcd.h"
#include "core/emu.h"
#include "core/joy.h"
#include "util/err.h"
#include "core/sound.h"
#include "state.h"
#include "input.h"


sys_t sys;

static size_t invoke_count;
static size_t last_sec_cc;
static size_t last_delay_cc;
static time_t delay_start;
static time_t last_sec;
static char rompath[256] = "rom/gold.gbc";
static int running;
static int fb_ready;

void sys_init(int argc, const char** argv) {
    last_sec_cc = 0;
    fb_ready = 0;

    last_sec = SDL_GetTicks();
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_Surface *screen = SDL_SetVideoMode(320, 288, 16, 0);

    cmd_init(argc, argv);
    audio_init();
    video_init();
    framerate_init();
    input_init();
}

void sys_close() {

}

bool sys_running()  {
    return TRUE;
}

bool sys_new_rom()  {
    return TRUE;
}

const char *sys_get_rompath() {
    return rompath;
}

static void render() {
    SDL_Rect area;

    area.x = 0;
    area.y = 0;
    area.w = SDL_GetVideoSurface()->w;
    area.h = SDL_GetVideoSurface()->h;

    video_render(SDL_GetVideoSurface(), area);
    SDL_Flip(SDL_GetVideoSurface());
}

static void handle_events() {
   SDL_Event event;

    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            input_event(SDL_KEYDOWN, event.key.keysym.sym);
        }

        else if(event.type == SDL_QUIT) {
            running = 0;
        }
    }
}

static void measure_performance() {
    invoke_count++;
    time_t dur = SDL_GetTicks() - last_sec;
    if(dur > 1000) {
        last_sec = SDL_GetTicks();
        printf("Invokes: %i %.2f%%\n", invoke_count, ((double)(cpu.cc - last_sec_cc)*100.0f*dur)/(cpu.freq*1000));
        invoke_count = 0;
        last_sec_cc = cpu.cc;
    }
}

int sys_invoke() {
    running = 1;
    sys.ticks = SDL_GetTicks();

    if(fb_ready && framerate_insync()) {
        if(!framerate_skip()) {
            render();
        }

        fb_ready = 0;
    }
    framerate_curb();

    handle_events();
    measure_performance();

    return running;
}

void sys_save_card() {
    char sramfile[256];
    FILE *file;
    size_t written;

    if(!mbc.has_battery || !(mbc.has_ram || mbc.has_rtc)) {
        return;
    }

    sprintf(sramfile, "%s.card", sys_get_rompath());
    file = fopen(sramfile, "w");
    assert(file);

    if(mbc.has_ram) {
        printf("Saving SRAM\n");
        written = fwrite(card.srambanks, 1, card.sramsize * sizeof(*card.srambanks), file);
        assert(written == card.sramsize * sizeof(*card.srambanks));
    }

    if(mbc.has_rtc) {
        printf("Saving RTC\n");
        written = fwrite(rtc.latched,     1, sizeof(rtc.latched), file);    assert (written == sizeof(rtc.latched));
        written = fwrite(rtc.ticking,     1, sizeof(rtc.ticking), file);    assert (written == sizeof(rtc.ticking));
        written = fwrite(&rtc.mapped,     1, sizeof(rtc.mapped), file);     assert (written == sizeof(rtc.mapped));
        written = fwrite(&rtc.prelatched, 1, sizeof(rtc.prelatched), file); assert (written == sizeof(rtc.prelatched));
        written = fwrite(&rtc.cc,         1, sizeof(rtc.cc), file);         assert (written == sizeof(rtc.cc));
    }

    fclose(file);
}

void sys_load_card() {
    char sramfile[256];
    FILE *file;
    size_t read;
    u8 dummy;

    if(!mbc.has_battery || !(mbc.has_ram || mbc.has_rtc)) {
        return;
    }

    sprintf(sramfile, "%s.card", sys_get_rompath());
    file = fopen(sramfile, "r");
    if(file == NULL) {
        printf("No SRAM-file found\n");
        return;
    }

    if(mbc.has_ram) {
        printf("Loading SRAM\n");
        read = fread(card.srambanks, 1, card.sramsize * sizeof(*card.srambanks), file);
        assert(read == card.sramsize * sizeof(*card.srambanks));
    }

    if(mbc.has_rtc) {
        printf("Loading RTC\n");
        read = fread(rtc.latched,     1, sizeof(rtc.latched), file);    assert (read == sizeof(rtc.latched));
        read = fread(rtc.ticking,     1, sizeof(rtc.ticking), file);    assert (read == sizeof(rtc.ticking));
        read = fread(&rtc.mapped,     1, sizeof(rtc.mapped), file);     assert (read == sizeof(rtc.mapped));
        read = fread(&rtc.prelatched, 1, sizeof(rtc.prelatched), file); assert (read == sizeof(rtc.prelatched));
        read = fread(&rtc.cc,         1, sizeof(rtc.cc), file);         assert (read == sizeof(rtc.cc));
    }

    fread(&dummy, 1, 1, file);
    assert(feof(file));


    fclose(file);
}

void sys_fb_ready() {
    fb_ready = 1;
}

