#include "performance.h"
#include "core/cpu.h"
#include "core/moo.h"
#include "sys/sys.h"
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#define STATUSLABEL_MAX_LENGTH 64

performance_t performance;

void performance_init() {
    performance.update_period = 250;
}

void performance_begin() {
    memset(&performance, 0x00, sizeof(performance));
}

void performance_invoked() {

    performance.update_cc += sys.invoke_cc;

    if(sys.ticks < performance.last_update_ticks + performance.update_period) {
        return;
    }

    performance.speed = (float)(performance.update_cc * 1000.0 * 100.0) / (cpu.freq * performance.update_period);

    memcpy(&performance.counters, &performance.counting, sizeof(performance.counting));
    memset(&performance.counting, 0x00, sizeof(performance.counting));
    performance.update_cc = 0;

    performance.last_update_ticks = sys.ticks;

    sys_new_performance_info();
}

