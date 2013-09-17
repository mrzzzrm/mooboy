#ifndef CORE_HW_H
#define CORE_HW_H

typedef struct hw_event_s {
    int cc;
    int mcs;
    void (*callback)(int);
    struct hw_event_s *next;
    char name[64];
} hw_event_t;

typedef struct {
    hw_event_t *first;
    hw_event_t *schedule;
    int cc;
} hw_event_queue_t;

extern hw_event_queue_t hw_event_queue;


void hw_init();
void hw_step(int mcs);
void hw_schedule(hw_event_t *event);

#endif
