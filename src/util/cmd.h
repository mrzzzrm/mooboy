#ifndef CMD_H
#define CMD_H

    #include <stdlib.h>

    #include "defines.h"

    void cmd_init(int argc, const char **argv);
    const char *cmd_get(const char *param);
    bool cmd_isset(const char *param) ;

#endif
