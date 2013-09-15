#include "performance.h"
#include "core/cpu.h"
#include "core/moo.h"
#include "sys/sys.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>


performance_t performance;

const int PERFORMANCE_UPDATE_PERIOD = 250;

void performance_reset() {
    memset(&performance, 0x00, sizeof(performance));
}

void performance_invoked() {
    performance.update_cc += sys.invoke_cc;

    if(sys.ticks < performance.last_update_ticks + PERFORMANCE_UPDATE_PERIOD) {
        return;
    }

    performance.speed = (float)(performance.update_cc * 1000.0 * 100.0) / (cpu.freq * PERFORMANCE_UPDATE_PERIOD);

    memcpy(&performance.counters, &performance.counting, sizeof(performance.counting));
    memset(&performance.counting, 0x00, sizeof(performance.counting));
    performance.update_cc = 0;

    performance.last_update_ticks = sys.ticks;

    sys_new_performance_info();
}

