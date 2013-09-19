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

void hw_step(int mcs) {
    hw_event_t *next_sched, *next, *event, *prev;

    fprintf(stderr, "1 ");
    if(hw_events.first != NULL) {
        hw_events.cc += mcs;

        while(hw_events.first != NULL) {
            u32 dist = hw_events.cc - hw_events.first->mcs;
            if(dist <= mcs) {
                next = hw_events.first->next;
                hw_events.first->callback(dist);
                hw_events.first = next;
            }
            else {
                break;
            }
        }
    }

    fprintf(stderr, "2 ");
    while(hw_events.sched != NULL) {
        next_sched = hw_events.sched->next;
        prev = NULL;

        for(event = hw_events.first; event != NULL;) {
            next = event->next;

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
    fprintf(stderr, "3 ");
}

void hw_schedule(hw_event_t *sched, int mcs) {
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
    unschedule_from_queue(&hw_events.first, del);
    unschedule_from_queue(&hw_events.sched, del);
}


