#include "framerate.h"
#include "performance.h"
#include "sys/sys.h"
#include "core/cpu.h"
#include "core/moo.h"
#include <stdio.h>

#define max(a, b) ((a) > (b) ? (a) : (b))

framerate_t framerate;

void framerate_init() {
    framerate.frameskip = -1;
    framerate.max_frameskip = 5;

    framerate.cc_ahead = 0;
    framerate.skipped = 0;
    framerate.first_frame_ticks = 0;
    framerate.framecount = 0;
}

void framerate_reset() {
    framerate.framecount = 0;
}

void framerate_begin() {
    framerate.first_frame_ticks = sys.ticks;
    framerate.cc_ahead = 0;
}

int framerate_next_frame() {
    unsigned int should_framecount;
    int next_frame;

    // Check if we're in time for a next frame
    should_framecount = ((sys.ticks - framerate.first_frame_ticks) * 60) / 1000;
    if(should_framecount <= framerate.framecount || !sys.fb_ready) {
        return 0;
    }

    // Fixed frameskip
    if(framerate.frameskip >= 0) {
        if(framerate.skipped >= framerate.frameskip) {
            framerate.skipped = 0;
            next_frame = 1;
        }
        else {
            framerate.skipped++;
            next_frame = 0;
        }
    }
    else {
        // Autoframeskip, but we skipped too many
        if(framerate.skipped >= framerate.max_frameskip) {
            framerate.skipped = 0;
            next_frame = 1;
        }
        // Autoframeskip, shall we?
        else if(should_framecount > framerate.framecount + 1) {
            framerate.skipped++;
            performance.counting.skipped++;
            next_frame = 0;
        }
        else {
            next_frame = 1;
        }
    }

    framerate.framecount = should_framecount;
    return next_frame;
}

