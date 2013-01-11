#include "debug.h"
#include "monitor.h"
#include "cpu.h"
#include "mem/io/lcd.h"
#include "cpu/defines.h"
#include "run.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>

dbg_t dbg;

static cpu_t cpu_before, cpu_after;
static ram_t ram_before, ram_after;

#define RUN_FOREVER 0
#define RUN_TRACE 1
#define RUN_UNTIL_CURSOR_EQ 2
#define RUN_UNTIL_CURSOR_GE 3
#define RUN_UNTIL_CURSOR_LE 4
#define RUN_UNTIL_REGISTER 5
#define RUN_UNTIL_MEMORY 6
#define RUN_UNTIL_IO 7
#define RUN_UNTIL_SYMBOL 8
#define RUN_UNTIL_JUMP 9

#define MONITOR_REGISTER 0x02
#define MONITOR_MEMORY_CELL 0x04
#define MONITOR_MEMORY_RANGE 0x08
#define MONITOR_IO 0x10
#define MONITOR_INTERRUPT 0x40
#define MONITOR_OP 0x80

#define BEGEQ(s1, s2) strcmp((s1),(s2)) == 0


u8 mem_before[0xFFFF+1], mem_after[0xFFFF+1];

typedef struct {
    cpu_t cpu;
} trace_node_t;

typedef struct {
    int size;
    trace_node_t **nodes;
} trace_t;

static trace_t trace = {0, NULL};

static int begeq(const char *sstr, const char *lstr) {
    if(strlen(lstr) < strlen(sstr))
        return false;
    return memcmp(sstr, lstr, strlen(sstr)) == 0;
}

static int streq(const char *sstr, const char *lstr) {
    return strcmp(sstr, lstr) == 0;
}

static void snap_mem(u8 *mem) {
    unsigned int i;
    for(i = 0; i <= 0xFFFF; i++) {
        mem[i] = mem_readb(i);
    }
}

static void trace_update() {
    trace.nodes = realloc(trace.nodes, (++trace.size) * sizeof(*trace.nodes));
    trace.nodes[trace.size-1] = malloc(sizeof(**trace.nodes));
    trace.nodes[trace.size-1]->cpu = cpu;
}

static void dump_trace(int lvl) {
    unsigned int n;
    FILE *f;

    f = fopen("trace.txt", "w");
    for(n = 0; n < trace.size; n++) {
        fprintf(f, "PC=%.4X\n", trace.nodes[n]->cpu.pc.w);
    }
    fclose(f);
}

static void print_bits8(u8 v) {
    int b;
    for(b = 7; b >= 0; b--) {
        fprintf(stderr, "%i", (1<<b)&v?1:0);
    }
}

static void cpu_print_diff_b(u8 b, u8 a, const char *name) {
    if(b != a)
        fprintf(stderr, "    %s: %.2X=>%.2X\n", name, b, a);
}

static void cpu_print_diff_w(u16 b, u16 a, const char *name) {
    if(b != a)
        fprintf(stderr, "    %s: %.4X=>%.4X\n", name, b, a);
}

static void cpu_print_diff_flag(u8 b, u8 a, const char *name) {
    if(b != a) {
        fprintf(stderr, "    %s: ", name);
        print_bits8(b);fprintf(stderr, "=>");print_bits8(a);fprintf(stderr, "\n");
    }
}

static void dump_fb(FILE *f, u8 *fb) {
    unsigned int x, y;
    for(y = 0; y < 144; y++) {
        for(x = 0; x < 160; x++) {
            fprintf(f, "%i", fb[y*160 + x]);
        }
        fprintf(f, "\n");
    }
}


static void dump_fbs() {
    FILE *f = fopen("fbs.txt", "w");
    assert(f != NULL);

    fprintf(f, "Clean-FB:\n"); dump_fb(f, lcd.clean_fb);
    fprintf(f, "Working-FB:\n"); dump_fb(f, lcd.working_fb);

    fclose(f);
}

static void dump_oam(FILE *f) {
    unsigned int s, ss;
    for(s = 0, ss = 0; s < 40; s++, ss+=4) {
        fprintf(f, "  Sprite %i Y=%i X=%i Tile=%.2X Attributes=%.2X\n", s, (int)ram.oam[ss], (int)ram.oam[ss+1], (int)ram.oam[ss+2], (int)ram.oam[ss+3]);
    }
}

