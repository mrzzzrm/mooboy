#include "pathes.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

pathes_t pathes = {NULL};

void pathes_rompath(const char *rompath) {
    pathes.rom = realloc(pathes.rom, strlen(rompath) + 1);
    strcpy(pathes.rom, rompath);

    char *pathend = strrchr(pathes.rom, '/');
    if(pathend == NULL) {
        pathes.romdir = realloc(pathes.romdir, 3);
        pathes.romname = realloc(pathes.romname, strlen(pathes.rom) + 1);
        strcpy(pathes.romdir, "./");
        strcpy(pathes.romname, pathes.rom);
    }
    else {
        int sep = pathend - pathes.rom + 1;
        pathes.romdir = realloc(pathes.romdir, sep + 1);
        memcpy(pathes.romdir, pathes.rom, sep);
        pathes.romdir[sep] = '\0';

        pathes.romname = realloc(pathes.romname, strlen(pathes.rom) - sep + 1);
        strcpy(pathes.romname, &pathes.rom[sep]);
    }

    int pathlen = strlen(pathes.romname);

    pathes.config = realloc(pathes.config, pathlen + 7 + 1);
    sprintf(pathes.config, "%s.config", pathes.romname);

    pathes.continue_state = realloc(pathes.continue_state, pathlen + 15 + 1);
    sprintf(pathes.continue_state, "%s.continue.state", pathes.romname);

    int s;
    for(s = 0; s < 10; s++) {
        pathes.states[s] = realloc(pathes.states[s], pathlen + 5 + 1);
        sprintf(pathes.states[s], "%s.sav%i", pathes.romname, s);
    }

    pathes.card = realloc(pathes.card, pathlen + 5 + 1);
    sprintf(pathes.card, "%s.card", pathes.romname);
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

