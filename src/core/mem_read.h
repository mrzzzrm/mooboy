#ifndef MEM_READ_H
#define MEM_READ_H

    u8 read_rom(u16 adr);
    u8 read_vram(u16 adr);
    u8 read_xram(u16 adr);
    u8 read_ram(u16 adr);
    u8 read_mirror(u16 adr);

#endif // MEM_READ_H
