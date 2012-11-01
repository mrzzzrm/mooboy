#ifndef MEM_H
#define MEM_H

    #include "util/defines.h"

    typedef struct ram_s {
        byte wram[0x2000];
    } ram_t;

    typedef struct rom_s {
        byte *banks[0x4000];
    } rom_t;

    typedef struct mbc_s {
        uint romsize;
        uint ramsize;
    } mbc_t;

    extern ram_t ram;
    extern rom_t rom;
    extern mbc_t mbc;

    void mem_reset();
    void mem_load_rom(u8 *data, uint size);

#endif // MEM_H
