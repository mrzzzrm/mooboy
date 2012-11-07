#include "mem.h"

#include "mem_write.h"
#include "mem_read.h"

#define assert_corrupt(expr, msg) if(!(expr)) {err_set(ERR_ROM_CORRUPT); fprintf(stderr, "%s\n", (msg)); return 0; }
#define assert_illegal_read(expr, msg) if(!(expr)) {err_set(ERR_ILLEGAL_READ); fprintf(stderr, "%s\n", (msg)); return 0; }
#define assert_illegal_write(expr, msg) if(!(expr)) {err_set(ERR_ILLEGAL_WRITE); fprintf(stderr, "%s\n", (msg)); return 0; }

ram_t ram;
rom_t rom;
mbc_t mbc;



static u8 rom_bankcount(u8 ref) {
    if(ref <= 6) {
        return ((u8[]){2, 4, 8, 16, 32, 64, 128})[ref];
    }
    else if(ref >= 0x52 && ref <= 0x54) {
        return ((u8[]){72, 80, 96})[ref - 0x52];
    }
    else {
        return 0;
    }
}

void mem_init() {
    ram.xbanks = NULL;
    rom.banks = NULL;
}

void mem_reset() {

}

bool mem_load_rom(u8 *data, uint datasize) {
    assert_corrupt(datasize > 0x014F, "ROM too small for header");

    mbc.romsize = rom_bankcount(data[0x0148]);
    assert_corrupt(mbc.romsize != 0, "Unknown romsize ref");
    assert_corrupt(mbc.romsize * 0x4000 == datasize, "Datasize doesn't match rom-internally specified size");
    rom.banks = realloc(rom.banks, mbc.romsize * sizeof(*rom.banks));
    memcpy(rom.banks, data, datasize);

    return true;
}

//static u8 (*readf_map[])(u16) = {
//    read_rom, read_rom, read_rom, read_rom,
//    read_rom, read_rom, read_rom, read_rom,
//    read_vram, read_vram, read_xram, read_xram,
//    read_ram
//};



u8 mem_readb(u16 adr) {
    if(adr < 0xE000) {
       // return readf_map[adr >> 12](adr);
    }
    else if(adr >= 0xE000 && adr < 0xFE00) { // Weird mirror of C000 -> FDFF
       // return read_mirror(adr);
    }
    else if(adr >= 0xFE00 && adr < 0xFEA0) { // Sprite attributes

    }
    else if(adr >= 0xFEA0 && adr < 0xFF00) { // Locked

    }
    else if(adr >= 0xFF00 && adr < 0xFF80) { // IO Registers

    }
    else if(adr >= 0xFF80 && adr < 0xFFFE) { // HiRAM

    }
    else { // FFFF

    }
}

void mem_writeb(u16 adr, u8 val) {

}

