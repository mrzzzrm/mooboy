#ifndef DEBUG_INT_H
#define DEBUG_INT_H

    #include "util/defines.h"
    #include "cpu/defines.h"

    void int_init();
    void int_update();
    void int_before();
    void int_after();
    void int_cmd(const char *cmd);
    void int_exec(u8 flag);
    void int_ie(u8 flag);
    void int_ime(u8 flag);
    void int_req(u8 flag);

#endif
