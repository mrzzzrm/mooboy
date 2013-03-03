#include "mem.h"
#include "mem/io.h"
#include "cpu.h"
#include "debug/debug.h"
#include "_assert.h"

ram_t ram;
rom_t rom;

void mem_init() {

}

void mem_reset() {
    memset(ram.ibanks, 0x23, sizeof(ram.ibanks));
    memset(ram.xbanks, 0x23, sizeof(*ram.xbanks));
    memset(ram.hram, 0x23, sizeof(ram.hram));
    memset(ram.vbanks, 0x23, sizeof(ram.vbanks));
    memset(ram.oam, 0x23, sizeof(ram.oam));

    mbc.rombank = rom.banks[1];
    mbc.irambank = ram.ibanks[1];
    mbc.xrambank = ram.xbanks[0];
    mbc.vrambank = ram.vbanks[0];
}

u8 mem_readb(u16 adr) {
    switch(adr >> 12) {
        case 0x0: case 0x1: case 0x2: case 0x3:
            return rom.banks[0][adr];
        break;
        case 0x4: case 0x5: case 0x6: case 0x7:
            return mbc.rombank[adr - 0x4000];
        break;
        case 0x8: case 0x9:
            return mbc.vrambank[adr - 0x8000];
        break;
        case 0xA: case 0xB:
            return mbc_upper_read(adr);
        break;
        case 0xC:
            return ram.ibanks[0][adr - 0xC000];
        break;
        case 0xD:
            return mbc.irambank[adr - 0xD000];
        break;
        case 0xE:
            mem_readb(adr - 0x2000);
        break;

        case 0xF:
            if(adr < 0xFE00) {
                return mem_readb(adr - 0x2000);
            }
            else if(adr >= 0xFE00 && adr < 0xFEA0) { // Sprite attributes
                return ram.oam[adr - 0xFE00];
            }
            else if(adr >= 0xFEA0 && adr < 0xFF00) { // Locked
            }
            else if(adr >= 0xFF00 && adr < 0xFF80) { // IO Registers
                return io_read(adr);
            }
            else if(adr >= 0xFF80 && adr < 0xFFFE) { // HiRAM
                return ram.hram[adr - 0xFF80];
            }
            else {
                return cpu.ie;
            }
        break;

        default:
            assert(0);
    }

    return 0; // ...and avoid warnings
}

u16 mem_readw(u16 adr) {
    return (u16)mem_readb(adr) | ((u16)mem_readb(adr + 1) << 8);
}

void mem_writeb(u16 adr, u8 val) {
    switch(adr >> 12) {
        case 0x0: case 0x1: case 0x2: case 0x3:
        case 0x4: case 0x5: case 0x6: case 0x7:
            mbc_lower_write(adr, val);
        break;
        case 0x8: case 0x9:
            mbc.vrambank[adr - 0x8000] = val;
        break;
        case 0xA: case 0xB:
            mbc_upper_write(adr, val);
        break;
        case 0xC:
            ram.ibanks[0][adr - 0xC000] = (mbc.type == 2) ? (val & 0x0F) : val;
        break;
        case 0xD:
            mbc.irambank[adr - 0xD000] = (mbc.type == 2) ? (val & 0x0F) : val;
        break;
        case 0xE:
            mem_writeb(adr - 0x2000, val);
        break;

        case 0xF:
            if(adr < 0xFE00) {
                mem_writeb(adr - 0x2000, val);
            }
            else if(adr >= 0xFE00 && adr < 0xFEA0) { // Sprite attributes
                ram.oam[adr - 0xFE00] = val;
            }
            else if(adr >= 0xFEA0 && adr < 0xFF00) { // Locked

            }
            else if(adr >= 0xFF00 && adr < 0xFF80) { // IO Registers
                return io_write(adr, val);
            }
            else if(adr >= 0xFF80 && adr < 0xFFFE) { // HiRAM
                ram.hram[adr - 0xFF80] = val;
            }
            else {
                debug_int_ie(val);
                cpu.ie = val;
            }
        break;

        default:
            assert(0);
    }
}

void mem_writew(u16 adr, u16 val) {
    mem_writeb(adr, val&0x00FF);
    mem_writeb(adr + 1, (val&0xFF00) >> 8);
}

