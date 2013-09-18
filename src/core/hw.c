#include "hw.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

hw_events_t hw_events;

void hw_reset() {
    hw_events.cc = 0;
    hw_events.first = NULL;
}

void hw_step(int mcs) {
    hw_event_t *next;
    hw_events.cc += mcs;
    while(hw_events.first != NULL && hw_events.first->mcs >= mcs) {
        next = hw_events.first->next;
        hw_events.first->callback(mcs - hw_events.first->mcs);
        hw_events.first = next;
    }
}

void hw_schedule(hw_event_t *sched, int mcs) {
    hw_event_t *event, *prev;
    int exec_mcs = hw_events.cc + mcs;

    for(event = hw_events.first, prev = NULL; event != NULL; event = event->next) {
        if(exec_mcs <= event->mcs) {
            sched->next = event;
            if(prev == NULL) {
                hw_events.first = sched;
            }
            else {
                prev->next = event;
            }
            return;
        }
        prev = event;
    }
    if(prev == NULL) {
        hw_events.first = sched;
    }
    else {
        prev->next = sched;
    }
    sched->next = NULL;
}

void hw_unschedule(hw_event_t *del) {
    hw_event_t *event, *prev;
    for(event = hw_events.first, prev = NULL; event != NULL; event = event->next) {
        if(event == del) {
            if(prev != NULL) {
                prev->next = event->next;
            }
            else {
                hw_events.first = event->next;
            }
        }
        prev = event;
    }
}


