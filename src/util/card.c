#include "card.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "sys/sys.h"
#include "core/mbc.h"
#include "core/mem.h"
#include "core/rtc.h"
#include "core/moo.h"
#include "util/pathes.h"

static FILE *file;

static char *get_path() {
static void read(void *ptr, size_t size) {
    size_t r = fread(ptr, size, 1, file);
    if(r != 1) {
        moo_errorf("SRAM-file corrupt");
    }
}

static void write(void *ptr, size_t size) {
    size_t written = fwrite(ptr, size, 1, file);
    if(written != 1) {
        moo_errorf("IO error");
    }
}

static void io_ram(void (*io)(void *ptr, size_t size)) {
    if(mbc.has_ram) {
        io(card.srambanks, card.sramsize * sizeof(*card.srambanks));
    }
}

static void io_rtc(void (*io)(void *ptr, size_t size), time_t *timestamp) {
    if(mbc.has_rtc) {
        io(rtc.latched, sizeof(rtc.latched));
        io(rtc.ticking, sizeof(rtc.ticking));
        io(&rtc.mapped, sizeof(rtc.mapped));
        io(&rtc.prelatched, sizeof(rtc.prelatched));
        io(&rtc.cc, sizeof(rtc.cc));
        io(timestamp, sizeof(*timestamp));
    }
}

void card_save() {
    if(!mbc.has_battery || !(mbc.has_ram || mbc.has_rtc)) {
        return;
    }

    printf("Saving card '%s'\n", pathes.card);

    file = fopen(path, "wb");
    free(path);

    file = fopen(pathes.card, "wb");
    if(file == NULL) {
        moo_errorf("Couldn't write to sram file");
        return;
    }

    io_ram(write);

    time_t timestamp = time(NULL);
    io_rtc(write, &timestamp);

    fclose(file);
}

void card_load() {
    if(!mbc.has_battery || !(mbc.has_ram || mbc.has_rtc)) {
        return;
    }

    file = fopen(pathes.card, "rb");
    if(file == NULL) {
        printf("No .card-file found\n");
        return;
    }

    printf("Loading SRAM file '%s'\n", pathes.card);
    io_ram(read);

    time_t card_ts;
    io_rtc(read, &card_ts);

    if(mbc.has_rtc && sys.auto_rtc) {
        time_t now_ts = time(NULL);
        if(now_ts > card_ts) {
            rtc_advance_seconds(now_ts - card_ts);
        }
    }

    u8 dummy;
    if(fread(&dummy, 1, 1, file) != 0) {
        moo_errorf("SRAM file too big");
    }

    fclose(file);
}

