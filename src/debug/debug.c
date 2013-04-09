#define DEBUG_DEBUG_C

#include "debug.h"
#include "monitor.h"
#include "utils.h"
#include "dump.h"
#include "disasm.h"
#include "cpu.h"
#include "lcd.h"
#include "defines.h"
#include "run.h"
#include "mon.h"
#include "sym.h"
#include "break.h"
#include "int.h"
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
#define RUN_CYCLES 10

#define MONITOR_REGISTER 0x02
#define MONITOR_MEMORY_CELL 0x04
#define MONITOR_MEMORY_RANGE 0x08
#define MONITOR_IO 0x10
#define MONITOR_INTERRUPT 0x40
#define MONITOR_OP 0x80

#define BEGEQ(s1, s2) strcmp((s1),(s2)) == 0


u8 mem_before[0xFFFF+1], mem_after[0xFFFF+1];

static struct {
    char op[18]; int op_set;
    char opl[18]; int opl_set;
    char opr[18]; int opr_set;
} ctrace;

static void (*on_vblank)(u8 iflag);
static void (*on_lcdstat)(u8 iflag);


static void snap_mem(u8 *mem) {
    if(dbg.monitor.mode == MONITOR_MEMORY_CELL) {
        mem[dbg.monitor.mem] = mem_read_byte(dbg.monitor.mem);
    }
    else {
        unsigned int i;
        for(i = dbg.monitor.from; i <= dbg.monitor.to; i++) {
            mem[i] = mem_read_byte(i);
        }
    }
}

static void trace_before() {
    dbg.trace.data = realloc(dbg.trace.data, (++dbg.trace.size) * sizeof(*dbg.trace.data));
    memset(&ctrace, 0x00, sizeof(ctrace));
}

static void trace_after() {
    char *str = dbg.trace.data[dbg.trace.size-1];
    if(ctrace.opr_set)
        sprintf(str, "%s %s, %s", ctrace.op, ctrace.opl, ctrace.opr);
    else if(ctrace.opl_set)
        sprintf(str, "%s %s", ctrace.op, ctrace.opl);
    else
        sprintf(str, "%s", ctrace.op);

}

static void dump_trace() {
    unsigned int n;
    FILE *f;

    f = fopen("trace.txt", "w");
    for(n = 0; n < dbg.trace.size; n++) {
        fprintf(f, "%s\n", dbg.trace.data[n]);
    }
    fclose(f);
}

