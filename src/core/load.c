#include "load.h"
#include "defines.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include "sys/sys.h"
#include "mem.h"
#include "moo.h"
#include "mbc.h"

#define LOAD_BUFSIZE (1024)

static u8 *load_binary(const char *path, size_t *size) {
    FILE *file;
    size_t r;
    u8 *data;

    *size = 0;

    file = fopen(path, "rb");
    if(file == NULL) {
        printf("Failed to open rompath: %s", strerror(errno));
        return NULL;
    }

    data = NULL;
    do {
        u8 loadbuf[LOAD_BUFSIZE];

        r = fread(loadbuf, sizeof(u8), LOAD_BUFSIZE, file);
        data = realloc(data, *size + r);
        memcpy(data + *size, loadbuf, r);
        *size += r;
    } while(r == LOAD_BUFSIZE);

    fclose(file);

    return data;
}

static u16 rom_bankcount(u8 ref) {
    if(ref <= 8) {
        return 2 << ref;
    }
    else if(ref >= 0x52 && ref <= 0x54) {
        return ((u8[]){72, 80, 96})[ref - 0x52];
    }
    else {
        fprintf(stderr, "ROM-bankcount ref unknown: %.2X\n", ref);
        return 0;
    }
}

static void init_mode(u8 ref) {
    moo.mode = ref & 0x80 ? CGB_MODE : NON_CGB_MODE;
}

static void init_card(u8 ref) {
    u8 ln = ref & 0x0F;
    u8 hn = ref >> 4;

    mbc.has_battery = 0;
    mbc.has_ram = 0;
    mbc.has_rtc = 0;

    if(hn == 0x0) {
        switch(ln) {
            case 0x0: case 0x8: case 0x9:
                mbc_set_type(0);
                mbc.has_battery = ln == 0x9;
                mbc.has_ram = ln & 0x8;
            break;
            case 0x1: case 0x2: case 0x3:
                mbc_set_type(1);
                mbc.has_battery = ln == 0x3;
                mbc.has_ram = ln & 0x2;
            break;
            case 0x5: case 0x6:
                mbc_set_type(2);
                mbc.has_battery = ln == 0x6;
            break;
            break;
            case 0xB: case 0xC: case 0xD:
                assert(0);
            break;
            case 0xF:
                mbc_set_type(3);
                mbc.has_battery = 1;
                mbc.has_rtc = 1;
            break;
        }
    }
    else if(hn == 0x1) {
        switch(ln) {
            case 0x0: case 0x1: case 0x2:  case 0x3:
                mbc_set_type(3);
                mbc.has_battery = ln == 0 || ln == 3;
                mbc.has_rtc = ln == 0;
                mbc.has_ram = ln != 1;
            break;
            case 0x5: case 0x6: case 0x7:
                mbc_set_type(4); // This is weird, as there is no such MBC...
            break;
            case 0x9: case 0xA: case 0xB: // Alas, no way to emulate rumbling...
            case 0xC: case 0xD: case 0xE:
                mbc_set_type(5);
                mbc.has_battery = ln == 0xB || ln == 0xE;
                mbc.has_ram = ln != 0x9 && ln != 0xC;
            break;
            default:
                assert(0);
        }
    }
    else {
        assert(0);
    }

    printf("Full cardridge type: %.2X\n", ref);
}

static void init_rom(u8 ref, u8 *rom, u32 romsize) {
    card.romsize = rom_bankcount(ref);
        assert(card.romsize != 0);
        assert(card.romsize * 0x4000 == romsize);
        assert(romsize <  sizeof(card.rombanks));
    memcpy(card.rombanks, rom, romsize);

    fprintf(stderr, "ROM-size set to %d banks, ref = %.2X\n", card.romsize, ref);
}

static void init_sram(u8 ref) {
    assert(ref <= 0x03);

    card.sramsize = (u8[]){1, 1, 1, 4}[ref];
    fprintf(stderr, "Cardridge-RAM set to %d banks by ref %i \n", card.sramsize, ref);
}

void load_rom() {
    size_t romsize;

    moo.state &= ~MOO_ROM_LOADED_BIT;

    u8 *rom = load_binary(sys.rompath, &romsize);
    assert(romsize > 0x014F);

    init_mode(rom[0x0143]);
    init_card(rom[0x0147]);
    init_rom(rom[0x0148], rom, romsize);
    init_sram(rom[0x0149]);

    sys_load_card();

    mbc.rombank = card.rombanks[1];
    mbc.srambank = card.srambanks[0];

    free(rom);

    moo.state |= MOO_ROM_LOADED_BIT;
    moo_begin();
}


