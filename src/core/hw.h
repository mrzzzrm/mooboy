#ifndef CORE_HW_H
#define CORE_HW_H

#include "defines.h"

typedef u32 hw_cycle_t;

typedef struct hw_event_s {
    void (*callback)(int);
    struct hw_event_s *next;
    hw_cycle_t mcs;
#ifdef DEBUG
    char name[64];
    int dbg_queued;
#endif
} hw_event_t;

typedef struct {
    hw_cycle_t cc;
    hw_cycle_t defered;
    hw_event_t *first, *sched;
} hw_events_t;

extern hw_events_t hw_events;


void hw_reset();
void hw_step(int mcs);
void hw_schedule(hw_event_t *event, int mcs);
void hw_unschedule(hw_event_t *del);
void hw_defer(hw_cycle_t mcs);

#endif