static void print_bits8(u8 v) {
    int b;
    for(b = 7; b >= 0; b--) {
        fprintf(stderr, "%i", (1<<b)&v?1:0);
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
            fprintf(f, "%X ", ram.vrambanks[0][y*32+x]);
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

static void dump_disasm() {
    unsigned int i;

    FILE *f = fopen("disasm.txt", "w");
    assert(f != NULL);

    for(i = 0; i <= 0xFFFF; i++) {
        fprintf(f, "%.4X %s\n", i, disasm(i));
    }

    fclose(f);
}

static void print_state() {
    fprintf(stderr, "[");
    fprintf(stderr, "PC:%.4X AF:%.2X%.2X BC:%.2X%.2X DE:%.2X%.2X HL:%.2X%.2X SP:%.4X F:", (int)PC, (int)A, (int)F, (int)B, (int)C, (int)D, (int)E, (int)H, (int)L, (int)SP);
    unsigned int b;
    for(b = 7; b >= 4; b--) {
        fprintf(stderr, "%i", (1<<b)&F?1:0);
    }
    fprintf(stderr, " \n LC:%.2X LS:%.2X LY:%.2X CC: %.8X]\n", lcd.c, lcd.stat, lcd.ly, cpu.cc);
}


static void handle_cmd(const char *str) {
    char cmd[256];
    const char *end;

    end = get_word(str, cmd, sizeof(cmd));

    if(streq("sym", cmd)) {
        sym_cmd(end); return;
    }
    else if(streq("int", cmd)) {
        int_cmd(end); return;
    }
    else if(streq("mon", cmd)) {
        mon_cmd(end); return;
    }
    else if(streq("break", cmd)) {
        break_cmd(end); return;
    }
    else if(streq("dump", cmd)) {
        dump_cmd(end); return;
    }



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
    else if(begeq("r", str)) {
        dbg.run.mode = RUN_CYCLES;
        dbg.console = 0;
        dbg.run.cc_end = cpu.cc + strtol(&str[2], NULL, 10);
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
            from = mem;
            to = strtol(&end[1], NULL, 16);
        }
        for(adr = from; adr <= to; adr++) {
            fprintf(stderr, "%.4X: %.2X\n", adr, mem_read_byte(adr));
        }
    }
    else if(begeq("dasm", str)) {
        dump_disasm();
    }
    else if(begeq("s", str)) {
        print_state();
    }
    else if(begeq("s", str)) {
        print_state();
    }
    else if(begeq("d", str)) {
        if(begeq("t", &str[1])) {
            dump_trace();
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
    dbg.log_indent = 0;
    dbg.run.mode = RUN_TRACE;
    dbg.monitor.mode = 0x00;
    on_vblank = NULL;
    snap_mem(mem_before);
    snap_mem(mem_after);
    sym_init();
    int_init();
    mon_init();
    break_init();
}

void debug_update() {
    char str[256];

    do {
        switch(dbg.run.mode) {
            case RUN_CYCLES: if(cpu.cc >= dbg.run.cc_end) to_trace(); break;
            case RUN_UNTIL_CURSOR_EQ: if(PC == dbg.run.cursor) to_trace(); break;
            case RUN_UNTIL_CURSOR_GE: if(PC >= dbg.run.cursor) to_trace(); break;
            case RUN_UNTIL_CURSOR_LE: if(PC <= dbg.run.cursor) to_trace(); break;
        }

        if(dbg.console) {
            fprintf(stderr, "%.4X: ", PC);
            assert(gets(str) != NULL);
            fflush(stdin);

            if(!isalnum(*str))
                break;

            handle_cmd(str);
        }
    } while(dbg.console);

    sym_update();
    int_update();
    mon_update();
    break_update();
}

void debug_before() {
    trace_before();
    snap_mem(dbg.before.mem);
    dbg.before.cpu = cpu;
    sym_before();
    int_before();
    mon_before();
    break_before();
}

void debug_after() {
    trace_after();
    if(dbg.console) {
        fprintf(stderr, "  %s\n", dbg.trace.data[dbg.trace.size-1]);
    }

    snap_mem(dbg.after.mem);
    dbg.after.cpu = cpu;
    sym_after();
    int_after();
    mon_after();
    break_after();
}

void debug_print_line_prefix() {
    unsigned int s;
    fprintf(stderr, "%.4X: ", dbg.before.cpu.pc.w);
    for(s = 0; s < dbg.log_indent*2; s++) {
        fprintf(stderr, " ");
    }
}

void debug_jp() {
    sym_jp();
}

void debug_call(u16 adr) {
    sym_call(adr, dbg.before.cpu.pc.w);
}

void debug_ret() {
    sym_ret();
}

void debug_int_exec(u8 flag) {


    switch(flag) {
        case IF_VBLANK: if(on_vblank != NULL) on_vblank(flag); break;
        case IF_LCDSTAT: if(on_lcdstat != NULL) on_lcdstat(flag); break;
    }
}

void debug_int_ie(u8 flag) {
    int_ie(flag);
}

void debug_int_ime(u8 flag) {
    int_ime(flag);
}

void debug_int_req(u8 flag) {
    int_req(flag);
}

void debug_trace_op(const char *name) {
    strcpy(ctrace.op, name);
}

static void debug_trace_opx(char *str, void *ptr, int len, int mem) {
    char buf[sizeof(str)];
    if(len == 1) {
             if(ptr == &A) sprintf(buf, "A");
        else if(ptr == &B) sprintf(buf, "B");
        else if(ptr == &C) sprintf(buf, "C");
        else if(ptr == &D) sprintf(buf, "D");
        else if(ptr == &E) sprintf(buf, "E");
        else if(ptr == &H) sprintf(buf, "H");
        else if(ptr == &L) sprintf(buf, "L");
        else if(ptr == &F) sprintf(buf, "F");
        else               sprintf(buf, "%.2X", *(u8*)ptr);
    }
    else {
             if(ptr == &AF) sprintf(buf, "AF");
        else if(ptr == &BC) sprintf(buf, "BC");
        else if(ptr == &DE) sprintf(buf, "DE");
        else if(ptr == &HL) sprintf(buf, "HL");
        else if(ptr == &SP) sprintf(buf, "SP");
        else                sprintf(buf, "%.4X", *(u16*)ptr);
    }

    if(mem)
        sprintf(str, "(%s)", buf);
    else
        sprintf(str, "%s", buf);
}

void debug_trace_opl(void *ptr, int len, int mem) {
//    if(ctrace.opl_set)
//        return;

    debug_trace_opx(ctrace.opl, ptr, len, mem);
    ctrace.opl_set = 1;
}

void debug_trace_opr(void *ptr, int len, int mem)  {
//    if(ctrace.opr_set)
//        return;

    debug_trace_opx(ctrace.opr, ptr, len, mem);
    ctrace.opr_set = 1;
}

void debug_trace_opl_data(int d) {
    if(ctrace.opl_set)
        return;

    sprintf(ctrace.opl, "%.4X", d);
    ctrace.opl_set = 1;
}

void debug_trace_opr_data(int d) {
    if(ctrace.opr_set)
        return;

    sprintf(ctrace.opr, "%.4X", d);
    ctrace.opr_set = 1;
}


void debug_on_int(u8 iflag, void (*func)(u8 iflag)) {
    switch(iflag) {
        case IF_VBLANK: on_vblank = func; break;
        case IF_LCDSTAT: on_lcdstat = func; break;
    }
}

