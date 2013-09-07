#include <SDL/SDL.h>
#include <assert.h>
#include "core/defines.h"
#include "sys/sys.h"
#include "core/moo.h"
#include "core/lcd.h"


static u8 buf[32768];
static u16 fb_color[160][144];
static int line_length;
static int line_byte_offset;
static int bytes_per_pixel;
static int bytes_per_line;
static SDL_Rect area;

static u16 dmg_palette[4] = {
    0x7bde, 0x5294, 0x294a, 0x0000
};

sys_t sys;


static inline u16 cgb_to_rgb(u16 cgb_color) {
    u16 r, g, b;

    r = (cgb_color & (0x001F << 10));
    g = (cgb_color & (0x001F << 5));
    b = (cgb_color & 0x001F);

    return (b<<11) | (g << 1) | (r>>10);
}

static inline u16 dmg_to_rgb(u16 dmg_color) {
    return dmg_palette[dmg_color];
}

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

static void cgb_fw_render_fbline(int line) {
    int fb_pixel, buf_pos, ax, ppos, fbx;
    u16 s_color;
    int pixels_to_set;

    buf_pos = line_byte_offset;
    fb_pixel = line * 160;
    fbx = 0;

    for(ax = 0; ax < area.w; fbx++) {
        s_color = cgb_to_rgb(lcd.clean_fb[fb_pixel++]);
        pixels_to_set = acolumns_in_fbpixel(ax, area.w, fbx);

        for(ppos = 0; ppos < pixels_to_set; ppos++, ax++) {
            buf[buf_pos++] = s_color & 0x00FF;
            buf[buf_pos++] = s_color >> 8;
        }
    }
}

static void dmg_fw_render_fbline(int line) {
    int fb_pixel, buf_pos, ax, ppos, fbx;
    u16 s_color;
    int pixels_to_set;

    buf_pos = line_byte_offset;
    fb_pixel = line * 160;
    fbx = 0;

    for(ax = 0; ax < area.w; fbx++) {
        s_color = dmg_to_rgb(lcd.clean_fb[fb_pixel++]);
        pixels_to_set = acolumns_in_fbpixel(ax, area.w, fbx);

        for(ppos = 0; ppos < pixels_to_set; ppos++, ax++) {
            buf[buf_pos++] = s_color & 0x00FF;
            buf[buf_pos++] = s_color >> 8;
        }
    }
}

static void fw_render_fbline(int line) {
    if(moo.hw == CGB_HW) {
        cgb_fw_render_fbline(line);
    }
    else {
        dmg_fw_render_fbline(line);
    }
}

static void fullwidth_render(SDL_Surface *surface, SDL_Rect _area) {
    int aline, buflines, fbline, alines_to_fill;

    area = _area;
    buflines = 0;
    fbline = 0;
    line_length = surface->w;
    line_byte_offset = 0;
    bytes_per_line = surface->pitch;
    bytes_per_pixel = surface->format->BytesPerPixel;

    for(aline = 0; aline < area.h; fbline++) {
        alines_to_fill = alines_in_fbline(aline, area.h, fbline);
        if(alines_to_fill > 0) {
            fw_render_fbline(fbline);

            for(buflines = 1; alines_to_fill >= buflines * 2; buflines *= 2) {
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


static void area_render(SDL_Surface *surface, SDL_Rect _area) {
    int aline, buflines, fbline, alines_to_fill;

    area = _area;
    buflines = 0;
    fbline = 0;
    bytes_per_line = surface->pitch;
    bytes_per_pixel = surface->format->BytesPerPixel;
    line_length = area.w;
    line_byte_offset = area.x * bytes_per_pixel;

    for(aline = 0; aline < area.h; fbline++) {
        alines_to_fill = alines_in_fbline(aline, area.h, fbline);
        if(alines_to_fill > 0) {
            fw_render_fbline(fbline);

            for(buflines = 1; alines_to_fill >= buflines * 2; buflines *= 2) {
                memcpy(&buf[buflines * bytes_per_line], buf, buflines * bytes_per_line);
            }
            fw_render_buffer(surface, buf, buflines, aline + area.y);
            aline += buflines;
            if(alines_to_fill > buflines) {
                fw_render_buffer(surface, buf, alines_to_fill - buflines, aline + area.y);
            }
            aline += alines_to_fill - buflines;
        }
    }
/*
    int x, y;
    int fbx = 0, fby = 0;


    if(moo.hw == CGB_HW) {
        for(x = 0; x < 160; x++) {
            for(y = 0; y < 144; y++) {
                fb_color[x][y] = cgb_to_rgb(lcd.clean_fb[y * 160 + x]);
            }
        }
    }
    else {
        for(x = 0; x < 160; x++) {
            for(y = 0; y < 144; y++) {
                fb_color[x][y] = dmg_to_rgb(lcd.clean_fb[y * 160 + x]);
            }
        }
    }

    for(y = 0; y < area.h; y++) {
        u16 *line = (u16*)&((u8*)surface->pixels)[(y+area.y) * surface->pitch];
        fby = (y * 144) / area.h;

        for(x = 0; x < area.w; x++, fbx++) {
            fbx = (x * 160) / area.w;
            line[x+area.x] = fb_color[fbx][fby];
        }
    }
*/
}

void video_init() {

}

void video_render(SDL_Surface *surface, SDL_Rect area) {
    if(area.x == 0 && area.w == surface->w) {
        fullwidth_render(surface, area);
    }
    else {
        area_render(surface, area);
    }
}

