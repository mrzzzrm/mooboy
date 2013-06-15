#include "sym.h"
#include "utils.h"
#include "cpu.h"
#include "debug.h"
#include "sys/sys.h"
#include "defines.h"
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
    u16 calladr;

    struct {
        struct node_s **data;
        unsigned int size;
    } children;
} node_t;

static struct {
    int anon;
    int call;
    int jp;
    int pc;
} handle;

static node_t rootnode;
static field_t anonfield;
static node_t *currentnode;
static int recent_call;
static void (*func_callback)(const char *name);

static node_t *new_node(node_t *parent, field_t *func, u16 calladr) {
    node_t *node = malloc(sizeof(*node));
    node->parent = parent;
    node->func = func;
    node->children.data = NULL;
    node->children.size = 0;
    node->calladr = calladr;

    return node;
}

static void append_childnode(node_t *parent, node_t *child) {
    parent->children.data = realloc(parent->children.data, (++parent->children.size) * sizeof(*parent->children.data));
    parent->children.data[parent->children.size-1] = child;
}

static void func_entered(field_t *func, u16 from, int call) {
    node_t *funcnode;

    debug_print_line_prefix();
    fprintf(stderr, "%s %s@0x%.4X %s\n", call ? "CALL" : "JUMP", func->name, func->adr, call ? "{" : "");

    funcnode = new_node(currentnode, func, dbg.before.cpu.pc.w);
    append_childnode(currentnode, funcnode);
    currentnode = funcnode;

    if(call)
        dbg.log_indent++;
}

static void func_call(field_t *func, u16 from) {
    if(!handle.call) return;
    func_entered(func, from, 1);
}

static void func_jp(field_t *func, u16 from) {
    if(!handle.jp) return;
    func_entered(func, from, 0);
}

static void append_func(field_t *func) {
    if(!handle.pc) return;
    node_t *funcnode;
    node_t *parentnode;

    parentnode = currentnode->parent != NULL ? currentnode->parent : currentnode;

    debug_print_line_prefix();
    fprintf(stderr, "ENTER %s@0x%.4X\n", func->name, PC);

    funcnode = new_node(parentnode, func, dbg.before.cpu.pc.w);
    append_childnode(parentnode, funcnode);
    currentnode = funcnode;
}

static void anon_func_call(u16 adr, u16 from) {
    if(!handle.call) return;
    if(!handle.anon) return;
    node_t *funcnode;

    debug_print_line_prefix();
    fprintf(stderr, "CALL 0x%.4X {\n", adr);

    funcnode = new_node(currentnode, &anonfield, dbg.before.cpu.pc.w);
    append_childnode(currentnode, funcnode);
    currentnode = funcnode;

    dbg.log_indent++;
}

static field_t *get_func(u16 adr) {
    unsigned int f;
    for(f = 0; f < funcs.size; f++) {
        field_t *func = funcs.data[f];

        if(func->bank == 0 && func->adr == adr) {
            return func;
        }
    }
    return NULL;
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

static void init_node(node_t *currentnode, u16 calladr) {
    currentnode->func = NULL;
    currentnode->parent = NULL;
    currentnode->children.data = NULL;
    currentnode->children.size = 0;
    currentnode->calladr = calladr;
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
    fprintf(stderr, "Loading symbols from: %s\n", path);
    f = fopen(path, "r");
    if(f == NULL) {
        fprintf(stderr, "No such sym-file\n");
        return;
    }

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

static void load_same_sym_file() {
    char *buf = malloc(strlen(sys_get_rompath()) + 3);
    char *dot;
    strcpy(buf, "rom/banks.gb");
    dot = strrchr(buf, '.');
    assert(dot != NULL);
    strcpy(dot+1, "sym\0");
    load_sym_file(buf);
    free(buf);
}

void sym_init() {
    funcs.data = NULL;
    funcs.size = 0;
    currentnode = &rootnode;
    init_node(&rootnode, 0x100);
    init_field(&anonfield);
    recent_call = 0;
    handle.anon = 0;
    handle.call = 1;
    handle.jp = 0;
    handle.pc = 0;
    func_callback = NULL;

    //load_same_sym_file();
}

void sym_cmd(const char *str)  {
    char cmd[256];
    const char *end;

    end = get_word(str, cmd, sizeof(cmd));

    if(streq(cmd, "off")) {
        handle.call = 0;
        handle.jp = 0;
        handle.pc = 0;
    }
    else if(streq(cmd, "on")) {
        handle.call = 1;
        handle.jp = 1;
        handle.pc = 1;
    }
    else if(streq(cmd, "anon")) {
        handle.anon = get_bool(end, "on", "off", &end);
        assert(handle.call >= 0);
    }
    else if(streq(cmd, "load")) {
        end = get_word(end, cmd, sizeof(cmd));
        if(strlen(cmd) != 0) {
            load_sym_file(cmd);
        }
        else {
            load_same_sym_file();
        }
    }
    else if(streq(cmd, "call")) {
        handle.call = get_bool(end, "on", "off", &end);
        assert(handle.call >= 0);
    }
    else if(streq(cmd, "jp")) {
        handle.jp = get_bool(end, "on", "off", &end);
        assert(handle.jp >= 0);
    }
    else if(streq(cmd, "pc")) {
        handle.pc = get_bool(end, "on", "off", &end);
        assert(handle.pc >= 0);
    }
    else {
        fprintf(stderr, "sym: unknown cmd\n");
    }
}

void sym_update() {
    if(!recent_call) {
        field_t *func = get_func(PC);
        if(func != NULL) {
            append_func(func);
        }
    }

    recent_call = 0;
}

void sym_before() {

}

void sym_after() {

}

void sym_jp() {
    field_t *func = get_func(PC);
    if(func != NULL) {
        func_jp(func, dbg.before.cpu.pc.w);
    }
    recent_call = 1;
}

void sym_call(u16 adr, u16 from) {
    field_t *func = get_func(PC);
    if(func != NULL) {
        func_call(func, from);
        if(func_callback != NULL) {
            func_callback(func->name);
        }
    }
    else {
        anon_func_call(adr, from);
    }
    recent_call = 1;
}

void sym_ret() {
    if(PC - 3 != currentnode->calladr)
        return;

    if(currentnode->parent != NULL) {
        node_t *oldnode = currentnode;
        currentnode = currentnode->parent;
        dbg.log_indent--;
        debug_print_line_prefix();
        if(handle.call)
            fprintf(stderr, "} RET %s@%.4X\n", oldnode->func->name, (unsigned)oldnode->func->adr);
    }
    else {
        if(handle.call)
            fprintf(stderr, "WARNING: Potential RET without corresponding CALL detected\n");
    }
}

void sym_on_func(void (*f)(const char *)) {
    func_callback = f;
}

