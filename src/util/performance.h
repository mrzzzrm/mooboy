#ifndef SYS_PERFORMANCE_H
#define SYS_PERFORMANCE_H

#include <SDL/SDL.h>

typedef struct {
    unsigned int slept;
    unsigned int skipped;
    unsigned int frames;
} performance_counters_t;

typedef struct {
    performance_counters_t counters, counting;

    float speed;

    time_t last_update_ticks;
    time_t update_period;
    int update_cc;
} performance_t;

extern performance_t performance;

void performance_init();
void performance_begin();
void performance_invoked();

#endif
