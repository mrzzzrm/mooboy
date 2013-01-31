#include "dump.h"
#include <stdio.h>
#include "utils.h"
#include "mem/io/lcd.h"

static void dump_fb() {
    unsigned int x,y;
    for(y = 0; y < 144; y++) {
        for(x = 0; x < 160; x++) {
            if(lcd.clean_fb[y*160+x] > 0)
                fprintf(stderr, "%.1X", lcd.clean_fb[y*160+x]);
            else
                fprintf(stderr, " ");
        }
        fprintf(stderr, "\n");
    }
}


void dump_cmd(const char *str) {
    char cmd[256];
    const char *end;

    end = get_word(str, cmd, sizeof(cmd));

    if(streq(cmd, "fb")) {
        dump_fb();
    }
}

