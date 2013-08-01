#ifndef CORE_SERIAL_H
#define CORE_SERIAL_H

#include "util/defines.h"

typedef struct {
    u8 sb;
    u8 sc;

    u32 last_bit_cc;
    u32 external_period;
    u8 bits_transfered;
    u8 buf;
} serial_t;

extern serial_t serial;

void serial_reset();
void serial_step();
void serial_start_transfer();

#endif
