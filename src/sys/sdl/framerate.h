#ifndef SYS_ADJUST_FRAMERATE_H
#define SYS_ADJUST_FRAMERATE_H

#include <time.h>

typedef struct {
    int limit_framerate;
    int frameskip;
    int max_frameskip;

    unsigned int skipped;
    unsigned int delay_threshold;
    time_t first_frame;
    size_t framecount;
} framerate_t;

extern framerate_t framerate;

void framerate_init();
void framerate_begin();
int framerate_skip();
void framerate_curb();

#endif // SYS_ADJUST_FRAMERATE_H
