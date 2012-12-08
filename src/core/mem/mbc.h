#ifndef MEM_MBC_H
#define MEM_MBC_H



    typedef struct mbc_s {
        uint type;

        uint romsize;
        uint ramsize;

        u8 *rombank;
        u8 *irambank;
        u8 *xrambank;
        u8 *vrambank;
    } mbc_t;

    extern mbc_t mbc;

    void mbc_control(u16 adr, u8 val);

#endif
