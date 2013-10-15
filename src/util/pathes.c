#include "pathes.h"
#include <string.h>

pathes_t pathes = {NULL};

void pathes_rompath(const char *rompath) {
    pathes.rom = realloc(pathes.rom, strlen(rompath) + 1);

    char *pathend = strrchr(pathes.rom, '/');
    if(pathend == NULL) {
        pathes.romdir = realloc(pathes.romdir, 3);
        strcpy(pathes.romdir, "./");
    }
    else {
        int len = pathend - pathes.rom + 1;
        pathes.romdir = realloc(pathes.romdir, len+1);
        memcpy(pathes.romdir, pathes.rom, len);
        pathes.romdir[len] = '\0';
    }

    int pathlen = strlen(pathes.romdir);

    pathes.config = realloc(pathes.config, pathlen + 7 + 1);
    sprintf(pathes.config, "%s.config", pathes.romdir);

    int s;
    for(s = 0; s < 10; s++) {
        pathes.states[s] = realloc(pathes.states[s], pathlen + 5 + 1);
        sprintf(pathes.config, "%s.sav%i", pathes.romdir, s);
    }

    pathes.card = realloc(pathes.card, pathlen + 5 + 1);
    sprintf(pathes.card, "%s.card", pathes.romdir);
}

void pathes_close() {
    free(pathes.rom);
    free(pathes.romdir);
    free(pathes.config);

    int s;
    for(s = 0; s < 10; s++) {
        free(pathes.states[s]);
    }

    free(pathes.card);
}

