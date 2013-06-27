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
#include "audio.h"
#include "framerate.h"
#include "performance.h"


sys_t sys;

static int running;

void sys_init(int argc, const char** argv) {
    sys.fb_ready = 0;
    sys.quantum_length = 1000;
    sys.bits_per_pixel = 16;
    sprintf(sys.rompath, "rom/crystal.gbc");

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
#ifdef PANDORA
    SDL_SetVideoMode(534, 480, sys.bits_per_pixel, SDL_FULLSCREEN);
    SDL_ShowCursor(0);
#else
    SDL_SetVideoMode(534, 480, sys.bits_per_pixel, 0);
#endif

    cmd_init(argc, argv);
    audio_init();
    video_init();
    framerate_init();
    performance_init();
    input_init();
}

void sys_close() {
    SDL_Quit();
}

bool sys_running()  {
    return TRUE;
}

bool sys_new_rom()  {
    return TRUE;
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
            if(event.key.keysym.sym == SDLK_ESCAPE) {
                running = 0;
            }
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
    performance_invoked();

    return running;
}

void sys_begin() {
    framerate_begin();
    performance_begin();
}

void sys_fb_ready() {
    sys.fb_ready = 1;
}

