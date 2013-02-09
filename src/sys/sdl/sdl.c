#include "sys/sys.h"
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include "util/cmd.h"
#include "core/fb.h"
#include "core/cpu.h"
#include "core/mem/io/lcd.h"
#include "util/err.h"

#define FB_WIDTH 160
#define FB_HEIGHT 144

fb_t fb;

static unsigned int invoke_count;
static unsigned int last_cc;
static time_t last_sec;

u32 palette[] = {
    0x000000FF,
    0x446644FF,
    0x77AA77FF,
    0xCCFFCCFF
};

static void set_pixel(SDL_Surface *surface, unsigned int x, unsigned int y, u32 color) {
     pixelColor(surface, x, y, color);
}


void sys_init(int argc, const char** argv) {
    invoke_count = 0;
    last_cc = 0;
    last_sec = SDL_GetTicks();
    cmd_init(argc, argv);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface *screen = SDL_SetVideoMode(160, 144, 24, SDL_DOUBLEBUF);
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
}

void sys_fb_ready() {
    unsigned int x, y;
    SDL_Surface *s = SDL_GetVideoSurface();

    for(y = 0; y < FB_HEIGHT; y++) {
        for(x = 0; x < FB_WIDTH; x++) {
            set_pixel(s, x, y, palette[lcd.clean_fb[y*FB_WIDTH + x]]);
        }
    }

    SDL_Flip(s);
}

