#include "sys/sys.h"
#include "video.h"
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
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

#define FB_WIDTH 160
#define FB_HEIGHT 144
#define DELAY_THRESHOLD 10

#define PIXELSIZE 4

static unsigned int invoke_count;
static unsigned int last_sec_cc;
static unsigned int last_delay_cc;
static time_t delay_start;
static time_t last_sec;
static char rompath[256] = "rom/gold.gbc";
static int running;


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
        if(event.type == SDL_KEYDOWN) {
            switch(event.key.keysym.sym) {
                case SDLK_m:    load_state(); break;
                case SDLK_n:    save_state(); break;
                default: break;
            }
        }
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
            running = 0;
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
    long ms_ahead = cc_ahead / ((long)cpu.freq/1000);

    if(ms_ahead >= DELAY_THRESHOLD) {
        SDL_Delay(DELAY_THRESHOLD);
    }
}

void sys_init(int argc, const char** argv) {
    invoke_count = 0;
    last_sec_cc = 0;
    last_delay_cc = 0;
    delay_start = 0;
    cmd_init(argc, argv);

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    SDL_Surface *screen = SDL_SetVideoMode(160*PIXELSIZE, 144*PIXELSIZE, 24, SDL_DOUBLEBUF);

    last_sec = SDL_GetTicks();

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
    return rompath;
}


int sys_invoke() {
    //printf("%i\n", cpu.freq);

    running = 1;
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
    SDL_Rect area;

    area.x = 0;
    area.y = 0;
    area.w = SDL_GetVideoSurface()->w;
    area.h = SDL_GetVideoSurface()->h;

    sdl_render(SDL_GetVideoSurface(), area, 0);
    SDL_Flip(SDL_GetVideoSurface());
}

