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
    sprintf(val, "%i", ival);
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

void config_save(const char *path) {
    printf("Saving config to '%s'\n", path);

    file = fopen(path, "w");
    if(file == NULL) {
        moo_errorf("Couldn't write to .conf file '%s'", path);
        return;
    }

    save_int("sound_on", sys.sound_on);
    save_int("scalingmode", sys.scalingmode);
    save_int("show_statusbar", sys.scalingmode);

    fclose(file);
}

int config_load(const char *path) {

    file = fopen(path, "r");
    if(file == NULL) {
        return 0;
    }

    printf("Loading config from '%s'\n", path);

    parse();

    sys.sound_on = load_int("sound_on");

    sys.scalingmode = load_int("scalingmode");
    if(sys.scalingmode < 0 || sys.scalingmode >= sys.num_scalingmodes) {
        moo_errorf("No such scalingmode %i, please delete config or insert a valid value", sys.scalingmode);
        sys.scalingmode = 0;
        return 0;
    }

    fclose(file);
    clear();

    return 1;
}


void config_default() {
    sys.sound_on = 1;
    sys.scalingmode = 0;
    sys.show_statusbar = 0;
}

