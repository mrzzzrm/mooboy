#include "framerate.h"
#include "performance.h"
#include "sys/sys.h"
#include "core/cpu.h"
#include "core/moo.h"

framerate_t framerate;

void framerate_init() {
    framerate.limit_framerate = 1;
    framerate.frameskip = -1;
    framerate.max_frameskip = 5;
    framerate.delay_threshold = 1;

    framerate.cc_ahead = 0;
    framerate.last_curb_ticks = 0;
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
    framerate.last_curb_ticks = 0;
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

void framerate_curb() {
    int period = sys.ticks - (long)framerate.last_curb_ticks;

    framerate.cc_ahead += sys.invoke_cc;
    framerate.cc_ahead -= (period * cpu.freq)/1000;

    int ms_ahead = framerate.cc_ahead / ((long)cpu.freq/1000);

    if(ms_ahead >= framerate.delay_threshold) {
        sys_delay(framerate.delay_threshold);
        performance.counting.slept += framerate.delay_threshold;
    }

    framerate.last_curb_ticks = sys.ticks;
}

