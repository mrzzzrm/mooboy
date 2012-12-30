#ifndef FB_H
#define FB_H

    #include "util/defines.h"

    typedef struct {
        u8* data;
        unsigned int w, h;
        unsigned int poffset;
        unsigned int psize;
        unsigned int pitch;
        u8 *gbpalette[4];
    } fb_t;

    extern fb_t fb;

#endif // FB_H
