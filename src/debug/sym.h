#ifndef DEBUG_SYM_H
#define DEBUG_SYM_H

    #include "util/defines.h"

    void sym_init();
    void sym_cmd(const char *cmd);
    void sym_update();
    void sym_before();
    void sym_after();
    void sym_jp();
    void sym_call(u16 adr, u16 from);
    void sym_ret();
    void sym_on_func(void (*f)(const char *));

#endif
