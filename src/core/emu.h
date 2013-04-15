#ifndef EMU_H
#define EMU_H

#include "util/defines.h"

#define DMG_HW 0
#define CGB_HW 1

typedef struct {
    int type;
} hw_t;

extern hw_t hw;

void emu_init();
void emu_reset();
void emu_close();

void emu_load_rom(u8 *data, size_t size);
void emu_run();
void emu_run_standby();
void emu_set_joy_button(u8 button, u8 state);

u8 *emu_get_savestate(size_t *size);
void emu_load_savestate(u8 *data, size_t size);

int emu_persistent_sram();
u8 *emu_get_sram(size_t *size);
void emu_load_sram(u8 *data, size_t *size);

#endif
