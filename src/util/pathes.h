#ifndef UTIL_PATHES_H
#define UTIL_PATHES_H

typedef struct {
    char *rom;
    char *romdir;
    char *romname;
    char *config;
    char *states[10];
    char *continue_state;
    char *card;
} pathes_t;

extern pathes_t pathes;

void pathes_rompath(const char *rompath);
void pathes_close();

#endif
