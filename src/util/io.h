#ifndef IO_H
#define IO_H

    #include <errno.h>
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
    #include "util/defines.h"

    u8 *io_load_binary(const char *path, size_t *size);

#endif // IO_H
