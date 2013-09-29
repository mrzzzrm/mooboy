#ifndef SYS_ADJUST_FRAMERATE_H
#define SYS_ADJUST_FRAMERATE_H

#include <time.h>

typedef struct {
    int limit_framerate;
    int frameskip;
    int max_frameskip;

    int cc_ahead;
    int skipped;
    int delay_threshold;
    time_t first_frame_ticks;
    size_t framecount;
    time_t last_curb_ticks;
} framerate_t;

extern framerate_t framerate;

void framerate_init();
void framerate_reset();
void framerate_begin();
int framerate_next_frame();
void framerate_curb();

#endif // SYS_ADJUST_FRAMERATE_H
