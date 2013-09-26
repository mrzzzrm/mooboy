#include "card.h"
#include <assert.h>
#include <stdio.h>
#include "sys/sys.h"
#include "core/mbc.h"
#include "core/mem.h"
#include "core/rtc.h"
#include "core/moo.h"

char *get_sramfile() {
    char *sramfile;

    sramfile = malloc(strlen(sys.rompath) + strlen(".card" + 1));
    sprintf(sramfile, "%s.card",  sys.rompath);

    return sramfile;
}

void card_save() {
    char *sramfile;
    FILE *file;
    size_t written;

    if(!mbc.has_battery || !(mbc.has_ram || mbc.has_rtc)) {
        return;
    }

    sramfile = get_sramfile();
    printf("Saving card '%s'\n", sramfile);

    file = fopen(sramfile, "wb");
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
        written = fwrite(rtc.latched,     1, sizeof(rtc.latched), file);    if (written != sizeof(rtc.latched)) moo_errorf("Error writing to sram file #2");
        written = fwrite(rtc.ticking,     1, sizeof(rtc.ticking), file);    if (written != sizeof(rtc.ticking)) moo_errorf("Error writing to sram file #3");
        written = fwrite(&rtc.mapped,     1, sizeof(rtc.mapped), file);     if (written != sizeof(rtc.mapped)) moo_errorf("Error writing to sram file #4");
        written = fwrite(&rtc.prelatched, 1, sizeof(rtc.prelatched), file); if (written != sizeof(rtc.prelatched)) moo_errorf("Error writing to sram file #5");
        written = fwrite(&rtc.cc,         1, sizeof(rtc.cc), file);         if (written != sizeof(rtc.cc)) moo_errorf("Error writing to sram file #6");
    }

    free(sramfile);
    fclose(file);
}

void card_load() {
    char *sramfile;
    FILE *file;
    size_t read;
    u8 dummy;

    if(!mbc.has_battery || !(mbc.has_ram || mbc.has_rtc)) {
        return;
    }

    sramfile = get_sramfile();

    file = fopen(sramfile, "rb");
    if(file == NULL) {
        printf("No .card-file found\n");
        return;
    }

    printf("Loading card '%s'\n", sramfile);

    if(mbc.has_ram) {
        read = fread(card.srambanks, 1, card.sramsize * sizeof(*card.srambanks), file);
        if(read != card.sramsize * sizeof(*card.srambanks)) {
            moo_errorf("Card corrupt #1");
        }
    }

    if(mbc.has_rtc) {
        read = fread(rtc.latched,     1, sizeof(rtc.latched), file);    if(read != sizeof(rtc.latched)) moo_notifyf("Card corrupt #2");
        read = fread(rtc.ticking,     1, sizeof(rtc.ticking), file);    if(read != sizeof(rtc.ticking)) moo_notifyf("Card corrupt #3");
        read = fread(&rtc.mapped,     1, sizeof(rtc.mapped), file);     if(read != sizeof(rtc.mapped)) moo_notifyf("Card corrupt #4");
        read = fread(&rtc.prelatched, 1, sizeof(rtc.prelatched), file); if(read != sizeof(rtc.prelatched)) moo_notifyf("Card corrupt #5");
        read = fread(&rtc.cc,         1, sizeof(rtc.cc), file);         if(read != sizeof(rtc.cc)) moo_notifyf("Card corrupt #6");
    }

    if(fread(&dummy, 1, 1, file) != 0) {
        moo_notifyf("Card corrupt #7");
    }

    fclose(file);
}

