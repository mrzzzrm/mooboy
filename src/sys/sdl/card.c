#include "sys/sys.h"
#include <assert.h>
#include <stdio.h>
#include "core/mbc.h"
#include "core/mem.h"
#include "core/rtc.h"


void sys_save_card() {
    char sramfile[256];
    FILE *file;
    size_t written;

    if(!mbc.has_battery || !(mbc.has_ram || mbc.has_rtc)) {
        return;
    }

    sprintf(sramfile, "%s.card",  sys.rompath);
    file = fopen(sramfile, "w");
    assert(file);

    if(mbc.has_ram) {
        printf("Saving SRAM\n");
        written = fwrite(card.srambanks, 1, card.sramsize * sizeof(*card.srambanks), file);
        assert(written == card.sramsize * sizeof(*card.srambanks));
    }

    if(mbc.has_rtc) {
        printf("Saving RTC\n");
        written = fwrite(rtc.latched,     1, sizeof(rtc.latched), file);    assert (written == sizeof(rtc.latched));
        written = fwrite(rtc.ticking,     1, sizeof(rtc.ticking), file);    assert (written == sizeof(rtc.ticking));
        written = fwrite(&rtc.mapped,     1, sizeof(rtc.mapped), file);     assert (written == sizeof(rtc.mapped));
        written = fwrite(&rtc.prelatched, 1, sizeof(rtc.prelatched), file); assert (written == sizeof(rtc.prelatched));
        written = fwrite(&rtc.cc,         1, sizeof(rtc.cc), file);         assert (written == sizeof(rtc.cc));
    }

    fclose(file);
}

void sys_load_card() {
    char sramfile[256];
    FILE *file;
    size_t read;
    u8 dummy;

    if(!mbc.has_battery || !(mbc.has_ram || mbc.has_rtc)) {
        return;
    }

    sprintf(sramfile, "%s.card",  sys.rompath);
    file = fopen(sramfile, "r");
    if(file == NULL) {
        printf("No SRAM-file found\n");
        return;
    }

    if(mbc.has_ram) {
        printf("Loading SRAM\n");
        read = fread(card.srambanks, 1, card.sramsize * sizeof(*card.srambanks), file);
        assert(read == card.sramsize * sizeof(*card.srambanks));
    }

    if(mbc.has_rtc) {
        printf("Loading RTC\n");
        read = fread(rtc.latched,     1, sizeof(rtc.latched), file);    assert (read == sizeof(rtc.latched));
        read = fread(rtc.ticking,     1, sizeof(rtc.ticking), file);    assert (read == sizeof(rtc.ticking));
        read = fread(&rtc.mapped,     1, sizeof(rtc.mapped), file);     assert (read == sizeof(rtc.mapped));
        read = fread(&rtc.prelatched, 1, sizeof(rtc.prelatched), file); assert (read == sizeof(rtc.prelatched));
        read = fread(&rtc.cc,         1, sizeof(rtc.cc), file);         assert (read == sizeof(rtc.cc));
    }

    fread(&dummy, 1, 1, file);
    assert(feof(file));


    fclose(file);
}
