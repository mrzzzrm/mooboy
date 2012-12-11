#ifndef MEM_MBC_H
#define MEM_MBC_H

    typedef struct mbc_s {
        uint type;

        void (*control_func)(u16, u8);

        uint romsize;
        uint ramsize;

        u8 *rombank;
        u8 *irambank;
        u8 *xrambank;
        u8 *vrambank;
    } mbc_t;

    extern mbc_t mbc;

    void mbc_set_type(u8 type);

    u8 mbc_upper_read(u16 adr);
    void mbc_lower_write(u16 adr, u8 val);
    void mbc_upper_write(u16 adr, u8 val);

#endif
