#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "core/emu.h"
#include "sys/sys.h"

static void close() {
    if(sys.rom_loaded) {
        sys_save_card();
    }

    emu_close();
    sys_close();
}

static void init(int argc, const char **argv) {
    atexit(&close);
    emu_set_hw(CGB_HW);
    emu_init();
    sys_init(argc, argv);
}

int main(int argc, const char **argv) {
    init(argc, argv);
    emu_run();
    close();

    return EXIT_SUCCESS;
}

