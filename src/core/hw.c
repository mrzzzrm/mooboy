#include "hw.h"
#include "cpu.h"
#include "sys/sys.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

hw_events_t hw_events;

void hw_reset() {
    hw_events.cc = 0;
    hw_events.first = NULL;
    hw_events.sched = NULL;
}

#ifdef DEBUG
//static void print_queue(hw_event_t *e) {
//    hw_event_t *c;
//    for(c = e; c != NULL; c = c->next) {
//        fprintf(stdout, "[%s %i] ", c->name, c->mcs);
//        assert(c != c->next);
//    }
//}
#endif

void hw_step(int mcs) {
    hw_event_t *next_sched, *next, *event, *prev;

    if(hw_events.first != NULL) {
        hw_events.cc += mcs;

        while(hw_events.first != NULL) {
            hw_cycle_t dist = hw_events.cc - hw_events.first->mcs;
            if(dist <= mcs) {
                next = hw_events.first->next;
#ifdef DEBUG
                assert(hw_events.first->dbg_queued);
                hw_events.first->dbg_queued = 0;
#endif
                hw_events.first->callback(dist);
                hw_events.first = next;
            }
            else {
                break;
            }
        }

        if(hw_events.defered > 0) {
            hw_cycle_t mcs = hw_events.defered;
            hw_events.defered = 0;
            hw_step(mcs);
        }
    }

    while(hw_events.sched != NULL) {
        next_sched = hw_events.sched->next;
        prev = NULL;

        for(event = hw_events.first; event != NULL;) {
            next = event->next;
            assert(next != event);

            if((hw_cycle_t)(hw_events.sched->mcs - hw_events.cc) <= (hw_cycle_t)(event->mcs - hw_events.cc)) {
                hw_events.sched->next = event;
                if(prev == NULL) {
                    hw_events.first = hw_events.sched;
                }
                else {
                    prev->next = hw_events.sched;
                }
                break;
            }

            prev = event;
            event = next;
        }

        if(event == NULL) {
            if(prev == NULL) {
                hw_events.first = hw_events.sched;
            }
            else {
                prev->next = hw_events.sched;
            }
            hw_events.sched->next = NULL;
        }
        hw_events.sched = next_sched;
    }
}

void hw_schedule(hw_event_t *sched, int mcs) {
#ifdef DEBUG
    if(sched->dbg_queued) {
        printf("%s already queued\n", sched->name);
        assert(0);
    }
    sched->dbg_queued = 1;
#endif

    sched->mcs = hw_events.cc + mcs;
    sched->next = hw_events.sched;
    hw_events.sched = sched;
}

static void unschedule_from_queue(hw_event_t **q, hw_event_t *del) {
    hw_event_t *event, *prev;
    for(event = *q, prev = NULL; event != NULL; event = event->next) {
        if(event == del) {
            if(prev != NULL) {
                prev->next = event->next;
            }
            else {
                *q = event->next;
            }
        }
        prev = event;
    }
}

void hw_unschedule(hw_event_t *del) {
#ifdef DEBUG
    del->dbg_queued = 0;
#endif

    unschedule_from_queue(&hw_events.first, del);
    unschedule_from_queue(&hw_events.sched, del);
}

void hw_defer(hw_cycle_t mcs) {
    hw_events.defered += mcs;
}
