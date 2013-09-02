#include "io.h"

#define LOAD_BUFSIZE (1024)

u8 *io_load_binary(const char *path, size_t *size) {
    FILE *file;
    size_t r;
    u8 *data;

    *size = 0;

    file = fopen(path, "rb");
    if(file == NULL) {
        printf("Failed to open rompath: %s", strerror(errno));
        return NULL;
    }

    data = NULL;
    do {
        u8 loadbuf[LOAD_BUFSIZE];

        r = fread(loadbuf, sizeof(u8), LOAD_BUFSIZE, file);
        data = realloc(data, *size + r);
        memcpy(data + *size, loadbuf, r);
        *size += r;
    } while(r == LOAD_BUFSIZE);

    fclose(file);

    return data;
}
