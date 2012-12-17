#ifndef MEM_H
#define MEM_H

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "util/err.h"
    #include "util/defines.h"
    #include "mem/mbc.h"

    typedef struct ram_s {
        u8 ibanks[8][0x1000]; // Gameboy internal; GB - 2 banks; CGB - 8 banks
        u8 (*xbanks)[0x2000]; // Variable number of cartridge-ram
        u8 hram[0x80];
        u8 vbanks[2][0x2000]; // 2nd bank for GBC
        u8 oam[0xA0];
    } ram_t;

    typedef struct rom_s {
        u8 (*banks)[0x4000];
    } rom_t;

    extern ram_t ram;
    extern rom_t rom;


    void mem_init();
    void mem_reset();

    u8 mem_readb(u16 adr);
    u16 mem_readw(u16 adr);
    void mem_writeb(u16 adr, u8 val);
    void mem_writew(u16 adr, u16 val);

#endif // MEM_H
