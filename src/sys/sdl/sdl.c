#include "sys/sys.h"
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include "util/cmd.h"
#include "core/cpu.h"
#include "core/lcd.h"
#include "core/joy.h"
#include "util/err.h"
#include "core/sound.h"

#define FB_WIDTH 160
#define FB_HEIGHT 144
#define DELAY_THRESHOLD 10

static unsigned int invoke_count;
static unsigned int last_sec_cc;
static unsigned int last_delay_cc;
static time_t delay_start;
static time_t last_sec;

u32 palette[] = {
    0xFFFFFFFF,
    0xAAAAAAFF,
    0x555555FF,
    0x000000FF,
    0xFF0000FF,
    0x00FF00FF,
    0x0000FFFF,
    0xFFFF00FF
};

static void set_pixel(SDL_Surface *surface, unsigned int x, unsigned int y, u32 color) {
     pixelColor(surface, x, y, color);
}

static void update_joypad() {
   SDL_Event event;

   while(SDL_PollEvent(&event)) {
        if(event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
            u8 state = event.key.state == SDL_RELEASED ? JOY_STATE_RELEASED : JOY_STATE_PRESSED;

            switch(event.key.keysym.sym) {
                case SDLK_UP:   joy_set_button(JOY_BUTTON_UP, state); break;
                case SDLK_DOWN: joy_set_button(JOY_BUTTON_DOWN, state); break;
                case SDLK_LEFT: joy_set_button(JOY_BUTTON_LEFT, state); break;
                case SDLK_RIGHT:joy_set_button(JOY_BUTTON_RIGHT, state); break;
                case SDLK_a:    joy_set_button(JOY_BUTTON_A, state); break;
                case SDLK_s:    joy_set_button(JOY_BUTTON_B, state); break;
                case SDLK_w:    joy_set_button(JOY_BUTTON_START, state); break;
                case SDLK_d:    joy_set_button(JOY_BUTTON_SELECT, state); break;
                default: break;
            }
        }
        else if(event.type == SDL_QUIT) {
            exit(EXIT_SUCCESS);
        }
   }
}

static u16 get_available_samples() {
    u16 r;
    sound_lock();
    if(sound.buf_start > sound.buf_end) {
        r = sound.buf_size - (sound.buf_start - sound.buf_end) + 1;
    }
    else {
        r = sound.buf_end - sound.buf_start;
    }
    sound_unlock();

    return r;
}

void move_buf(void *nichtVerwendet, Uint8 *stream, int length) {
    sound_lock();
    u16 requested_samples = length / (sound.sample_size * 2);
    u16 available_samples;
    u16 served_samples;

    while((available_samples = get_available_samples()) < requested_samples) {
        sound_mix();
    }

    if(sound.buf_start > sound.buf_end) {
        served_samples = sound.buf_size - sound.buf_start;
        if(served_samples >= requested_samples) {
            memcpy(stream, &sound.buf[sound.buf_start*2*2], length);
        }
        else {
            u16 bytes = served_samples * sound.sample_size * 2;
            memcpy(&stream[0], &sound.buf[sound.buf_start*2*2], bytes);
            memcpy(&stream[bytes], &sound.buf[0], (requested_samples - served_samples) * sound.sample_size * 2);
        }
    }
    else {
        memcpy(stream, &sound.buf[sound.buf_start*2*2], length);
    }


    sound.buf_start += requested_samples;
    sound.buf_start %= sound.buf_size;
    sound_unlock();

//    moved += requested_samples;
}

static void handle_delay() {
    if(delay_start == 0) {
        delay_start = SDL_GetTicks();
    }

    long should_cc = ((long)SDL_GetTicks() - (long)delay_start)*(long)(cpu.freq/1000);
    long is_cc = cpu.cc;
    long cc_ahead = is_cc - should_cc;
    long ms_ahead = (is_cc - should_cc) / ((long)cpu.freq/1000);

    if(ms_ahead >= DELAY_THRESHOLD) {
        SDL_Delay(DELAY_THRESHOLD);
    }

}

void sys_init(int argc, const char** argv) {
    invoke_count = 0;
    last_sec_cc = 0;
    last_delay_cc = 0;
    delay_start = 0;
    last_sec = SDL_GetTicks();
    cmd_init(argc, argv);

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_Surface *screen = SDL_SetVideoMode(800, 720, 24, SDL_DOUBLEBUF);

    /* Format: 16 Bit, stereo, 22 KHz */
    SDL_AudioSpec format;
    format.freq = sound.freq;
    format.format = AUDIO_S16;
    format.channels = 2;
    format.samples = 512;
    format.callback = move_buf;
    format.userdata = NULL;

    if (SDL_OpenAudio(&format, NULL) < 0 ) {
        fprintf(stderr, "Audio-Gerät konnte nicht geöffnet werden: %s\n", SDL_GetError());
        exit(1);
    }
    SDL_PauseAudio(0);
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
    const char *rom;
    if((rom = cmd_get("--rom")) == NULL) {
        err_set(ERR_ROM_NOT_FOUND);
        return NULL;
    }
    return rom;
}

void sys_sleep(time_t ticks) {
    SDL_Delay(ticks);
}

void sys_error() {
    exit(EXIT_FAILURE);
}

void sys_invoke() {
    invoke_count++;
    time_t dur = SDL_GetTicks() - last_sec;
    if(dur > 1000) {
        last_sec = SDL_GetTicks();
        fprintf(stderr, "Invokes: %i %.2f%%\n", invoke_count, ((double)(cpu.cc - last_sec_cc)*100.0f*dur)/(cpu.freq*1000));
        invoke_count = 0;
        last_sec_cc = cpu.cc;
    }

    update_joypad();
    handle_delay();
}

void sys_fb_ready() {
    unsigned int x, y;
    SDL_Surface *s = SDL_GetVideoSurface();

    for(y = 0; y < FB_HEIGHT; y++) {
        for(x = 0; x < FB_WIDTH; x++) {
            u8 gbc = lcd.clean_fb[y*FB_WIDTH + x];

//            if(gbc == 0)
//                boxColor(s, x*5, y*5, x*5+4, y*5+4, ((x/8)%2==0 && (y/8)%2==0) || ((x/8)%2==1 && (y/8)%2==1) ? 0x220000ff : 0x000000ff);
//            else
                boxColor(s, x*5, y*5, x*5+4, y*5+4, palette[lcd.clean_fb[y*FB_WIDTH + x] % 4]);
        }
    }

    SDL_Flip(s);
}

