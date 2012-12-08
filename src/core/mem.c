#include "mem.h"
#include "cpu.h"
#include <assert.h>
#include "mem_write.h"
#include "mem_read.h"

#define assert_corrupt(expr, msg) if(!(expr)) {err_set(ERR_ROM_CORRUPT); fprintf(stderr, "%s\n", (msg)); return 0; }
#define assert_illegal_read(expr, msg) if(!(expr)) {err_set(ERR_ILLEGAL_READ); fprintf(stderr, "%s\n", (msg)); return 0; }
#define assert_illegal_write(expr, msg) if(!(expr)) {err_set(ERR_ILLEGAL_WRITE); fprintf(stderr, "%s\n", (msg)); return 0; }

ram_t ram;
rom_t rom;

static inline u8 *mem_direct_map(u16 adr) {
    switch((adr & 0xF000) >> 12) {
        case 0x0: case 0x1: case 0x2: case 0x3: return &rom.banks[0][adr]; break;
        case 0x4: case 0x5: case 0x6: case 0x7: return &mbc.rombank[adr - 0x4000]; break;
        case 0x8: case 0x9: return &mbc.vrambank[adr - 0x8000]; break;
        case 0xA: case 0xB: return &mbc.xrambank[adr - 0xA000]; break;
        case 0xC: return &ram.ibanks[0][adr - 0xC0000]; break;
        case 0xD: return &mbc.irambank[adr - 0xD000]; break;
    }
    assert(0);
}

void mem_init() {

}

void mem_reset() {
    mbc.rombank = rom.banks[0];
    mbc.irambank = ram.ibanks[0];
    mbc.xrambank = ram.xbanks[0];
    mbc.vrambank = ram.vbanks[0];
}

u8 mem_readb(u16 adr) {
    if(adr < 0xE000) { // Common memory accesses
        return *mem_direct_map(adr);
    }
    else if(adr >= 0xE000 && adr < 0xFE00) { // Weird mirror of C000 -> FDFF
        return ram.ibanks[0][adr - 0xC0000];
    }
    else if(adr >= 0xFE00 && adr < 0xFEA0) { // Sprite attributes

    }
    else if(adr >= 0xFEA0 && adr < 0xFF00) { // Locked
        assert_corrupt(0, "Invalid access to locked memory location");
    }
    else if(adr >= 0xFF00 && adr < 0xFF80) { // IO Registers
        return read_io(adr - 0xFF00);
    }
    else if(adr >= 0xFF80 && adr < 0xFFFE) { // HiRAM
        return ram.hram[adr - 0xFF80];
    }
    else { // FFFF
        //return cpu.ime;
    }
    assert(0);
}

u16 mem_readw(u16 adr) {
    return ((u16)mem_readb(adr) << 8) | (u16)mem_readb(adr + 1);
}

void mem_writeb(u16 adr, u8 val) {
    if(adr < 0x8000) {
        mbc_control(adr, val);
    }
    else if(adr >= 0x8000 && adr < 0xE000) { // Common memory accesses
        *mem_direct_map(adr) = val;
    }
    else if(adr >= 0xE000 && adr < 0xFE00) { // Weird mirror of C000 -> FDFF
        ram.ibanks[0][adr - 0xC0000] = val;
    }
    else if(adr >= 0xFE00 && adr < 0xFEA0) { // Sprite attributes

    }
    else if(adr >= 0xFEA0 && adr < 0xFF00) { // Locked
        assert_corrupt(0, "Invalid access to locked memory location");
    }
    else if(adr >= 0xFF00 && adr < 0xFF80) { // IO Registers
        return write_io(adr - 0xFF00, val);
    }
    else if(adr >= 0xFF80 && adr < 0xFFFE) { // HiRAM
        ram.hram[adr - 0xFF80] = val;
    }
    else { // FFFF
        //return cpu.ime;
    }
    assert(0);
}

void mem_writew(u16 adr, u16 val) {
    mem_writeb(adr, (val&0xFF00) >> 8);
    mem_writeb(adr + 1, val&0x00FF);
}

