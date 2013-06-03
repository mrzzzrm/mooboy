#ifndef SYS_H
#define SYS_H

    #include <time.h>
    #include "util/defines.h"

    void sys_init(int argc, const char** argv);
    void sys_close();
    int sys_invoke();
    void sys_fb_ready();

    void sys_save_sram();
    u8 *sys_load_sram(int *size);

    bool sys_running();
    bool sys_new_rom();

    const char *sys_get_rompath();


#endif // SYS_H
