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
    framerate.delay_threshold = 3;
    framerate.cc_ahead = 0;
    framerate.last_curb_ticks = 0;
    framerate.skipped = 0;
    framerate.first_frame_ticks = 0;
    framerate.framecount = 0;
}

void framerate_begin() {
    framerate.first_frame_ticks = sys.ticks;
    framerate.cc_ahead = 0;
    framerate.last_curb_ticks = 0;
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
    should_framecount = ((sys.ticks - framerate.first_frame_ticks) * 60) / 1000;

    if(should_framecount > framerate.framecount+1 || framerate.frameskip >= 0) {
        framerate.framecount = should_framecount;
        framerate.skipped++;
        performance.counting.skipped++;
        return 1;
    }

    return 0;
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