static void dump_vram(FILE *f) {
    unsigned int x, y;
    for(y = 0; y < 256; y++) {
        fprintf(f, "  ");
        for(x = 0; x < 32; x++) {
            fprintf(f, "%X ", ram.vbanks[0][y*32+x]);
        }
        fprintf(f, "\n");
    }
}

static void dump_video() {
    FILE *f = fopen("video.txt", "w");
    assert(f != NULL);

    fprintf(f, "OAM:\n"); dump_oam(f);
    fprintf(f, "VRAM:\n"); dump_vram(f);

    fclose(f);
}

static void handle_cmd(const char *str) {
    if(begeq("rc", str)) {
        switch(str[2]) {
            case '=': dbg.run.mode = RUN_UNTIL_CURSOR_EQ; break;
            case '>': dbg.run.mode = RUN_UNTIL_CURSOR_GE; break;
            case '<': dbg.run.mode = RUN_UNTIL_CURSOR_LE; break;
        }
        dbg.run.cursor = strtol(&str[3], NULL, 16);
        dbg.console = 0;
    }
    else if(streq("r", str)) {
        dbg.run.mode = RUN_FOREVER;
        dbg.console = 0;
    }
    else if(begeq("mm", str)) {
        char *end;
        dbg.monitor.mem = strtol(&str[3], &end, 16);
        dbg.monitor.mode &= ~(MONITOR_MEMORY_RANGE | MONITOR_MEMORY_CELL);
        if(end[0] == '-') {
            dbg.monitor.from = dbg.monitor.mem;
            dbg.monitor.to = strtol(&end[1], NULL, 16);
            dbg.monitor.mode |= MONITOR_MEMORY_RANGE;
        }
        else {
            dbg.monitor.mode |= MONITOR_MEMORY_CELL;
        }
    }
    else if(begeq("ms", str)) {
        if(streq("dma", &str[3])) {
            dbg.monitor.sym |= MONITOR_SYM_DMA;
        }
        else {
            fprintf(stderr, "Unknown sym\n");
        }
    }
    else if(begeq("mem", str)) {
        u16 mem, from, to, adr;
        char *end;
        from = to = mem = strtol(&str[4], &end, 16);
        if(end[0] == '-') {
            dbg.monitor.from = dbg.monitor.mem;
            dbg.monitor.to = strtol(&end[1], NULL, 16);
        }
        for(adr = from; adr <= to; adr++) {
            fprintf(stderr, "%.4X: %.2X\n", adr, mem_readb(adr));
        }
    }
    else {
        fprintf(stderr, "Unknown cmd\n");
    }
}

static void to_trace() {
    dbg.run.mode = RUN_TRACE;
    dbg.console = 1;
}

void debug_init() {
    dbg.verbose = 1;
    dbg.console = 1;
    dbg.run.mode = RUN_TRACE;
    dbg.monitor.mode = 0x00;
    snap_mem(mem_before);
    snap_mem(mem_after);
}

void debug_console() {
    char str[256];

    do {
        switch(dbg.run.mode) {
            case RUN_UNTIL_CURSOR_EQ: if(PC == dbg.run.cursor) to_trace(); break;
            case RUN_UNTIL_CURSOR_GE: if(PC >= dbg.run.cursor) to_trace(); break;
            case RUN_UNTIL_CURSOR_LE: if(PC <= dbg.run.cursor) to_trace(); break;
        }

        if(dbg.monitor.mode & MONITOR_MEMORY_CELL)
            monitor_cell(dbg.monitor.mem);
        if(dbg.monitor.mode & MONITOR_MEMORY_RANGE)
            monitor_range(dbg.monitor.from, dbg.monitor.to);

        if(dbg.console) {
            fprintf(stderr, "%.4X: ", PC);
            assert(gets(str) != NULL);
            fflush(stdin);

            if(!isalnum(*str))
                break;

            handle_cmd(str);
        }
    } while(dbg.console);
}


