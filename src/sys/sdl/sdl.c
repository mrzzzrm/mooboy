#include "sys/sys.h"
#include <SDL/SDL.h>
#include "util/cmd.h"
#include "core/fb.h"
#include "util/err.h"

fb_t fb;

void sys_init(int argc, const char** argv) {
    cmd_init(argc, argv);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Surface *screen = SDL_SetVideoMode(160, 144, 24, SDL_DOUBLEBUF);

//    fb.data = screen->pixels;
//    fb.pitch = screen->pitch;
//    fb.w = 160;
//    fb.h = 144;
//    fb.poffset = 3;
//    fb.psize = 3;
//
//    unsigned int s;
//    for(s = 0; s < 4; s++)
//        fb.gbpalette[s] = malloc(sizeof(**fb.gbpalette) * fb.psize);
//    memset(fb.gbpalette[0], 0x44, 3);
//    memset(fb.gbpalette[1], 0x88, 3);
//    memset(fb.gbpalette[2], 0xBB, 3);
//    memset(fb.gbpalette[3], 0xFF, 3);
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
