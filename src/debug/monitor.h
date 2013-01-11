#ifndef DEBUG_MONITOR_H
#define DEBUG_MONITOR_H

    #include "util/defines.h"

    #define MONITOR_SYM_DMA 0x01

    void monitor_cell(u16 adr);
    void monitor_range(u16 from, u16 to);

#endif
