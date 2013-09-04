#ifndef CORE_IO_H
#define CORE_IO_H

#include "defines.h"

u8 io_read(u16 adr);
void io_write(u16 adr, u8 val);

#endif
