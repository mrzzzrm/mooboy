#ifndef EMU_H
#define EMU_H

    #include "util/defines.h"
    #include "util/err.h"

    void emu_init();
    void emu_close();

    bool emu_load(const char *path);
    bool emu_run();

#endif // EMU_H
