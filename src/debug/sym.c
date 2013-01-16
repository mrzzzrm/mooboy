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

typedef struct node_s {
    field_t *func;
    struct node_s *parent;

    struct {
        struct node_s **data;
        unsigned int size;
    } children;
} node_t;


static node_t rootnode;
static field_t anonfield;
static node_t *currentnode;
static int recent_call;

static node_t *new_node(node_t *parent, field_t *func) {
    node_t *node = malloc(sizeof(*node));
    node->parent = parent;
    node->func = func;
    node->children.data = NULL;
    node->children.size = 0;

    return node;
}

static void append_childnode(node_t *parent, node_t *child) {
    parent->children.data = realloc(parent->children.data, (++parent->children.size) * sizeof(*parent->children.data));
    parent->children.data[parent->children.size-1] = child;
}

static void func_called(field_t *func, u16 from) {
    unsigned int s;
    node_t *funcnode;

    for(s = 0; s < dbg.log_indent*2; s++) {
        fprintf(stderr, " ");
    }
    fprintf(stderr, "%.4X >> %s@0x%.4X\n", from, func->name, func->adr);

    funcnode = new_node(currentnode, func);
    append_childnode(currentnode, funcnode);
    currentnode = funcnode;

    dbg.log_indent++;
}

static void append_func(field_t *func) {
    node_t *funcnode;
    node_t *parentnode;

    parentnode = currentnode->parent != NULL ? currentnode->parent : currentnode;

    debug_indent();
    fprintf(stderr, "%.4X >> %s@0x%.4X\n", PC, func->name, PC);

    funcnode = new_node(parentnode, func);
    append_childnode(parentnode, funcnode);
    currentnode = funcnode;
}

static void anon_func_called(u16 adr, u16 from) {
    node_t *funcnode;

    debug_indent();
    fprintf(stderr, ">> CALL 0x%.4X from %.4X\n", adr, from);

    funcnode = new_node(currentnode, &anonfield);
    append_childnode(currentnode, funcnode);
    currentnode = funcnode;

    dbg.log_indent++;
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

static void init_node(node_t *currentnode) {
    currentnode->func = NULL;
    currentnode->parent = NULL;
    currentnode->children.data = NULL;
    currentnode->children.size = 0;
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
    currentnode = &rootnode;
    init_node(&rootnode);
    init_field(&anonfield);
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

void sym_call(u16 adr, u16 from) {
    recent_call = 1;
    unsigned int f;
    for(f = 0; f < funcs.size; f++) {
        field_t *func = funcs.data[f];

        if(func->bank == 0 && func->adr == adr) {
            func_called(func, from);
            break;
        }
    }
    if(f == funcs.size) {
        anon_func_called(adr, from);
    }
}

void sym_ret() {
    if(currentnode->parent != NULL) {
        currentnode = currentnode->parent;
        dbg.log_indent--;
    }
}

