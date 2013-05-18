#include <stdio.h>
#include <stdlib.h>
//#include <SDL/SDL.h>

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
    sys_error();
}

static void load_romfile(char *romname) {
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
    emu_set_hw(CGB_HW);
    sprintf(rom, "instr_timing.gb");
    load_romfile(rom);
    emu_run();
    close();

    return EXIT_SUCCESS;
}
