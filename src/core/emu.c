#include "emu.h"
#include "cpu/defines.h"
#include "loader.h"

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

    if(!load_rom(data, size)) {
        err_set(ERR_ROM_CORRUPT);
        return false;
    }

    return true;
}

bool emu_run() {
    printf("Starting emulation\n");
    for(;;) {
        printf("Emulating opcode\n\tPC=%i\n{\n", PC);
        cpu_emulate(1);
        printf("}\n");
    }
    return true;
}

