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

int main(int argc, const char **argv) {
    init(argc, argv);
    while(sys_running()) {
        if(sys_new_rom()) {
            const char *rom;

            if((rom = sys_get_rompath()) == NULL) {
                error();
            }
            if(!emu_load(rom)) {
                error();
            }
        }

        if(!emu_run()) {
            error();
        }
    }
    close();

    return EXIT_SUCCESS;
}
