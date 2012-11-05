#ifndef MEM_H
#define MEM_H

    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "util/err.h"
    #include "util/defines.h"

    typedef struct ram_s {
        byte ibanks[8][0x1000]; // GB - 2 banks; CGB - 8 banks
        byte (*xbanks)[0x2000]; // Variable number of cartridge-ram
    } ram_t;

    typedef struct rom_s {
        byte (*banks)[0x4000];
    } rom_t;

    typedef struct mbc_s {
        uint romsize;
        uint ramsize;
    } mbc_t;

    extern ram_t ram;
    extern rom_t rom;
    extern mbc_t mbc;

    void mem_init();
    void mem_reset();
    bool mem_load_rom(u8 *data, uint datasize);

    u8 mem_readb(u16 adr);
    void mem_writeb(u16 adr, u8 val);

#endif // MEM_H
