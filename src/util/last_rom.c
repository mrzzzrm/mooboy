#include "last_rom.h"
#include "core/moo.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

char *last_rom_path() {
    FILE *f = fopen("lastrom.txt", "r");
    if(f == NULL) {
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long pathlen = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *path = malloc(pathlen + 1);
    fread(path, 1, pathlen, f);
    path[pathlen] = '\0';

    fclose(f);

    return path;
}

int last_rom_exists() {
    char *path = last_rom_path();
    int re = access(path, R_OK) == 0;
    free(path);
    return re;
}

void load_last_rom() {
    char *path = last_rom_path();
    moo_load_rom(path);
    free(path);
}
