#include "sym.h"
#include "utils.h"
#include "cpu.h"
#include "debug.h"
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

static struct {
    field_t **data;
    unsigned int size;
} funcs;

typedef struct callnode_s {
    field_t *func;
    struct callnode_s *parent;

    struct {
        struct callnode_s **data;
        unsigned int size;
    } children;
} callnode_t;


static callnode_t rootcallnode;
static field_t anonfield;
static callnode_t *callnode;
static int recent_call;


static void func_called(field_t *func) {
    unsigned int s;
    callnode_t *funcnode;

    for(s = 0; s < dbg.log_indent*2; s++) {
        fprintf(stderr, " ");
    }
    fprintf(stderr, ">> %s(0x%.4X)\n", func->name, func->adr);

    funcnode = malloc(sizeof(*funcnode));
    funcnode->parent = callnode;
    funcnode->func = func;
    funcnode->children.data = NULL;
    funcnode->children.size = 0;

    callnode->children.data = realloc(callnode->children.data, (++callnode->children.size) * sizeof(*callnode->children.data));
    callnode->children.data[callnode->children.size-1] = funcnode;

    dbg.log_indent++;

    callnode = funcnode;
}

static void append_func(field_t *func) {
    unsigned int s;
    callnode_t *funcnode;
    callnode_t *parentnode;

    parentnode = callnode->parent != NULL ? callnode->parent : callnode;

    for(s = 0; s < dbg.log_indent*2; s++) {
        fprintf(stderr, " ");
    }
    fprintf(stderr, ">> %s(0x%.4X)\n", func->name, func->adr);

    funcnode = malloc(sizeof(*funcnode));
    funcnode->parent = parentnode;
    funcnode->func = func;
    funcnode->children.data = NULL;
    funcnode->children.size = 0;

    parentnode->children.data = realloc(parentnode->children.data, (++parentnode->children.size) * sizeof(*parentnode->children.data));
    parentnode->children.data[parentnode->children.size-1] = funcnode;

    callnode = funcnode;
}

static void anon_func_called(u16 adr) {
    callnode_t *funcnode;

    funcnode = malloc(sizeof(*funcnode));
    funcnode->parent = callnode;
    funcnode->func = &anonfield;
    funcnode->children.data = NULL;
    funcnode->children.size = 0;

    callnode->children.data = realloc(callnode->children.data, (++callnode->children.size) * sizeof(*callnode->children.data));
    callnode->children.data[callnode->children.size-1] = funcnode;

    dbg.log_indent++;

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

static void set_node(callnode_t *node, field_t *field, callnode_t *node) {
    node->func = NULL;
    node->parent = NULL;
    node->children.data = NULL;
    node->children.size = 0;
}

static void init_field(field_t *field) {
    field->bank = 0;
    field->adr = 0xFFFF;
    field->name[0] = '\0';
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
    set_node(&rootcallnode, NULL, NULL, NULL);
    set_field(&anonfield, 0, 0xFFF, "");
    recent_call = 0;
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
    if(!recent_call) {
        unsigned int f;
        for(f = 0; f < funcs.size; f++) {
            field_t *func = funcs.data[f];

            if(func->bank == 0 && func->adr == PC) {
                append_func(func);
                return;
            }
        }
    }

    recent_call = 0;
}

void sym_before() {

}

void sym_after() {

}

void sym_call(u16 adr) {
    recent_call = 1;
    unsigned int f;
    for(f = 0; f < funcs.size; f++) {
        field_t *func = funcs.data[f];

        if(func->bank == 0 && func->adr == PC) {
            func_called(func);
            break;
        }
    }
}

void sym_ret() {
    if(callnode->parent != NULL) {
        callnode = callnode->parent;
        dbg.log_indent--;
    }
}
