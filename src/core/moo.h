#ifndef CORE_MOO_H
#define CORE_MOO_H

#include "defines.h"
#include <time.h>

#define DMG_HW 0
#define CGB_HW 1

#define CGB_MODE 0
#define NON_CGB_MODE 1

#define MOO_ROM_LOADED_BIT  0x01
#define MOO_ROM_RUNNING_BIT 0x02
#define MOO_ERROR_BIT       0x04
#define MOO_RUNNING_BIT     0x08


typedef struct moo_error_s {
    char text[256];
} moo_error_t;


typedef struct {
    int hw;
    int mode;
    int state;
    char title[17];
    moo_error_t *error;
} moo_t;

extern moo_t moo;

void moo_init();
void moo_reset();
void moo_close();

void moo_begin();
void moo_pause();
void moo_continue();
void moo_quit();

void moo_load_rom(const char *path);
void moo_load_rom_config();

void moo_run();

void moo_step_hw(int mcs);

void moo_set_joy_button(u8 button, u8 state);

void moo_set_hw(int hw);

void moo_notifyf(const char *format, ...);
void moo_errorf(const char *format, ...);
void moo_fatalf(const char *format, ...);
void moo_error_clear();

#endif
