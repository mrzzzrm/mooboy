#include "config.h"
#include "sys/sys.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

typedef struct {
    char key[64];
    char val[64];
} tuple_t;

static FILE *file;
static tuple_t **tuples = NULL;
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
    char val[64];
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
            tuple_t *tuple;

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
    file = fopen(path, "w");
    assert(file != NULL);

    save_int("sound_on", sys.sound_on);

    fclose(file);
}

void config_load(const char *path) {
    file = fopen(path, "r");
    if(file == NULL) {
        return;
    }

    parse();

    sys.sound_on = load_int("sound_on");

    fclose(file);
    clear();
}