void debug_print_cpu_state() {
//    fprintf(stderr, "[");
//    fprintf(stderr, "PC:%.4X AF:%.2X%.2X BC:%.2X%.2X DE:%.2X%.2X HL:%.2X%.2X SP:%.4X F:", (int)PC, (int)A, (int)F, (int)B, (int)C, (int)D, (int)E, (int)H, (int)L, (int)SP);
//    int b;
//    for(b = 7; b >= 4; b--) {
//        fprintf(stderr, "%i", (1<<b)&F?1:0);
//    }
//    if(dbg.state_lvl > 0)
//        fprintf(stderr, " \n LC:%.2X LS:%.2X LY:%.2X CC: %.8X]\n", lcd.c, lcd.stat, lcd.ly, cpu.cc);
//    e  lse
//        fprintf(stderr, "]\n");
}

static void debug_cpu_before() {
}

static void debug_cpu_after() {
    cpu_after = cpu;
}

static void debug_cpu_print_diff() {
    if(cpu_after.pc.w - 1 == cpu_before.pc.w)
        cpu_before.pc = cpu_after.pc;

    cpu_before.cc = cpu_after.cc;
    if(memcmp(&cpu_before, &cpu_after, sizeof(cpu_t)) == 0)
        return;

    fprintf(stderr, "  CPU-Diff \n");
    cpu_print_diff_b(cpu_before.af.b[1], cpu_after.af.b[1], "A");
    cpu_print_diff_flag(cpu_before.af.b[0], cpu_after.af.b[0], "F");
    cpu_print_diff_b(cpu_before.bc.b[1], cpu_after.bc.b[1], "B");
    cpu_print_diff_b(cpu_before.bc.b[0], cpu_after.bc.b[0], "C");
    cpu_print_diff_b(cpu_before.de.b[1], cpu_after.de.b[1], "D");
    cpu_print_diff_b(cpu_before.de.b[0], cpu_after.de.b[0], "E");
    cpu_print_diff_b(cpu_before.hl.b[1], cpu_after.hl.b[1], "H");
    cpu_print_diff_b(cpu_before.hl.b[0], cpu_after.hl.b[0], "L");
    cpu_print_diff_w(cpu_before.pc.w, cpu_after.pc.w, "PC");
}

static void debug_ram_before() {
    ram_before = ram;
}

static void debug_ram_after() {
    ram_after = ram;
}


static void ram_print_i_diff() {
    unsigned int bank, byte;

    for(bank = 0; bank < 8; bank++) {
        for(byte = 0; byte < 0x1000; byte++) {
            if(ram_before.ibanks[bank][byte] != ram_after.ibanks[bank][byte]) {
                fprintf(stderr, "    IRAM[%i][%.4X] %.2X=>%.2X\n", bank, byte, ram_before.ibanks[bank][byte], ram_after.ibanks[bank][byte]);
            }
        }
    }
}

static void ram_print_h_diff() {
    unsigned int byte;

    for(byte = 0; byte < 0x80; byte++) {
        if(ram_before.hram[byte] != ram_after.hram[byte]) {
            fprintf(stderr, "    HRAM[%.2X] %.2X=>%.2X\n", byte, ram_before.hram[byte], ram_after.hram[byte]);
        }
    }
}

static void ram_print_v_diff() {
    unsigned int bank, byte;

    for(bank = 0; bank < 2; bank++) {
        for(byte = 0; byte < 0x2000; byte++) {
            if(ram_before.vbanks[bank][byte] != ram_after.vbanks[bank][byte]) {
                fprintf(stderr, "    VRAM[%i][%.4X] %.2X=>%.2X\n", bank, byte, ram_before.vbanks[bank][byte], ram_after.vbanks[bank][byte]);
            }
        }
    }
}

static void ram_print_oam_diff() {
    unsigned int byte;

    for(byte = 0; byte < 0xA0; byte++) {
        if(ram_before.oam[byte] != ram_after.oam[byte]) {
            fprintf(stderr, "    OAM[%.2X] %.2X=>%.2X\n", byte, ram_before.oam[byte], ram_after.oam[byte]);
        }
    }
}

static void debug_ram_print_diff() {
    if(memcmp(&ram_before, &ram_after, sizeof(ram_t)) == 0)
        return;

    fprintf(stderr, "  RAM-Diff \n");
    ram_print_i_diff();
    ram_print_h_diff();
    ram_print_v_diff();
    ram_print_oam_diff();
}

void debug_before() {
    snap_mem(dbg.before.mem);
    dbg.before.cpu = cpu;
}

void debug_after() {
    snap_mem(dbg.after.mem);
    dbg.after.cpu = cpu;
}

void debug_print_diff() {
    debug_cpu_print_diff();
    debug_ram_print_diff();
}

