#ifndef DEBUG_H
#define DEBUG_H

    typedef struct {
        int verbose;
        int mode;
        int state_lvl;

        int cursor;
    } dbg_t;

    extern dbg_t dbg;

    #define DBG_TRACE 0
    #define DBG_IRQ 1
    #define DBG_CURSOR 2

    #define DBG_VLVL_NONE 0
    #define DBG_VLVL_PC_ONLY 1
    #define DBG_VLVL_MIN 2
    #define DBG_VLVL_NORMAL 3
    #define DBG_VLVL_MAX 4

    void debug_init();

    void debug_console();

    void debug_print_cpu_state();

    void debug_before();
    void debug_after();
    void debug_print_diff();

    void debug_cpu_before();
    void debug_cpu_after();
    void debug_cpu_print_diff();

    void debug_ram_before();
    void debug_ram_after();
    void debug_ram_print_diff();

#endif // DEBUG_H
