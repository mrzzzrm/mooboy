#ifndef SYS_PERFORMANCE_H
#define SYS_PERFORMANCE_H

#include <SDL/SDL.h>
#include "util/defines.h"

typedef struct {
    unsigned int slept;
    unsigned int skipped;
    unsigned int invokes;
    unsigned int frames;

    float speed;

    time_t last_update_ticks;
    time_t update_period;
    int update_cc;

    SDL_Surface *statuslabel;
} performance_t;

extern performance_t performance;

void performance_init();
void performance_begin();
void performance_invoked();

#endif
