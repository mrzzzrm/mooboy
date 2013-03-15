#ifndef IO_H
#define IO_H

#include "util/defines.h"

u8 io_read(u16 adr);
void io_write(u16 adr, u8 val);

#endif
