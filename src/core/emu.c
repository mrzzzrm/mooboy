#include "emu.h"
#include "cpu/defines.h"
#include "debug.h"
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
    if(!load_rom(data, size)) {
        err_set(ERR_ROM_CORRUPT);
        return false;
    }
    emu_reset();

    return true;
}

bool emu_run() {
    printf("Starting emulation\n");
    for(;;) {
        printf("Emulating opcode @ PC=%X\n", PC);
        debug_print_cpu_state();
        printf("{\n");
        debug_before();
        cpu_emulate(1);
        debug_after();
        debug_print_diff();
        printf("}\n");
        getchar();
    }
    return true;
}

