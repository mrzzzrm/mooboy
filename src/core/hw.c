#include "hw.h"
#include <stdlib.h>
#include <stdio.h>

hw_event_queue_t hw_event_queue;


static void schedule() {
    hw_event_t *event, *next;

    if(hw_event_queue.schedule == NULL)
        return;

    for(event = hw_event_queue.first; event != NULL; event = event->next) {
        printf("%s[%i/%i] ", event->name, event->cc, event->mcs);
    }
    printf("-> ");

    for(; hw_event_queue.schedule != NULL; hw_event_queue.schedule = next) {
        event = hw_event_queue.schedule;
        next = event->next;

        event->next = hw_event_queue.first;
        hw_event_queue.first = event;

        hw_event_t *prev = NULL;
        while(event->next != NULL) {
            if(event->mcs > event->next->mcs) {
                printf("Switching %s and %s\n", event->name, event->next->name);
                if(prev == NULL) {
                    hw_event_queue.first = event->next;
                }
                else {
                    prev->next = event->next;
                }

                prev = event->next;
                event->next = event->next->next;
                prev->next = event;
            }
            else {
                break;
            }
        }
    }
    for(event = hw_event_queue.first; event != NULL; event = event->next) {
        printf("%s[%i/%i] ", event->name, event->cc, event->mcs);
    }
    printf("\n");
}

static void event_queue() {
    hw_event_t *event;

    if(hw_event_queue.first != NULL) {
        if(hw_event_queue.cc >= hw_event_queue.first->cc) {

            printf("(1) [");
            for(event = hw_event_queue.first; event != NULL; event = event->next) {
                printf("%s[%i/%i] ", event->name, event->cc, event->mcs);
            }
            printf("]-> ");

            do {
                hw_event_queue.first->cc -= hw_event_queue.cc;
                printf("Exec{%s} ", hw_event_queue.first->name);
               hw_event_queue.first->callback(hw_event_queue.first->cc);
                hw_event_queue.first = hw_event_queue.first->next;
                printf("Front{%p} ", hw_event_queue.first);
            } while(hw_event_queue.first != NULL && hw_event_queue.first->mcs >= hw_event_queue.first->cc);

            printf("(2) [");
            for(event = hw_event_queue.first; event != NULL; event = event->next) {
                printf("%s[%i/%i] ", event->name, event->cc, event->mcs);
            }
            printf("]-> ");

            for(event = hw_event_queue.first; event != NULL; event = event->next) {
                event->cc -= hw_event_queue.cc;
                event = event->next;
            }
            hw_event_queue.cc = 0;

            printf("(3) [");
            for(event = hw_event_queue.first; event != NULL; event = event->next) {
                printf("%s[%i/%i] ", event->name, event->cc, event->mcs);
            }
            printf("]\n");
        }
    }
}

void hw_init() {
    hw_event_queue.cc = 0;
    hw_event_queue.first = NULL;
    hw_event_queue.schedule = NULL;
}

void hw_step(int mcs) {
    hw_event_queue.cc += mcs;

    schedule();
    event_queue();
}

void hw_schedule(hw_event_t *event) {
    printf("New %s\n", event->name);
    event->next = hw_event_queue.schedule;
    hw_event_queue.schedule = event;
}

