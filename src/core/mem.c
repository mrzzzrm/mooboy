#include "mem.h"
#include "defines.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "io.h"
#include "lcd.h"
#include "cpu.h"
#include "mbc.h"

ram_t ram;
card_t card;

static u8 read_locked_mem(u16 adr) {
#ifdef DEBUG
    printf("Denied read from locked memory location %.4X\n", adr);
#endif
    return 0xFF;
}

static void write_locked_mem(u16 adr, u8 val) {
#ifdef DEBUG
    printf("Denied write of %.2X to locked memory location %.4X\n", val, adr);
#endif
}

void mem_reset() {
    memset(ram.rambanks, 0x00, sizeof(ram.rambanks));
    memset(ram.hram, 0x00, sizeof(ram.hram));
    memset(ram.vrambanks, 0x00, sizeof(ram.vrambanks));
    memset(ram.oam, 0x00, sizeof(ram.oam));

    ram.rambank = ram.rambanks[1];
    ram.vrambank = ram.vrambanks[0];

    mbc.ram_selected = 1;

    ram.rambank_index = 1;
}

u8 mem_read_byte(u16 adr) {
    switch(adr >> 12) {
        case 0x0: case 0x1: case 0x2: case 0x3:
            return card.rombanks[0][adr];
        break;
        case 0x4: case 0x5: case 0x6: case 0x7:
            return mbc.rombank[adr - 0x4000];
        break;
        case 0x8: case 0x9:
            if((lcd.stat & 0x03) == 0x03 && (lcd.c & 0x80))
                return read_locked_mem(adr);
            else
                return ram.vrambank[adr - 0x8000];
        break;
        case 0xA: case 0xB:
            return mbc_upper_read(adr);
        break;
        case 0xC:
            return ram.rambanks[0][adr - 0xC000];
        break;
        case 0xD:
            return ram.rambank[adr - 0xD000];
        break;
        case 0xE:
            return mem_read_byte(adr - 0x2000);
        break;
        case 0xF:
            if(adr < 0xFE00) {
                return mem_read_byte(adr - 0x2000);
            }
            else if(adr >= 0xFE00 && adr < 0xFEA0) { // Sprite attributes
                if((lcd.stat & 0x03) > 0x01 && (lcd.c & 0x80))
                    return read_locked_mem(adr);
                else
                    return ram.oam[adr - 0xFE00];
            }
            else if(adr >= 0xFEA0 && adr < 0xFF00) { // Locked
                return read_locked_mem(adr);
            }
            else if(adr >= 0xFF00 && adr < 0xFF80) { // IO Registers
                return io_read(adr);
            }
            else if(adr >= 0xFF80 && adr < 0xFFFF) { // HiRAM
                return ram.hram[adr - 0xFF80];
            }
            else {
                return cpu.ie;
            }
        break;
    }

    assert(0);
    return 0x00; // ...and avoid warnings
}

u16 mem_read_word(u16 adr) {
    return (u16)mem_read_byte(adr) | ((u16)mem_read_byte(adr + 1) << 8);
}

void mem_write_byte(u16 adr, u8 val) {
    switch(adr >> 12) {
        case 0x0: case 0x1: case 0x2: case 0x3:
        case 0x4: case 0x5: case 0x6: case 0x7:
            mbc_lower_write(adr, val);
        return;
        case 0x8: case 0x9:
            if((lcd.stat & 0x03) != 0x03 || !(lcd.c & 0x80))
                ram.vrambank[adr - 0x8000] = val;
            else
                write_locked_mem(adr, val);
        return;
        case 0xA: case 0xB:
            mbc_upper_write(adr, val);
        return;
        case 0xC:
            ram.rambanks[0][adr - 0xC000] = (mbc.type == 2) ? (val & 0x0F) : val;
        return;
        case 0xD:
            ram.rambank[adr - 0xD000] = (mbc.type == 2) ? (val & 0x0F) : val;
        return;
        case 0xE:
            mem_write_byte(adr - 0x2000, val);
        return;

        case 0xF:
            if(adr < 0xFE00) {
                mem_write_byte(adr - 0x2000, val);
            }
            else if(adr >= 0xFE00 && adr < 0xFEA0) { // Sprite attributes
                if((lcd.stat & 0x03) <= 0x01 || !(lcd.c & 0x80))
                    ram.oam[adr - 0xFE00] = val;
                else
                    write_locked_mem(adr, val);
            }
            else if(adr >= 0xFEA0 && adr < 0xFF00) { // Locked
                write_locked_mem(adr, val);
            }
            else if(adr >= 0xFF00 && adr < 0xFF80) { // IO Registers
                return io_write(adr, val);
            }
            else if(adr >= 0xFF80 && adr < 0xFFFF) { // HiRAM
                ram.hram[adr - 0xFF80] = val;
            }
            else {
                cpu.ie = val & 0x1F;
            }
        return;
    }
    assert(0);
}

void mem_write_word(u16 adr, u16 val) {
    mem_write_byte(adr, val&0x00FF);
    mem_write_byte(adr + 1, (val&0xFF00) >> 8);
}

