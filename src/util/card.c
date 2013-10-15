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


static void print_time(time_t secs) {
    int y = secs / (3600*24*365) + 1970;
    int m = secs / (3600*24*31) % 12;
    int d = secs / (3600*24) % 31;
    int h = secs / (3600) % 24;
    int mi = secs/60 % 60;
    int s = secs % 60;

    printf("%i-%i-%i %i:%i:%i", y, m, d, h, mi, s);
}

void card_save() {
    FILE *file;
    size_t written;

    if(!mbc.has_battery || !(mbc.has_ram || mbc.has_rtc)) {
        return;
    }

    printf("Saving card '%s'\n", pathes.card);

    file = fopen(pathes.card, "wb");
    if(file == NULL) {
        moo_errorf("Couldn't write to sram file");
        return;
    }

    if(mbc.has_ram) {
        written = fwrite(card.srambanks, 1, card.sramsize * sizeof(*card.srambanks), file);
        if(written != card.sramsize * sizeof(*card.srambanks)) {
            moo_errorf("Error writing to sram file #1");
        }
    }

    if(mbc.has_rtc) {
        time_t timestamp = time(NULL);

        written = fwrite(rtc.latched, 1, sizeof(rtc.latched), file) +
                  fwrite(rtc.ticking, 1, sizeof(rtc.ticking), file) +
                  fwrite(&rtc.mapped, 1, sizeof(rtc.mapped), file) +
                  fwrite(&rtc.prelatched, 1, sizeof(rtc.prelatched), file) +
                  fwrite(&rtc.cc, 1, sizeof(rtc.cc), file) +
                  fwrite(&timestamp, 1, sizeof(timestamp), file);
        printf("Saving RTC at "); print_time(timestamp); printf("\n");
        if(written != sizeof(rtc.latched) + sizeof(rtc.ticking) + sizeof(rtc.mapped) +
                      sizeof(rtc.prelatched) + sizeof(rtc.cc) + sizeof(timestamp)) {
            moo_errorf("Error writing to sram file #2");
        }
    }

    fclose(file);
}

void card_load() {
    FILE *file;
    size_t read;
    u8 dummy;

    if(!mbc.has_battery || !(mbc.has_ram || mbc.has_rtc)) {
        return;
    }

    file = fopen(pathes.card, "rb");
    if(file == NULL) {
        printf("No .card-file found\n");
        return;
    }

    printf("Loading SRAM file '%s'\n", pathes.card);

    if(mbc.has_ram) {
        read = fread(card.srambanks, 1, card.sramsize * sizeof(*card.srambanks), file);
        if(read != card.sramsize * sizeof(*card.srambanks)) {
            moo_errorf("SRAM file corrupt #1");
        }
    }

    if(mbc.has_rtc) {
        time_t card_ts, now_ts;

        read = fread(rtc.latched, 1, sizeof(rtc.latched), file) +
               fread(rtc.ticking, 1, sizeof(rtc.ticking), file) +
               fread(&rtc.mapped, 1, sizeof(rtc.mapped), file) +
               fread(&rtc.prelatched, 1, sizeof(rtc.prelatched), file) +
               fread(&rtc.cc, 1, sizeof(rtc.cc), file) +
               fread(&card_ts, 1, sizeof(card_ts), file);
        if(read != sizeof(rtc.latched) + sizeof(rtc.ticking) + sizeof(rtc.mapped) +
                   sizeof(rtc.prelatched) + sizeof(rtc.cc) + sizeof(card_ts)) {
            moo_errorf("SRAM file corrupt corrupt #2");
        }
        if(sys.auto_rtc) {
            now_ts = time(NULL);
            printf("Saved time is "); print_time(card_ts); printf(", we now have "); print_time(now_ts); printf("\n");
            if(now_ts > card_ts) {
                printf("  Advancing RTC by "); print_time(now_ts - card_ts); printf("\n");
                rtc_advance_seconds(now_ts - card_ts);
            }
        }
    }

    if(fread(&dummy, 1, 1, file) != 0) {
        moo_notifyf("SRAM file corrupt #3");
    }

    fclose(file);
}

