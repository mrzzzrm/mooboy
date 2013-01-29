#include "break.h"
#include "debug/debug.h"
#include "debug/utils.h"
#include "cpu/defines.h"
#include <assert.h>

#define BREAK {dbg.console = 1;}

static struct {
    u16 *data;
    unsigned int size;
} bps;

static struct {
    int vblank;
} bss;

static u16 *add_bp(u16 adr) {
    bps.data = realloc(bps.data, ++bps.size * sizeof(*bps.data));
    bps.data[bps.size-1] = adr;
    return &bps.data[bps.size-1];
}

static void on_vblank(u8 iflag) {
    if(bss.vblank) {
        BREAK;
    }
}

void break_init() {
    bps.data = NULL;
    bps.size = 0;

    bss.vblank = 0;

    debug_on_int(IF_VBLANK, on_vblank);
}

void break_before() {

}

void break_after() {
    unsigned p;
    for(p = 0; p < bps.size; p++) {
        if(bps.data[p] == PC) {
            BREAK;
        }
    }
}

void break_update() {

}

void break_cmd(const char *str) {
    const char *end;
    char cmd[256];
    u16 adr;

    end = get_word(str, cmd, sizeof(cmd));


    if(streq(cmd, "vblank")) {
        bss.vblank = get_bool(end, "on", "off", &end);
        assert(bss.vblank >= 0);
        fprintf(stderr, "Breaking on VBLANK %s\n", bss.vblank ? "on" : "off");
    }
    else {
        adr = strtol(cmd, NULL, 16);
        add_bp(adr);
        fprintf(stderr, "New breakpoint @ %.4X\n", adr);
    }
}

