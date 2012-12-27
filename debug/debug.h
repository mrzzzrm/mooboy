#ifndef DEBUG_H
#define DEBUG_H

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
