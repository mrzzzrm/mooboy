#include "hw.h"
#include "cpu.h"
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

static void print_queue(hw_event_t *e) {
    hw_event_t *c;
    for(c = e; c != NULL; c = c->next) {
        fprintf(stdout, "[%p %i] ", c, c->mcs);
        assert(c != c->next);
    }
}

void hw_step(int mcs) {
    hw_event_t *next_sched, *next, *event, *prev;

    if(hw_events.first != NULL) {
        hw_events.cc += mcs;

        while(hw_events.first != NULL) {
            u32 dist = hw_events.cc - hw_events.first->mcs;
            if(dist < mcs) {
                next = hw_events.first->next;
                hw_events.first->dbg_queued = 0;
                hw_events.first->callback(dist);
                hw_events.first = next;
            }
            else {
                break;
            }
        }
    }

//    fprintf(stdout, "1)Queue: "); print_queue(hw_events.first); fprintf(stdout, "X\n");
//    fprintf(stdout, "1)Sched: "); print_queue(hw_events.sched); fprintf(stdout, "X\n");
    while(hw_events.sched != NULL) {
        next_sched = hw_events.sched->next;
        prev = NULL;

        for(event = hw_events.first; event != NULL;) {
            next = event->next;
            assert(next != event);

            if((u32)(hw_events.sched->mcs - hw_events.cc) <= (u32)(event->mcs - hw_events.cc)) {
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
//    fprintf(stdout, "2)Queue: "); print_queue(hw_events.first); fprintf(stdout, "X\n");
//    fprintf(stdout, "2)Sched: "); print_queue(hw_events.sched); fprintf(stdout, "X\n");
}

void hw_schedule(hw_event_t *sched, int mcs) {
    if(sched->dbg_queued) {
        printf("%s already queued\n", sched->name);
        assert(0);
    }
    sched->dbg_queued = 1;

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
    del->dbg_queued = 0;

    unschedule_from_queue(&hw_events.first, del);
    unschedule_from_queue(&hw_events.sched, del);
}


