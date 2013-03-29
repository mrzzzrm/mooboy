#ifndef EMU_H
#define EMU_H

#include "util/defines.h"

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
