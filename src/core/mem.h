#ifndef CORE_MEM_H
#define CORE_MEM_H

#include "defines.h"

typedef struct {
    u8 rambanks[8][0x1000]; // GB - 2 banks; CGB - 8 banks
    u8 vrambanks[2][0x2000]; // 2nd bank for GBC
    u8 hram[0x80];
    u8 oam[0xA0];

    u8 *rambank;

    u8 rambank_index;
    u8 selected_vrambank;
} ram_t;

typedef struct {
    u8 srambanks[4][0x2000];
    u8 rombanks[256][0x4000];
    u16 romsize;
    u16 sramsize;
} card_t;


extern ram_t ram;
extern card_t card;

void mem_reset();

u8 mem_read_byte(u16 adr);
u16 mem_read_word(u16 adr);

void mem_write_byte(u16 adr, u8 val);
void mem_write_word(u16 adr, u16 val);

#endif
