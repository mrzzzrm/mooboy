#include "config.h"
#include "sys/sys.h"
#include "core/moo.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

typedef struct { // TODO: Dynamic
    char key[64];
    char val[64];
} pair_t;

static FILE *file;
static pair_t **tuples = NULL;
static int num_tuples;

static void trim(char *str) {
    char *c;
    for(c = str; isalnum(*c); c++) {}
    *c = '\0';
}

static void save(const char *key, const char *val) {
    fprintf(file, "%s=%s\n", key, val);
}

static void save_int(const char *key, int ival) {
    char val[64]; // TODO: Dynamic, or at least synced to pair/pair_t
    snprintf(val, sizeof(val), "%i", ival);
    save(key, val);
}

static const char *load(const char *key) {
    int t;
    for(t = 0; t < num_tuples; t++) {
        if(strcmp(key, tuples[t]->key) == 0) {
            return tuples[t]->val;
        }
    }
    return NULL;
}

static int load_int(const char *key) {
    int val;
    char *end;
    const char *sval = load(key);
    if(sval == NULL) {
        return 0;
    }

    val = strtol(sval, &end, 10);
    if(*end != '\0') {
        return -1;
    }

    return val;
}

static void clear() {
    int t;

    if(tuples != NULL) {
        for(t = 0; t < num_tuples; t++) {
            free(tuples[t]);
        }
        free(tuples);
    }
    num_tuples = 0;
    tuples = NULL;
}

static void parse() {
    char line[256], *val;

    while(fgets(line, sizeof(line), file) != NULL) {
        val = strchr(line, '=');

        if(val != NULL) {
            pair_t *tuple;

            *val = '\0';
            val++;

            tuple = malloc(sizeof(*tuple));

            strncpy(tuple->key, line, sizeof(tuple->key));
            strncpy(tuple->val, val, sizeof(tuple->val));
            trim(tuple->val);

            tuples = realloc(tuples, sizeof(*tuples) * (++num_tuples));
            tuples[num_tuples-1] = tuple;
        }
    }
}

static void config_save(const char *path) {
    printf("Saving config to '%s'\n", path);

    file = fopen(path, "w");
    if(file == NULL) {
        moo_errorf("Couldn't write to .conf file '%s'", path);
        return;
    }

    save_int("sound_on", sys.sound_on);
    save_int("scalingmode", sys.scalingmode);
    save_int("show_statusbar", sys.show_statusbar);
    save_int("auto_continue", sys.auto_continue);
    save_int("auto_rtc", sys.auto_rtc);
    save_int("warned_rtc_sav_conflict", sys.warned_rtc_sav_conflict);

    fclose(file);
}

static int config_load(const char *path) {

    file = fopen(path, "r");
    if(file == NULL) {
        return 0;
    }

    printf("Loading config from '%s'\n", path);

    parse();
    fclose(file);

    sys.sound_on = load_int("sound_on");

    int scalingmode = load_int("scalingmode");
    sys.show_statusbar = load_int("show_statusbar");
    sys.auto_continue = load_int("auto_continue");
    sys.auto_rtc = load_int("auto_rtc");
    sys.warned_rtc_sav_conflict = load_int("warned_rtc_sav_conflict");

    clear();


    if(scalingmode < 0 || scalingmode >= sys.num_scalingmodes) {
        moo_errorf("No such scalingmode %i, please delete config or insert a valid value", scalingmode);
        sys_set_scalingmode(0);
        return 0;
    }
    sys_set_scalingmode(scalingmode);

    return 1;
}

void config_default() {
    sys.sound_on = 1;
    sys_set_scalingmode(sys.scalingmode);
    sys.show_statusbar = 0;
    sys.auto_continue = SYS_AUTO_CONTINUE_ASK;
    sys.auto_rtc = 1;
    sys.warned_rtc_sav_conflict = 0;
}

void config_save_local() {
    char rom_config_path[sizeof(sys.rompath) + 5 + 1];
    sprintf(rom_config_path, "%s.conf", sys.rompath);
    config_save(rom_config_path);
}

int config_load_local() {
    char rom_config_path[sizeof(sys.rompath) + 5 + 1];
    sprintf(rom_config_path, "%s.conf", sys.rompath);
    return config_load(rom_config_path);
}

void config_save_global() {
    config_save("mooboy.conf");
}

int config_load_global() {
    return config_load("mooboy.conf");
}

