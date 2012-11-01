#ifndef MEM_H
#define MEM_H

    #include "util/defines.h"

    typedef struct ram_s {

    } ram_t;

    typedef struct rom_s {
        byte *banks[0x4000];
    } rom_t;


    void mem_reset();
    void mem_load_rom(u8 *data, uint size);

#endif // MEM_H
