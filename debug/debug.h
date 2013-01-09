#ifndef DEBUG_H
#define DEBUG_H

    #include "util/defines.h"

    typedef struct {
        int verbose;
        int mode;
        int state_lvl;

        int cursor;
    } dbg_t;

    extern dbg_t dbg;

    #define DBG_TRACE 0
    #define DBG_IRQ 1
    #define DBG_CURSOR_EQ 2
    #define DBG_CURSOR_GE 3

    #define DBG_VLVL_NONE 0
    #define DBG_VLVL_PC_ONLY 1
    #define DBG_VLVL_MIN 2
    #define DBG_VLVL_NORMAL 3
    #define DBG_VLVL_MAX 4

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
    void debug_sym_vram(u16 adr, u8 val);
    void debug_sym_xram(u16 adr, u8 val);
    void debug_sym_hram(u16 adr, u8 val);
    void debug_sym_io(u8 r, u8 val);

    /* OPS */
    void debug_op(u8 b);


    /*
        Debugging console:
            r - Run
            c[=;<;>]XXXX - Run to or relative to cursor

            mr=R - Monitor CPU-register(s)
            mm=XXXX-[XXXX] - Monitor memory
            ms=S - Monitor symbol

            d - Dump everything
            dm - Dump memory
            dr - Dump Registers
            dio - Dump io registers
            df - Dump framebuffer
            dv - Dump video

            j - run to next jump (excl jr)
    */


#endif // DEBUG_H
