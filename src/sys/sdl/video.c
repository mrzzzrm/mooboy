#include <SDL/SDL.h>
#include <assert.h>
#include "util/defines.h"
#include "core/emu.h"
#include "core/lcd.h"

static int line_length;
static int bytes_per_pixel;
static int bytes_per_line;

static u32 dmg_palette[] = {
    0xFFFFFFFF,
    0xAAAAAAFF,
    0x555555FF,
    0x000000FF
};

static void render_fbline(void **_buf, int line) {
    int x, b;
    u8 *buf;

    *_buf = realloc(*_buf, bytes_per_line);

    switch(bytes_per_pixel) {
        case 1: break;
        case 2: break;
        case 3:
           buf = (u8*)_buf;
//            for(x = 0, b = 0; x < line_length; x++) {
//                u16 col = lcd.clean_fb[y*160 + x];
//                u32 r = ((col >> 0) & 0x001F)  << 3 << 24;
//                u32 g = ((col >> 5) & 0x001F)  << 3 << 16;
//                u32 b = ((col >> 10) & 0x001F) << 3 << 8;

//                buf[b++] =
//                buf[b++] =
//                buf[b++] =
//            }
        break;
        case 4: break;

        default:
            assert(0);
    }
}

static void fullwidth_render(SDL_Surface *surface, SDL_Rect area, u32 flags) {
    int vline, buflines, fbline;
    void *buf;

    buf = NULL;
    buflines = 0;
    fbline = 0;
    line_length = surface->w;
    bytes_per_line = surface->pitch;
    bytes_per_pixel = surface->format->BytesPerPixel;

    for(vline = 0; vline < area.h;) {
        render_fbline(&buf, fbline);
    }
}

static void area_render(SDL_Surface *surface, SDL_Rect area, u32 flags) {

}

void sdl_render(SDL_Surface *surface, SDL_Rect area, u32 flags) {
    if(area.x == 0 && area.w == surface->w) {
        fullwidth_render(surface, area, flags);
    }
    else {
        area_render(surface, area, flags);
    }
}
//
//    unsigned int x, y;
//    static time_t last = 0;
//    SDL_Surface *s = SDL_GetVideoSurface();
//
//    if(last == 0) {
//        last = SDL_GetTicks();
//    }
//
//    if(SDL_GetTicks() - last < 16) {
//        return;
//    }
//    else {
//        last += 16;
//    }
//
//    for(y = 0; y < 144; y++) {
//        for(x = 0; x < 160; x++) {
//            if(emu.hw == DMG_HW) {
//                pixelColor(s, x, y, palette[lcd.clean_fb[y*160 + x] % 4]);
////                boxColor(s, x*1, y*1, x*1, y*1, palette[lcd.clean_fb[y*FB_WIDTH + x] % 4]);
//            }
//            else {
//                u16 col = lcd.clean_fb[y*160 + x];
//                u32 r = ((col >> 0) & 0x001F)  << 3 << 24;
//                u32 g = ((col >> 5) & 0x001F)  << 3 << 16;
//                u32 b = ((col >> 10) & 0x001F) << 3 << 8;
////
////                if((r | g | b) != 0)
////                printf("%.8X ", r | g | b | 0xFF);
//                boxColor(s, x*4, y*4, x*4+4-1, y*4+4-1, r | g | b | 0xFF);
//            }
//        }
//    }
