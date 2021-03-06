#include "sys/sys.h"
#include "video.h"
#include <stdarg.h>
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_image.h>
#include <assert.h>
#include "core/cpu.h"
#include "core/rtc.h"
#include "core/mbc.h"
#include "core/lcd.h"
#include "core/moo.h"
#include "core/joy.h"
#include "core/defines.h"
#include "core/sound.h"
#include "util/state.h"
#include "input.h"
#include "audio.h"
#include "util/speed.h"
#include "util/framerate.h"
#include "util/performance.h"
#include "util/speed.h"

#define SCALING_PROPORTIONAL 0
#define SCALING_STRECHED 1
#define SCALING_PROPORTIONAL_FULL 2
#define SCALING_NONE 3

sys_t sys;

static SDL_Surface *statuslabel;

static char *scalingmode_names[] = {"Proportional", "Streched", "Full Proportional", "None"};

void sys_init(int argc, const char** argv) {
    memset(&sys, 0x00, sizeof(sys));

    sys.sound_on = 0;
    sys.sound_freq = 22050;
    sys.sound_sample_size = 2;
    sys.sound_buf_size = 4096;
    sys.sound_buf = malloc(sys.sound_buf_size * sys.sound_sample_size * 2);
    sys.quantum_length = 1000;
    sys.bits_per_pixel = 16;
    sys.bytes_per_pixel = 2;
    sys.show_statusbar = 0;
    sys.auto_continue = SYS_AUTO_CONTINUE_ASK;
    sys.fb_ready = 0;
    moo.state = MOO_RUNNING_BIT;

    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        moo_fatalf("Couln't initialize SDL");
    }

#ifdef DEBUG
    int screen_w = 800;
    int screen_h = 480;
    int flags = 0;
#else
    int screen_w = 0;
    int screen_h = 0;
    int flags = SDL_FULLSCREEN;
#endif

//    flags |= SDL_DOUBLEBUF;

    SDL_ShowCursor(0);

    if(SDL_SetVideoMode(screen_w, screen_h, sys.bits_per_pixel, flags) == NULL) {
        moo_fatalf("Setting of SDL video-mode failed");
    }

    if(!IMG_Init(IMG_INIT_PNG)) {
        moo_fatalf("Initialisation of SDL_image failed");
    }

    sys.scalingmode = 0;
    sys.num_scalingmodes = 4;
    sys.scalingmode_names = scalingmode_names;

    audio_init();
    framerate_init();
    input_init();
    video_init();

    statuslabel = SDL_CreateRGBSurface(0, SDL_GetVideoSurface()->w, 8, sys.bits_per_pixel, 0, 0, 0, 0);
    assert(statuslabel != NULL);
}

void sys_reset() {
    sys.sound_buf_start = 0;
    sys.sound_buf_end = 0;
    sys.ticks = 0;
    sys.invoke_cc = 0;
}

void sys_close() {
    free(sys.sound_buf);

    video_close();
    IMG_Quit();
    SDL_PauseAudio(1);
    SDL_Quit();
}

void sys_pause() {
    sys_play_audio(0);
    video_switch_display_mode();
}

void sys_begin() {
    sys.ticks_diff = sys.ticks - (long long)SDL_GetTicks();
    sys_play_audio(sys.sound_on);
    video_switch_display_mode();
}

void sys_continue() {
    sys.ticks_diff = (long long)sys.ticks - (long long)SDL_GetTicks();
    sys_play_audio(sys.sound_on);
    video_switch_display_mode();
}

SDL_Rect none_scaling_area() {
    SDL_Rect area;
    area.x = SDL_GetVideoSurface()->w/2 - 80;
    area.y = SDL_GetVideoSurface()->h/2 - 72;
    area.w = 160;
    area.h = 144;
    return area;
}

SDL_Rect proportional_scaling_area() {
    SDL_Rect area;
    int fw, fh, f;
    fw = SDL_GetVideoSurface()->w / 160;
    fh = SDL_GetVideoSurface()->h / 144;
    f = min(fw, fh);
    area.w = f * 160;
    area.h = f * 144;
    area.x = SDL_GetVideoSurface()->w/2 - area.w/2;
    area.y = SDL_GetVideoSurface()->h/2 - area.h/2;
    return area;
}

SDL_Rect proportional_full_scaling_area() {
    SDL_Rect area;
    float fw, fh, f;
    fw = SDL_GetVideoSurface()->w / 160.0f;
    fh = SDL_GetVideoSurface()->h / 144.0f;
    f = min(fw, fh);
    area.w = f * 160;
    area.h = f * 144;
    area.x = SDL_GetVideoSurface()->w/2 - area.w/2;
    area.y = SDL_GetVideoSurface()->h/2 - area.h/2;
    return area;
}

SDL_Rect streched_scaling_area() {
    SDL_Rect area;
    area.x = 0;
    area.y = 0;
    area.w = SDL_GetVideoSurface()->w;
    area.h = SDL_GetVideoSurface()->h;
    return area;
}


static void render() {
    video_render(SDL_GetVideoSurface());
    if(sys.show_statusbar) {
        SDL_BlitSurface(statuslabel, NULL, SDL_GetVideoSurface(), NULL);
    }
    SDL_Flip(SDL_GetVideoSurface());
}

void sys_delay(int ticks) {
    SDL_Delay(ticks);
}

void sys_fb_ready() {
    sys.fb_ready = 1;
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

    if(framerate_next_frame()) {
        render();
        sys.fb_ready = 0;
        performance.counting.frames++;
    }

    speed_limit();

    sys_handle_events(input_event);
    performance_invoked();
    //sys_serial_step();
}

void sys_play_audio(int on) {
    if(speed.factor == 1) {
        SDL_PauseAudio(!on);
    }
    else {
        SDL_PauseAudio(1);
    }
}

void sys_new_performance_info() {
    char statusline[256];
    snprintf(statusline, sizeof(statusline), "Skipped %i/%i frames, Slept %6.2f %%, Speed: %6.2f %%, CPU: %i Hz", performance.counters.skipped, performance.counters.frames, (float)performance.counters.slept*100/PERFORMANCE_UPDATE_PERIOD, performance.speed, cpu.freq);

    SDL_FillRect(statuslabel, NULL, 0);
    stringColor(statuslabel, 0, 0, statusline, 0xaaaaaaff);
}

void sys_set_scalingmode(int mode) {
    SDL_Rect area;

    switch(mode) {
        case SCALING_NONE: area = none_scaling_area(); break;
        case SCALING_PROPORTIONAL: area = proportional_scaling_area(); break;
        case SCALING_PROPORTIONAL_FULL: area = proportional_full_scaling_area(); break;
        case SCALING_STRECHED: area = streched_scaling_area(); break;
        default: moo_errorf("No valid scalingmode selected"); return;
    }
    sys.scalingmode = mode;

    video_set_area(area);
}
