#ifndef SYS_H
#define SYS_H

    #include <util/defines.h>

    void sys_init(int argc, const char** argv);
    void sys_close();

    bool sys_running();
    bool sys_new_rom();

    const char *sys_get_rompath();

    void sys_error();

#endif // SYS_H
