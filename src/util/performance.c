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
    performance.statuslabel = SDL_CreateRGBSurface(0, STATUSLABEL_MAX_LENGTH * 8, 8, sys.bits_per_pixel, 0, 0, 0, 0);
    //SDL_SetColorKey(performance.statuslabel, SDL_SRCCOLORKEY, 0x00000000);
    assert(performance.statuslabel);
}

void performance_begin() {
    performance.slept = 0;
    performance.skipped = 0;
    performance.invokes = 0;
    performance.frames = 0;
    performance.speed = 0;
    performance.last_update_ticks = 0;
    performance.update_cc = 0;
    performance.update_period = 250;
}

void performance_invoked() {
    char statusline[STATUSLABEL_MAX_LENGTH + 1];

    performance.update_cc += sys.invoke_cc;

    if(sys.ticks < performance.last_update_ticks + performance.update_period) {
        return;
    }

    performance.speed = (double)(performance.update_cc * 1000.0 * 100.0) / (cpu.freq * performance.update_period);
    snprintf(statusline, sizeof(statusline), "Skipped %i/%i Slept %6.2f %% frames, speed: %6.2f %%", performance.skipped, performance.frames, (float)performance.slept*100/performance.update_period, performance.speed);

    SDL_FillRect(performance.statuslabel, NULL, 0);
    stringColor(performance.statuslabel, 0, 0, statusline, 0xaaaaaaff);

    performance.slept = 0;
    performance.skipped = 0;
    performance.frames = 0;
    performance.update_cc = 0;

    performance.last_update_ticks = sys.ticks;
}
