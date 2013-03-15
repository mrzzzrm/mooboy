#include "break.h"
#include "sym.h"
#include "debug.h"
#include "utils.h"
#include "defines.h"
#include <assert.h>
#include <stdio.h>

#define BREAK {dbg.console = 1;}

static struct {
    u16 *data;
    unsigned int size;
} bps;

typedef struct {
    char name[64];
    int on;
} func_t;

static struct {
    int vblank;

    struct {
        int size;
        func_t *data;
    } funcs;
} bss;

static func_t *get_func(const char *name) {
    unsigned int f;
    for(f = 0; f < bss.funcs.size; f++) {
        if(streq(bss.funcs.data[f].name, name))
            return &bss.funcs.data[f];
    }

    func_t *re;
    bss.funcs.data = realloc(bss.funcs.data, ++bss.funcs.size * sizeof(*bss.funcs.data));
    re = &bss.funcs.data[bss.funcs.size-1];

    strcpy(re->name, name);
    re->on = 0;
    return re;
}

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

static void on_func(const char *name) {
    func_t *f = get_func(name);
    if(f->on) {
        BREAK;
    }
}

void break_init() {
    bps.data = NULL;
    bps.size = 0;

    bss.vblank = 0;
    bss.funcs.size = 0;
    bss.funcs.data = NULL;

    debug_on_int(IF_VBLANK, on_vblank);
    sym_on_func(on_func);
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
    else if(streq(cmd, "func")) {
        end = get_word(end, cmd, sizeof(cmd));
        func_t *f = get_func(cmd);
        f->on = get_bool(end, "on", "off", &end);
        assert(f->on >= 0);
        fprintf(stderr, "Breaking on function %s %s\n", f->name, f->on ? "on" : "off");
    }
    else {
        adr = strtol(cmd, NULL, 16);
        add_bp(adr);
        fprintf(stderr, "New breakpoint @ %.4X\n", adr);
    }
}

