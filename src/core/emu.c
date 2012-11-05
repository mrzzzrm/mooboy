#include "emu.h"

static void emu_reset() {
    mem_reset();
    cpu_reset();
}

void emu_init() {

}

void emu_close() {

}

bool emu_load(u8 *data, size_t size) {
    emu_reset();

    if(!mem_load_rom(data, size)) {
        err_set(ERR_ROM_CORRUPT);
        return false;
    }

    return true;
}

bool emu_run() {
    cpu_emulate(1);
    return true;
}

