#ifndef UTIL_PATH_H
#define UTIL_PATH_H

typedef struct {
    char *rom;
    char *config;
    char *states[10];
    char *continue_state;
    char *ram;
} pathes_t;

extern pathes_t pathes;

void path_new_rom(const char *path);
void path_close();

#endif
