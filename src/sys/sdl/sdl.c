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
#include "performance.h"


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
    sys.fb_ready = 0;
    sys.bits_per_pixel = 16;

    last_sec = SDL_GetTicks();
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_Surface *screen = SDL_SetVideoMode(320, 288, sys.bits_per_pixel, 0);

    cmd_init(argc, argv);
    audio_init();
    video_init();
    framerate_init();
    performance_init();
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

    SDL_BlitSurface(performance.statuslabel, NULL, SDL_GetVideoSurface(), NULL);

    SDL_Flip(SDL_GetVideoSurface());
}

static void handle_events() {
   SDL_Event event;

    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            input_event(event.type, event.key.keysym.sym);
        }
        else if(event.type == SDL_QUIT) {
            running = 0;
        }
    }
}


int sys_invoke() {
    running = 1;
    sys.ticks = SDL_GetTicks();

    if(sys.fb_ready) {
        if(!framerate_skip()) {
            render();
        }

        sys.fb_ready = 0;
    }
    framerate_curb();


    handle_events();
    performance_update();

    return running;
}

void sys_begin() {
    framerate_begin();
    performance_begin();
}

void sys_fb_ready() {
    sys.fb_ready = 1;
}

