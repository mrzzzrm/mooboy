#include "mon.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "utils.h"
#include "util/defines.h"

u8 before[0x10000];
u8 after[0x10000];

typedef struct {
    int on;
    u16 start, end;
} range;

static struct {
    range *data;
    unsigned int size;
} ranges;

#define VRAM ranges.data[0]


static range *add_range(u16 start, u16 end) {
    ranges.data = realloc(ranges.data, sizeof(*ranges.data) * ++ranges.size);
    range *r = &ranges.data[ranges.size-1];

    r->on = 1;
    r->start = start;
    r->end = end;

    return &ranges.data[ranges.size-1];
}

static void update_range(range *r, u8 *buf) {
    unsigned int a;
    for(a = r->start; a <= r->end; a++) {
        buf[a] = mem_readb(a);
    }
}

void mon_init() {
    ranges.data = NULL;
    ranges.size = 0;

    range *vram = add_range(0x8000, 0x9FFF);
        vram->on = 0;
}

void mon_cmd(const char *str) {
    char cmd[256];
    const char *end;

    end = get_word(str, cmd, sizeof(cmd));
    if(end == str + strlen(str)) {
        fprintf(stderr, "Unknown arg 1\n");
        return;
    }

    if(streq("vram", cmd)) {
        VRAM.on = get_bool(end, "on", "off", &end);
        assert(VRAM.on >= 0);
        fprintf(stderr, "VRAM monitoring %s\n", VRAM.on ? "on" : "off");
    }
    else  {
        const char *sfrom = cmd;
        char sto[256];
        u16 from, to;

        end = get_word(end, sto, sizeof(sto));

        from = strtol(sfrom, NULL, 16);
        to = strtol(sto, NULL, 16);

        range *r = add_range(from, to);

        fprintf(stderr, "Monitoring range from %.4X -> %.4X\n", from, to);
    }
}

void mon_update() {
    unsigned int i;
    unsigned int a;
    u16 min, max;
    int mon[0x10000];

    min = 0xFFFF;
    max = 0;
    memset(mon, 0x00, sizeof(mon));
    for(i = 0; i < ranges.size; i++) {
        range *r = &ranges.data[i];
        if(r->on) {
            if(r->start < min) min = r->start;
            if(r->end > max) max = r->end;
            memset(&mon[r->start], 0xFF, (r->end - r->start + 1)*sizeof(*mon));
        }
    }

    for(a = min; a <= max; a++) {
        if(mon[a]) {
            if(before[a] != after[a]) {
                debug_print_line_prefix();
                fprintf(stderr, "%.4X %.2X=>%.2X\n", a, before[a], after[a]);
            }
        }
    }
}

void mon_before() {
    unsigned int i;
    for(i = 0; i < ranges.size; i++) {
        range *r = &ranges.data[i];
        update_range(r, before);
    }
}

void mon_after() {
    unsigned int i;
    for(i = 0; i < ranges.size; i++) {
        range *r = &ranges.data[i];
        update_range(r, after);
    }
}



