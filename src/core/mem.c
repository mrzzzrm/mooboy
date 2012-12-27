#include "mem.h"
#include "mem/io.h"
#include "cpu.h"
#include "_assert.h"

ram_t ram;
rom_t rom;

void mem_init() {

}

void mem_reset() {
    mbc.rombank = rom.banks[1];
    mbc.irambank = ram.ibanks[1];
    mbc.xrambank = ram.xbanks[0];
    mbc.vrambank = ram.vbanks[0];
}

u8 mem_readb(u16 adr) {
    //fprintf(stderr, "  Reading ");
    switch(adr>>12) {
        case 0x0: case 0x1: case 0x2: case 0x3: //fprintf(stderr, "ROM 0 [%X]=%X\n", adr, rom.banks[0][adr]);
                                                return rom.banks[0][adr];           break;
        case 0x4: case 0x5: case 0x6: case 0x7: //fprintf(stderr, "ROM X [%X]=%X\n", adr - 0x4000, mbc.rombank[adr - 0x4000]);
                                                return mbc.rombank[adr - 0x4000];   break;
        case 0x8: case 0x9:                     //fprintf(stderr, "VRAM [%X]=%X\n", adr - 0x8000, mbc.vrambank[adr - 0x8000]);
                                                return mbc.vrambank[adr - 0x8000];  break;
        case 0xA: case 0xB:                     //fprintf(stderr, "MBC [%X]=%X\n", adr, mbc_upper_read(adr));
                                                return mbc_upper_read(adr);         break;
        case 0xC:                               //fprintf(stderr, "RAM 0 [%X]=%X\n", adr - 0xC000, ram.ibanks[0][adr - 0xC000]);
                                                return ram.ibanks[0][adr - 0xC000]; break;
        case 0xD:                               //fprintf(stderr, "RAM X [%X]=%X\n", adr - 0xD000, mbc.irambank[adr - 0xD000]);
                                                return mbc.irambank[adr - 0xD000];  break;
        case 0xE:                               mem_readb(adr - 0x2000);  break;

        case 0xF:
            if(adr < 0xFE00) {
                return mem_readb(adr - 0x2000);
            }
            else if(adr >= 0xFE00 && adr < 0xFEA0) { // Sprite attributes
                //fprintf(stderr, "OAM\n");
                return ram.oam[adr - 0xFE00];
            }
            else if(adr >= 0xFEA0 && adr < 0xFF00) { // Locked
                assert_corrupt(0, "Invalid access to locked memory location\n");
            }
            else if(adr >= 0xFF00 && adr < 0xFF80) { // IO Registers
                //fprintf(stderr, "IO");
                return io_read(adr);
            }
            else if(adr >= 0xFF80 && adr < 0xFFFE) { // HiRAM
                //fprintf(stderr, "HiRAM\n");
                return ram.hram[adr - 0xFF80];
            }
            else {
                //fprintf(stderr, "IME\n");
                return cpu.ime;
            }
        break;

        default:
            assert(0);
            //fprintf(stderr, "Nothing?!?");
    }
    ////fprintf(stderr, "  [Done]\n");

    return 0; // ...and avoid warnings
}

u16 mem_readw(u16 adr) {
    return (u16)mem_readb(adr) | ((u16)mem_readb(adr + 1) << 8);
}

void mem_writeb(u16 adr, u8 val) {
    //fprintf(stderr, "  Writing ");

    // Mirroring

    switch(adr>>12) {
        case 0x0: case 0x1: case 0x2: case 0x3:
        case 0x4: case 0x5: case 0x6: case 0x7: //fprintf(stderr, "MBC L\n");
                                                return mbc_lower_write(adr, val);              break;
        case 0x8: case 0x9:                     //fprintf(stderr, "VRAM\n");
                                                mbc.vrambank[adr - 0x8000] = val;       break;
        case 0xA: case 0xB:                     //fprintf(stderr, "MBC U\n");
                                                mbc_upper_write(adr, val);              break;
        case 0xC:                               //fprintf(stderr, "RAM 0\n");
                                                ram.ibanks[0][adr - 0xC000] = (mbc.type == 2) ? (val & 0x0F) : val;  break;
        case 0xD:                               //fprintf(stderr, "RAM X\n");
                                                mbc.irambank[adr - 0xC000] = (mbc.type == 2) ? (val & 0x0F) : val;  break;
        case 0xE:                               mem_writeb(adr - 0x2000, val); break;

        case 0xF:
            if(adr < 0xFE00) {
                mem_writeb(adr - 0x2000, val);
            }
            else if(adr >= 0xFE00 && adr < 0xFEA0) { // Sprite attributes
                //fprintf(stderr, "OAM\n");
                ram.oam[adr - 0xFE00] = val;
            }
            else if(adr >= 0xFEA0 && adr < 0xFF00) { // Locked
                assert_corrupt(0, "Invalid access to locked memory location");
            }
            else if(adr >= 0xFF00 && adr < 0xFF80) { // IO Registers
                //fprintf(stderr, "IO\n");
                return io_write(adr, val);
            }
            else if(adr >= 0xFF80 && adr < 0xFFFE) { // HiRAM
                //fprintf(stderr, "HiRAM\n");
                ram.hram[adr - 0xFF80] = val;
            }
            else {
                //fprintf(stderr, "IME\n");
                cpu.ime = val;
            }
        break;

        default:
            assert(0);
            //fprintf(stderr, "Nothing?!?\n");
    }

    //fprintf(stderr, " [Done]\n");
}

void mem_writew(u16 adr, u16 val) {
    mem_writeb(adr, (val&0xFF00) >> 8);
    mem_writeb(adr + 1, val&0x00FF);
}

