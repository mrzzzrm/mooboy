#include "sym.h"
#include "utils.h"
#include "cpu.h"
#include "cpu/defines.h"
#include "util/defines.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int bank;
    u16 adr;
    char name[256];
} field_t;

struct {
    field_t **data;
    unsigned int size;
} funcs;

typedef struct callnode_s {
    field_t *func;
    struct callnode_s *parent;
    unsigned int depth;

    struct {
        struct callnode_s **data;
        unsigned int size;
    } children;
} callnode_t;


callnode_t rootcallnode;
callnode_t *callnode;

static void func_called(field_t *func) {
    unsigned int s;
    callnode_t *funcnode;

    for(s = 0; s < callnode->depth*2; s++) {
        printf(" ");
    }
    fprintf(stderr, ">>%s\n", func->name);

    funcnode = malloc(sizeof(*funcnode));
    funcnode->parent = callnode;
    funcnode->depth = callnode->depth+1;
    funcnode->func = func;
    funcnode->children.data = NULL;
    funcnode->children.size = 0;

    callnode->children.data = realloc(callnode->children.data, (++callnode->children.size) * sizeof(*callnode->children.data));
    callnode->children.data[callnode->children.size-1] = funcnode;

    callnode = funcnode;
}

static field_t *read_field(const char *str) {
    field_t *field;
    char *end;

    field = malloc(sizeof(*field));
    field->bank = strtol(str, &end, 10);
    field->adr = strtol(&end[1], &end, 16);
    get_word(&end[1], field->name, sizeof(field->name));


    return field;
}

static void load_sym_file(const char *path) {
    FILE *f;
    int in_func_area;
    char line[256];

    if(funcs.data != NULL)
        free(funcs.data);
    funcs.data = NULL;
    funcs.size = 0;

    in_func_area = 0;
    f = fopen(path, "r"); assert(f != NULL);

    while (fgets(line, sizeof(line), f) != NULL) {
        if(begeq("; Area: _CODE", line) ||
           begeq("; Area: _HEADER", line) ||
           begeq("; Area: _HOME", line) ||
           begeq("; Area: _BSS", line)
           ) {
            in_func_area = 1;
           }
        else if(begeq(line, "; Area:")) {
            in_func_area = 0;
        }

        if(in_func_area) {
            field_t *func = read_field(line);
            funcs.data = realloc(funcs.data, (++funcs.size) * sizeof(*funcs.data));
            funcs.data[funcs.size - 1] = func;
        }
    }

    fprintf(stderr, "sym: Found %i functions fields\n", funcs.size);

    fclose(f);
}

void sym_init() {
    funcs.data = NULL;
    funcs.size = 0;
    callnode = &rootcallnode;
    rootcallnode.func = 0;
    rootcallnode.depth = 0;
    rootcallnode.parent = NULL;
    rootcallnode.parent = NULL;
    rootcallnode.children.data = NULL;
    rootcallnode.children.size = 0;
}

void sym_cmd(const char *str)  {
    char cmd[256];
    const char *end;

    end = get_word(str, cmd, sizeof(cmd));

    if(streq(cmd, "load")) {
        end = get_word(end, cmd, sizeof(cmd));
        load_sym_file(cmd);
    }
    else {
        fprintf(stderr, "sym: unknown cmd\n");
    }
}

void sym_update() {
    unsigned int f;
    for(f = 0; f < funcs.size; f++) {
        field_t *func = funcs.data[f];

        if(func->bank == 0 && func->adr == PC) {
            func_called(func);
        }
    }
}

void sym_before() {

}

void sym_after() {

}

