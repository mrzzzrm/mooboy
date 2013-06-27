#include <SDL/SDL.h>
#include <assert.h>
#include "util/defines.h"
#include "sys/sys.h"
#include "core/emu.h"
#include "core/lcd.h"


static u8 buf[32768];
static u16 fb_color[160][144];
static int line_length;
static int bytes_per_pixel;
static int bytes_per_line;
static SDL_Rect area;

sys_t sys;

static struct {
    u8 **colorpool;
} rgb8, rgb16, rgb24, rgba32;


// TODO: This is bufferable since aheight won't change for a
//       while and fbline depends on aline and aheight
//       9,600/s
static inline int alines_in_fbline(int aline, int aheight, int fbline) {
    return (((fbline + 1) * aheight) / 144) - aline;
}

// TODO: This should be buffered - it is executed for every lcd-pixel
//       1,382,400/s
static inline int acolumns_in_fbpixel(int acolumn, int awidth, int fbx) {
    return (((fbx + 1) * awidth) / 160) - acolumn;
}

static inline void fw_render_buffer(SDL_Surface *surface, void *buf, int buflines, int y) {
    memcpy(surface->pixels + bytes_per_line * y, buf, buflines * bytes_per_line);
}

static inline void cgb_fw_render_fbline(int line) {
    int fb_pixel, buf_pos, ax, ppos, fbx;
    u16 s_color, fb_color;
    u16 r, g, b;
    int pixels_to_set;

    buf_pos = 0;
    fb_pixel = line * 160;
    fbx = 0;

    for(ax = 0; ax < area.w; fbx++) {
        fb_color = lcd.clean_fb[fb_pixel++];

        r = (fb_color & (0x001F << 10));
        g = (fb_color & (0x001F << 5));
        b = (fb_color & 0x001F);

        s_color = (b<<11) | (g << 1) | (r>>10);

        pixels_to_set = acolumns_in_fbpixel(ax, area.w, fbx);

        for(ppos = 0; ppos < pixels_to_set; ppos++, ax++) {
            buf[buf_pos++] = s_color & 0x00FF;
            buf[buf_pos++] = s_color >> 8;
        }
    }

}

static void fw_render_fbline(int line) {
    if(emu.hw == CGB_HW) {
        cgb_fw_render_fbline(line);
    }
    else {

    }
}

static void fullwidth_render(SDL_Surface *surface, SDL_Rect _area) {
    int aline, buflines, fbline, alines_to_fill;

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
            aline += alines_to_fill - buflines;
        }
    }
}

static void area_render(SDL_Surface *surface, SDL_Rect area) {
    int x, y;
    int fbx = 0, fby = 0;

    for(x = 0; x < 160; x++) {
        for(y = 0; y < 144; y++) {
            u8 r, g, b;
            u16 val, c;

            c = lcd.clean_fb[y * 160 + x];

            r = ((c >> 10) & 0x001F);
            g = ((c >> 5) & 0x001F);
            b = ((c >> 0) & 0x001F);

            val = (b<<11) | (g<<6) | (r<<0);

            fb_color[x][y] = val;
        }
    }

    for(y = 0; y < area.h; y++) {
        u16 *line = &surface->pixels[(y+area.y) * surface->pitch];
        fby = (y * 144) / area.h;

        for(x = 0; x < area.w; x++, fbx++) {
            fbx = (x * 160) / area.w;
            line[x+area.x] = fb_color[fbx][fby];
        }
    }
}

void video_init() {
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

void video_render(SDL_Surface *surface, SDL_Rect area) {
    if(area.x == 0 && area.w == surface->w) {
        fullwidth_render(surface, area);
    }
    else {
        area_render(surface, area);
    }
}

