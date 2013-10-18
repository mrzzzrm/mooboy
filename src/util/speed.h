#ifndef UTIL_SPEED_H
#define UTIL_SPEED_H

#include <time.h>

#define SPEED_MAX_FACTOR 10
#define SPEED_DELAY_THRESHOLD 1

typedef struct {
    int factor;
    int cc_ahead;
    time_t last_limit_check;
} speed_t;

extern speed_t speed;

void speed_begin();
void speed_limit();
void speed_set_factor(int factor);

#endif
