#ifndef DEBUG_H
#define DEBUG_H

    #include "cpu.h"
    #include "util/defines.h"

    typedef struct {
        int verbose;
        int console;

        struct {
            int mode;
            u32 cursor;
            u32 from, to;
            u32 reg;
            u32 sym;
        } run;

        struct {
            int mode;
            u32 mem;
            u32 from, to;
            u32 reg;
            u32 sym;
            u32 ops;
            u32 io;
        } monitor;

        struct {
            cpu_t cpu;
            u8 mem[0xFFFF+1];
        } after, before;

    } dbg_t;

    extern dbg_t dbg;

    void debug_init();

    void debug_console();

    void debug_before();
    void debug_after();

    void debug_print_diff();
    void debug_print_cpu_state();

    /* Symbols */
    void debug_sym_dma(u8 hn);
    void debug_sym_irq(u8 f);
    void debug_sym_ie(u8 f);
    void debug_sym_jmp(u16 adr);
    void debug_sym_ram(u16 adr, u8 val);
    void debug_sym_io(u8 r, u8 val);

    /* OPS */
    void debug_op(u8 b);


    /*
        Debugging console:
            Run until
            r - forever
            rt - Run tracing
            rc[=;<;>]XXXX - Run to or relative to cursor
            rr - cpu register changes
            rm - memory (range) changes
            rio - io reg changes
            rs - symbol is called
            rj - run to next jump

            Monitor (log)
            [!]m - Do not monitor
            mr=R - CPU-register(s)
            mm=XXXX-[XXXX] - memory
            mio=XX - io register
            ms=S - symbol
            mi=I - interrupts
            mop[=op] - opcodes

            Dump
            d - everything
            dm - memory
            dr - CPU Registers
            dio - io registers
            df - framebuffer
            dv - video

            v[=0;1;pc] Additional Verbosing on/off/pc only

    */


#endif // DEBUG_H
