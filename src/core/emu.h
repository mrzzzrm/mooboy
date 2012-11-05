#ifndef EMU_H
#define EMU_H

    #include "cpu.h"
    #include "mem.h"
    #include "util/defines.h"
    #include "util/err.h"
    #include "util/io.h"

    void emu_init();
    void emu_close();

    bool emu_load(u8 *data, size_t size);
    bool emu_run();

#endif // EMU_H
