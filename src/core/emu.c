#include "emu.h"
#include "cpu/defines.h"
#include "io/lcd.h"
#include "debug.h"
#include "loader.h"

bool verbose;
u16 debug_limit;

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

static void debug_console() {
    if(verbose) {
        char str[256];
        gets(str);
        fflush(stdin);
        if(str[0] == '\n') {
            return;
        }
        else {
            verbose = false;
            debug_limit = strtol(str, NULL, 16);
        }
    }
    else {
        if(PC >= debug_limit)
            verbose = true;
    }
}

bool emu_run() {
    verbose = 1;
    printf("Starting emulation\n");
    for(;;) {
        if(verbose) {
            printf("Emulating opcode @ PC=%X\n", PC);
            debug_print_cpu_state();
            printf("{\n");
            debug_before();
        }
        cpu_emulate(1);
        if(verbose) {
            debug_after();
            debug_print_diff();
            printf("}\n");
        }
        debug_console();

        SDL_FillRect(NULL, 0, SDL_GetVideoSurface());
        unsigned int l;
        for(l = 0; l < 144; l++) {
            lcd.ly = l;
            lcd_drawl();
        }
        SDL_Flip(SDL_GetVideoSurface());
    }
    return true;
}

