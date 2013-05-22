#ifndef CORE_EMU_H
#define CORE_EMU_H

#include "util/defines.h"

#define DMG_HW 0
#define CGB_HW 1

#define CGB_MODE 0
#define NON_CGB_MODE 1

typedef struct {
    int hw;
    int mode;
} emu_t;

extern emu_t emu;

void emu_init();
void emu_reset();
void emu_close();

void emu_load_rom(u8 *data, size_t size);
void emu_run();
void emu_step_hw(u8 mcs);
void emu_set_joy_button(u8 button, u8 state);
void emu_set_hw(int hw);

u8 *emu_get_savestate(size_t *size);
void emu_load_savestate(u8 *data, size_t size);

int emu_persistent_sram();
u8 *emu_get_sram(size_t *size);
void emu_load_sram(u8 *data, size_t *size);

#endif
