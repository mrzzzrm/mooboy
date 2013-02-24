#include "sys/sys.h"
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include "util/cmd.h"
#include "core/fb.h"
#include "core/cpu.h"
#include "core/mem/io/lcd.h"
#include "core/mem/io/joy.h"
#include "util/err.h"

#define FB_WIDTH 160
#define FB_HEIGHT 144

fb_t fb;

static unsigned int invoke_count;
static unsigned int last_cc;
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
            }
        }
        else if(event.type == SDL_QUIT) {
            exit(EXIT_SUCCESS);
        }
   }
}


void sys_init(int argc, const char** argv) {
    invoke_count = 0;
    last_cc = 0;
    last_sec = SDL_GetTicks();
    cmd_init(argc, argv);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface *screen = SDL_SetVideoMode(800, 720, 24, SDL_DOUBLEBUF);
}

void sys_close() {

}

bool sys_running()  {
    return true;
}

bool sys_new_rom()  {
    return true;
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
    if(SDL_GetTicks() - last_sec > 1000) {
        last_sec = SDL_GetTicks();
        fprintf(stderr, "Invokes: %i %f%%\n", invoke_count, (double)(cpu.cc - last_cc)/10000.0);
        invoke_count = 0;
        last_cc = cpu.cc;
    }

    update_joypad();
}

void sys_fb_ready() {
    unsigned int x, y;
    SDL_Surface *s = SDL_GetVideoSurface();

    for(y = 0; y < FB_HEIGHT; y++) {
        for(x = 0; x < FB_WIDTH; x++) {
            u8 gbc = lcd.clean_fb[y*FB_WIDTH + x];

//            if(gbc == 3)
//                boxColor(s, x*5, y*5, x*5+4, y*5+4, ((x/8)%2==0 && (y/8)%2==0) || ((x/8)%2==1 && (y/8)%2==1) ? 0x220000ff : 0x000000ff);
//            else
                boxColor(s, x*5, y*5, x*5+4, y*5+4, palette[lcd.clean_fb[y*FB_WIDTH + x] % 4]);
        }
    }

    SDL_Flip(s);
}

