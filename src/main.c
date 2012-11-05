#include <stdio.h>
#include <stdlib.h>

#include "core/emu.h"
#include "sys/sys.h"
#include "util/err.h"

void close() {
    emu_close();
    sys_close();
}

void init(int argc, const char **argv) {
    atexit(&close);

    sys_init(argc, argv);
    emu_init();
}

void error() {
    printf("Error: %s\n", err_msg());
    sys_error();
}

void load_rom() {
    const char *rompath;
    u8 *romdata;
    size_t romsize;

    if((rompath = sys_get_rompath()) == NULL) {
        error();
    }
    if((romdata = io_load_binary(rompath, &romsize)) == NULL) {
        err_set(ERR_ROM_NOT_FOUND);
        error();
    }
    if(!emu_load(romdata, romsize)) {
        error();
    }
    free(romdata);
}

int main(int argc, const char **argv) {
    init(argc, argv);
    while(sys_running()) {
        if(sys_new_rom()) {
            load_rom();
        }
        if(!emu_run()) {
            error();
        }
    }
    close();

    return EXIT_SUCCESS;
}
