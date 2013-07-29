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
#include "sys/menu/menu.h"


sys_t sys;

void sys_init(int argc, const char** argv) {
    sys.fb_ready = 0;
    sys.sound_on = 1;
    sys.in_menu = 1;
    sys.running = 1;
    sys.rom_loaded = 0;
    sys.pause_start = 0;
    sys.quantum_length = 1000;
    sys.bits_per_pixel = 16;
    sprintf(sys.rompath, "rom/gold.gbc");

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
#ifdef PANDORA
    SDL_SetVideoMode(534, 480, sys.bits_per_pixel, SDL_FULLSCREEN);
    SDL_ShowCursor(0);
#else
    SDL_SetVideoMode(480, 432, sys.bits_per_pixel, 0);
#endif

    sys.ticks_diff = SDL_GetTicks();

    cmd_init(argc, argv);
    audio_init();
    video_init();
    framerate_init();
    performance_init();
    input_init();

    menu_init();
}

void sys_close() {
    menu_close();

    SDL_Quit();
}

void sys_pause() {
    sys.pause_start = SDL_GetTicks();
}

void sys_run() {
    sys.ticks_diff += SDL_GetTicks() - sys.pause_start;
}

void sys_get_rompath_base(char *buf) {
    int c;
    strcpy(buf, sys.rompath);
    for(c = (int)strlen(buf)-2; c >= 0 && buf[c] != '.'; c--) {

    }

    if(c > 0) {
        buf[c] = '\0';
    }
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

void sys_handle_events(void (*input_handle)(int, int)) {
   SDL_Event event;

    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            input_handle(event.type, event.key.keysym.sym);
        }
        if(event.type == SDL_QUIT) {
            sys.running = 0;
        }
    }
}


void sys_invoke() {
    sys.ticks = SDL_GetTicks() - sys.ticks_diff;

    if(sys.fb_ready) {
        if(!framerate_skip()) {
            render();
        }

        sys.fb_ready = 0;
        performance.frames++;
    }
    framerate_curb();

    sys_handle_events(input_event);
    performance_invoked();
}

void sys_begin() {
    framerate_begin();
    performance_begin();
}

void sys_fb_ready() {
    sys.fb_ready = 1;
}

