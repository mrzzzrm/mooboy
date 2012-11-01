#include "emu.h"

static void emu_reset() {
    mem_reset();
    cpu_reset();
}

static bool emu_start_rom(u8 *data, size_t size) {
    mem_set_rom(data, size);
}

void emu_init() {

}

void emu_close() {

}

bool emu_load(const char *path) {
    u8 *romdata;
    size_t romsize;

    if((romdata = io_load_binary(path, &romsize)) == NULL) {
        err_set(ERR_ROM_NOT_FOUND);
        return false;
    }

    emu_reset();

    if(!emu_start_rom(romdata, romsize)) {
        err_set(ERR_ROM_CORRUPT);
        return false;
    }

    return true;
}

bool emu_run() {
    cpu_emulate(1);
    return false;
}

