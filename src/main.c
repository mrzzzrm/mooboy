#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "util/io.h"
#include "core/moo.h"
#include "sys/sys.h"

static void close() {
    if(sys.rom_loaded) {
        sys_save_card();
    }

    moo_close();
    sys_close();
}

static void init(int argc, const char **argv) {
    atexit(&close);
    moo_set_hw(CGB_HW);
    moo_init();
    sys_init(argc, argv);
}

int main(int argc, const char **argv) {
    init(argc, argv);

    moo_run();
    close();


    return EXIT_SUCCESS;
}

