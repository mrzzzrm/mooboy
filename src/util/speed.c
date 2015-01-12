#include "speed.h"
#include "core/cpu.h"
#include "sys/sys.h"
#include "util/performance.h"

speed_t speed;

void speed_reset() {
    speed.factor = 1;
}

void speed_begin() {
    speed.cc_ahead = 0;
    speed.last_limit_check = 0;
}

void speed_limit() {
    int period = sys.ticks - (long)speed.last_limit_check;

    speed.cc_ahead += sys.invoke_cc;
    speed.cc_ahead -= (period * cpu.freq * speed.factor)/1000;

    // Set a lower limit for cc_ahead, kinda random
    speed.cc_ahead = max(speed.cc_ahead, -cpu.freq/10);

    if(speed.factor < SPEED_MAX_FACTOR) {
        int ms_ahead = speed.cc_ahead / (((long)cpu.freq/1000));
        if(ms_ahead >= SPEED_DELAY_THRESHOLD) {
            sys_delay(SPEED_DELAY_THRESHOLD);
            performance.counting.slept += SPEED_DELAY_THRESHOLD;
        }
    }
    else {
        speed.cc_ahead = 0;
    }

    speed.last_limit_check = sys.ticks;
}

void speed_set_factor(int factor) {
    speed.factor = factor;
    sys_play_audio(sys.sound_on);
}

