#include "framerate.h"
#include "performance.h"
#include "sys/sys.h"
#include "core/cpu.h"

framerate_t framerate;


void framerate_init() {
    framerate.limit_framerate = 1;
    framerate.frameskip = -1;
    framerate.max_frameskip = 5;
    framerate.delay_threshold = 10;

    framerate.skipped = 0;
    framerate.first_frame = 0;
    framerate.framecount = 0;
}

void framerate_begin() {
    framerate.first_frame = sys.ticks;
}

int framerate_skip() {
    unsigned int should_framecount;

    if(framerate.frameskip >= 0) {
        if(framerate.skipped >= framerate.frameskip) {
            framerate.skipped = 0;
            return 0;
        }
    }
    else {
        if(framerate.skipped >= framerate.max_frameskip) {
            framerate.skipped = 0;
            return 0;
        }
    }

    framerate.framecount++;
    should_framecount = ((sys.ticks - framerate.first_frame) * 60) / 1000;

    if(should_framecount > framerate.framecount || framerate.frameskip >= 0) {
        framerate.framecount = should_framecount;
        framerate.skipped++;
        performance.skipped++;
        return 1;
    }

    return 0;
}

void framerate_curb() {
    long should_cc = ((long)sys.ticks - (long)framerate.first_frame)*(long)(cpu.freq/1000);
    long is_cc = cpu.cc;
    long cc_ahead = is_cc - should_cc;
    long ms_ahead = cc_ahead / ((long)cpu.freq/1000);

    if(ms_ahead >= (long)framerate.delay_threshold) {
       SDL_Delay(framerate.delay_threshold);
       performance.slept += framerate.delay_threshold;
    }
}

