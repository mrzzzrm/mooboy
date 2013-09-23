#include "hw.h"
#include "cpu.h"
#include "lcd.h"
#include "rtc.h"
#include "sound.h"
#include "timers.h"
#include "sys/sys.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

hw_t hw;

void hw_reset() {
#ifdef DEBUG
    lcd_mode_0_event.dbg_queued = 0;
    lcd_mode_1_event.dbg_queued = 0;
    lcd_mode_2_event.dbg_queued = 0;
    lcd_mode_3_event.dbg_queued = 0;
    lcd_vblank_line_event.dbg_queued = 0;
    rtc_event.dbg_queued = 0;
    sound_mix_event.dbg_queued = 0;
    sound_envelopes_event.dbg_queued = 0;
    sound_length_counters_event.dbg_queued = 0;
    sound_sweep_event.dbg_queued = 0;
    timers_div_event.dbg_queued = 0;
    timers_tima_event.dbg_queued = 0;
#endif

    hw.cc = 0;
    hw.queue = NULL;
    hw.sched = NULL;
}

#ifdef DEBUG
//void hw_print_queue(hw_event_t *e) {
//    hw_event_t *c;
//    printf("%i: ", hw.cc);
//    for(c = e; c != NULL; c = c->next) {
//        fprintf(stdout, "[%s %i] ", c->name, c->mcs);
//        assert(c != c->next);
//    }
//    printf("\n");
//}
#endif

void hw_step(int mcs) {
    hw_event_t *next_sched, *next, *event, *prev;

#ifdef DEBUG
    assert(mcs <= 10);
    cpu.dbg_mcs += mcs;
#endif

    while(hw.sched != NULL) {
        next_sched = hw.sched->next;
        prev = NULL;

        for(event = hw.queue; event != NULL;) {
            next = event->next;
            assert(next != event);

            if((hw_cycle_t)(hw.sched->mcs - hw.cc) <= (hw_cycle_t)(event->mcs - hw.cc)) {
                hw.sched->next = event;
                if(prev == NULL) {
                    hw.queue = hw.sched;
                }
                else {
                    prev->next = hw.sched;
                }
                break;
            }

            prev = event;
            event = next;
        }

        if(event == NULL) {
            if(prev == NULL) {
                hw.queue = hw.sched;
            }
            else {
                prev->next = hw.sched;
            }
            hw.sched->next = NULL;
        }
        hw.sched = next_sched;
    }

    if(hw.queue != NULL) {
        hw.cc += mcs;

        while(hw.queue != NULL) {
            hw_cycle_t dist = hw.cc - hw.queue->mcs;
            if(dist <= mcs) {
                next = hw.queue->next;
#ifdef DEBUG
                assert(hw.queue->dbg_queued);
                hw.queue->dbg_queued = 0;
#endif
                hw.queue->callback(dist);
                hw.queue = next;
            }
            else {
                break;
            }
        }

        if(hw.defered > 0) {
            hw_cycle_t mcs = hw.defered;
            hw.defered = 0;
            hw_step(mcs);
        }
    }

    sys.invoke_cc += mcs;
}

void hw_schedule(hw_event_t *sched, int mcs) {
#ifdef DEBUG
    if(sched->dbg_queued) {
        printf("%s already queued\n", sched->name);
        assert(0);
    }
    sched->dbg_queued = 1;
#endif

    sched->mcs = hw.cc + mcs;
    sched->next = hw.sched;
    hw.sched = sched;
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

    unschedule_from_queue(&hw.queue, del);
    unschedule_from_queue(&hw.sched, del);
}

void hw_defer(hw_cycle_t mcs) {
    hw.defered += mcs;
}
