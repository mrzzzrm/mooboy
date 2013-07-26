#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "core/mbc.h"
#include "core/emu.h"
#include "sys/sys.h"
#include "util/err.h"
#include "util/io.h"

static void close() {
    emu_close();
    sys_close();
}

static void init(int argc, const char **argv) {
    atexit(&close);

    emu_init();
    sys_init(argc, argv);
}

static void error() {
    printf("Error: %s\n", err_msg());
    assert(0);
}

static void load_romfile() {
    u8 *romdata;
    size_t romsize;

    printf("Loading ROM: %s\n", sys.rompath);
    if((romdata = io_load_binary(sys.rompath, &romsize)) == NULL) {
        error();
    }
    printf("Firing emu with romdata (size=%i)\n", romsize);
    emu_load_rom(romdata, romsize);
    free(romdata);
}

int main(int argc, const char **argv) {
    init(argc, argv);
    emu_set_hw(CGB_HW);
    emu_run();

    sys_save_card();
    close();

    return EXIT_SUCCESS;
}
