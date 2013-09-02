#ifndef CORE_SERIAL_H
#define CORE_SERIAL_H

#include "util/defines.h"

typedef struct {
    u8 sb;
    u8 sc;

    u32 last_shift_cc;
    u32 external_period;
    u32 internal_period;
    int num_in, num_out;
} serial_t;

extern serial_t serial;

void serial_reset();
void serial_step();
void serial_sc_write(u8 val);
void serial_update_internal_period();

#endif
