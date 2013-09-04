#include "sys/sys.h"
#include "video.h"
#include <stdarg.h>
#include <SDL/SDL.h>
#include <assert.h>
#include "core/cpu.h"
#include "core/rtc.h"
#include "core/mbc.h"
#include "core/lcd.h"
#include "core/moo.h"
#include "core/joy.h"
#include "core/sound.h"
#include "util/state.h"
#include "input.h"
#include "audio.h"
#include "util/framerate.h"
#include "util/performance.h"


sys_t sys;

void sys_init(int argc, const char** argv) {
    memset(&sys, 0x00, sizeof(sys));

    sys.sound_on = 1;
    sys.sound_freq = 22050;
    sys.quantum_length = 1000;
    sys.bits_per_pixel = 16;
    moo.state = MOO_RUNNING_BIT;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
#ifdef PANDORA
    SDL_SetVideoMode(800, 480, sys.bits_per_pixel, SDL_FULLSCREEN);
    SDL_ShowCursor(0);
#else
    SDL_SetVideoMode(800, 480, sys.bits_per_pixel, 0);
#endif


    audio_init();
    video_init();
    framerate_init();
    performance_init();
    input_init();
}

void sys_reset() {
    sys.ticks = 0;
    sys.invoke_cc = 0;
    sys.ticks_diff = -(long long)SDL_GetTicks();
    framerate_begin();
    performance_begin();
}

void sys_close() {

    SDL_Quit();
}

void sys_pause() {
    sys.pause_start = SDL_GetTicks();
}

void sys_run() {
    sys.ticks_diff -= SDL_GetTicks() - sys.pause_start;
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

void sys_delay(int ticks) {
    SDL_Delay(ticks);
}

void sys_handle_events(void (*input_handle)(int, int)) {
   SDL_Event event;

    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            input_handle(event.type, event.key.keysym.sym);
        }
        if(event.type == SDL_QUIT) {
            moo_quit();
        }
    }
}

void sys_invoke() {
    sys.ticks = SDL_GetTicks() + sys.ticks_diff;
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
    //sys_serial_step();
}

void sys_fb_ready() {
    sys.fb_ready = 1;
}

void sys_play_audio(int on) {
    SDL_PauseAudio(!on);
}

