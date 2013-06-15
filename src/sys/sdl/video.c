#include <SDL/SDL.h>
#include <assert.h>
#include "util/defines.h"
#include "core/emu.h"
#include "core/lcd.h"

static u8 buf[32768];
static int line_length;
static int bytes_per_pixel;
static int bytes_per_line;
static SDL_Rect area;

static struct {
    u8 **colorpool;
} rgb8, rgb16, rgb24, rgba32;


static u32 dmg_palette[] = {
    0xFFFFFFFF,
    0xAAAAAAFF,
    0x555555FF,
    0x000000FF
};

// TODO: This is bufferable since aheight won't change for a
//       while and fbline depends on aline and aheight
static inline int alines_in_fbline(int aline, int aheight, int fbline) {
    return (((fbline + 1) * aheight) / 144) - aline;
}

// TODO: This should be buffered - it is executed for every lcd-pixel
static inline int acolumns_in_fbpixel(int acolumn, int awidth, int fbx) {
    return (((fbx + 1) * awidth) / 160) - acolumn;
}

static inline void fw_render_buffer(SDL_Surface *surface, void *buf, int buflines, int y) {
    memcpy(surface->pixels + bytes_per_line * y, buf, buflines * bytes_per_line);
}

static inline void cgb_fw_render_fbline(int line, u8 **colorpool) {
    int fbx, sx, b;
    int pixels_to_set;

    for(fbx = 0, b = 0, sx = 0; fbx < 160; fbx++) {
        pixels_to_set = acolumns_in_fbpixel(sx, area.w, fbx);

        u16 col = lcd.clean_fb[line * 160 + fbx];
        memcpy(&buf[b], colorpool[col], pixels_to_set * bytes_per_pixel);

        b += pixels_to_set * bytes_per_pixel;
        sx += pixels_to_set;
    }
}

static void fw_render_fbline(int line) {
    if(emu.hw == CGB_HW) {
        switch(bytes_per_pixel) {
            case 1: cgb_fw_render_fbline(line, rgb8.colorpool); break;
            case 2: cgb_fw_render_fbline(line, rgb16.colorpool); break;
            case 3: cgb_fw_render_fbline(line, rgb24.colorpool); break;
            case 4: cgb_fw_render_fbline(line, rgba32.colorpool); break;

            default:
                assert(0);
        }
    }
    else {

    }
}

static void fullwidth_render(SDL_Surface *surface, SDL_Rect _area) {
    int i, aline, buflines, fbline, alines_to_fill;

    area = _area;
    buflines = 0;
    fbline = 0;
    line_length = surface->w;
    bytes_per_line = surface->pitch;
    bytes_per_pixel = surface->format->BytesPerPixel;

    for(aline = 0; aline < area.h; fbline++) {
        alines_to_fill = alines_in_fbline(aline, area.h, fbline);
        if(alines_to_fill > 0) {
            fw_render_fbline(fbline);
            buflines = 1;

            for(;alines_to_fill >= buflines * 2; buflines *= 2) {
                memcpy(&buf[buflines * bytes_per_line], buf, buflines * bytes_per_line);
            }
            fw_render_buffer(surface, buf, buflines, aline);
            aline += buflines;
            if(alines_to_fill > buflines) {
                fw_render_buffer(surface, buf, alines_to_fill - buflines, aline);
            }
        }
    }
}

static void area_render(SDL_Surface *surface, SDL_Rect area) {

}

void sdl_video_init() {
    int c, p;

    rgb16.colorpool = malloc(sizeof(*rgb16.colorpool) * 0x10000);
    for(c = 0; c < 0x10000; c++) {
        rgb16.colorpool[c] = malloc(24);
        for(p = 0; p < 24;) {
            u8 r, g, b;
            u16 val;

            r = ((c >> 10) & 0x001F);
            g = ((c >> 5) & 0x001F);
            b = ((c >> 0) & 0x001F);

            val = (b<<11) | (g<<6) | (r<<0);

            rgb16.colorpool[c][p++] = val & 0x00FF;
            rgb16.colorpool[c][p++] = val >> 8;
        }
    }
    rgb24.colorpool = malloc(sizeof(*rgb24.colorpool) * 0x10000);
    for(c = 0; c < 0x10000; c++) {
        rgb24.colorpool[c] = malloc(24);
        for(p = 0; p < 24;) {
            rgb24.colorpool[c][p++] = ((c >> 10) & 0x001F) << 3;
            rgb24.colorpool[c][p++] = ((c >> 5) & 0x001F) << 3;
            rgb24.colorpool[c][p++] = ((c >> 0) & 0x001F) << 3;
        }
    }
}

void sdl_video_render(SDL_Surface *surface, SDL_Rect area) {
    static time_t last = 0;

    if(last == 0)
        last = SDL_GetTicks();

    if(SDL_GetTicks() - last < 16)
        return;
    else
        last += 16;

    if(area.x == 0 && area.w == surface->w) {
        fullwidth_render(surface, area);
    }
    else {
        area_render(surface, area);
    }
}

