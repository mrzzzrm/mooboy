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

typedef struct {
    char *name;
    int *ptr;
    int default_val;
} config_value_t;

static config_value_t values[] = {
    {"sound_on", &sys.sound_on, 1},
    {"scalingmode", &sys.scalingmode, 0},
    {"show_statusbar", &sys.show_statusbar, 0},
    {"auto_continue", &sys.auto_continue, SYS_AUTO_CONTINUE_ASK},
    {"auto_rtc", &sys.auto_rtc, 1},
    {"warned_rtc_sav_conflict", &sys.warned_rtc_sav_conflict, 0}
};

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

    int v;
    for(v = 0; v < sizeof(values)/sizeof(*values); v++) {
        save_int(values[v].name, *values[v].ptr);
    }

    fclose(file);
}

static int config_load(const char *path) {
    printf("Loading config from '%s'\n", path);

    file = fopen(path, "r");
    if(file == NULL) {
        return 0;
    }

    parse();
    fclose(file);

    int v;
    for(v = 0; v < sizeof(values)/sizeof(*values); v++) {
         *values[v].ptr = load_int(values[v].name);
    }

    clear();

    if(sys.scalingmode < 0 || sys.scalingmode >= sys.num_scalingmodes) {
        moo_errorf("No such scalingmode %i, please delete config or insert a valid value", sys.scalingmode);
        sys_set_scalingmode(0);
        return 0;
    }
    sys_set_scalingmode(sys.scalingmode);

    return 1;
}

void config_default() {
    int v;
    for(v = 0; v < sizeof(values)/sizeof(*values); v++) {
         *values[v].ptr = values[v].default_val;
    }
    sys_set_scalingmode(sys.scalingmode);
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

