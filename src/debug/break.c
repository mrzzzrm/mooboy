#include "break.h"
#include "debug/debug.h"
#include "debug/utils.h"
#include "cpu/defines.h"

static struct {
    u16 *data;
    unsigned int size;
} bps;

static u16 *add_bp(u16 adr) {
    bps.data = realloc(bps.data, ++bps.size * sizeof(*bps.data));
    bps.data[bps.size-1] = adr;
    return &bps.data[bps.size-1];
}

void break_init() {
    bps.data = NULL;
    bps.size = 0;
}

void break_before() {

}

void break_after() {
    unsigned p;
    for(p = 0; p < bps.size; p++) {
        if(bps.data[p] == PC) {
            dbg.console = 1;
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

    adr = strtol(cmd, NULL, 16);
    add_bp(adr);
    fprintf(stderr, "New breakpoint @ %.4X\n", adr);
}

