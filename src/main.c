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

    emu_init();
    sys_init(argc, argv);
}

void error() {
    printf("Error: %s\n", err_msg());
    sys_error();
}

void load_romfile(char *romname) {
    char rompath[256];
    u8 *romdata;
    size_t romsize;

    sprintf(rompath, "rom/%s", romname);
    printf("Loading ROM: %s\n", rompath);
    if((romdata = io_load_binary(rompath, &romsize)) == NULL) {
        error();
    }
    printf("Firing emu with romdata (size=%i)\n", romsize);
    emu_load_rom(romdata, romsize);
    free(romdata);
}

int main(int argc, const char **argv) {
    char rom[256];


    init(argc, argv);
    sprintf(rom, "gold.gbc");
    load_romfile(rom);
    emu_run();
    close();

    return EXIT_SUCCESS;
}
