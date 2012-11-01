#include "cmd.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "string.h"


int argc = 0;
const char **argv = NULL;

void cmd_init(const int _argc, const char **_argv) {
   argc = _argc;
   argv = _argv;
}

const char *cmd_get(const char *param) {
    int c;
    for(c = 1; c < argc; c++) {
        if(!strcmp(param, argv[c])) {
            if(c+1 < argc) {
                return argv[c+1];
            }
            else {
                break;
            }
        }
    }

    return NULL;
}

bool cmd_isset(const char *param) {
    int c;
    for(c = 1; c < argc; c++) {
        if(!strcmp(param, argv[c])) {
            return 1;
        }
    }

    return 0;
}
