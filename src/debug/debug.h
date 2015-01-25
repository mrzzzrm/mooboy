#ifndef DEBUG_DEBUG_H
#define DEBUG_DEBUG_H

#include "core/defines.h"

void debug_step();

u8 debug_read_byte(u16 addr);
u16 debug_read_word(u16 addr);

void debug_scope_begin();
void debug_scope_end();
int in_debug_scope();

#endif // DEBUG_DEBUG_H
