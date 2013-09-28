#include "continue.h"
#include "last_rom.h"
#include "state.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

char *continue_state_path() {
    char *rom_path = last_rom_path();
    if(rom_path == NULL) {
        return NULL;
    }

    char *ext = ".continue.sav";
    int state_path_len = strlen(rom_path) + strlen(ext);
    char *state_path = malloc(state_path_len + 1);
    sprintf(state_path, "%s%s", rom_path, ext);

    return state_path;
}

int continue_state_exists() {
    char *state_path = continue_state_path();

    if(state_path == NULL) {
        return 0;
    }

    int re = access(state_path, R_OK) == 0;
    free(state_path);

    return re;
}

void continue_state_load() {
    char *state_path = continue_state_path();
    state_load(state_path);
    free(state_path);
}
