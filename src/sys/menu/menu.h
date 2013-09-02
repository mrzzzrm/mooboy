#ifndef SYS_MENU_H
#define SYS_MENU_H

#include <SDL/SDL.h>

#define MENU_CONTINUE     0
#define MENU_NEW_ROM      1
#define MENU_STATE_LOADED 2
#define MENU_RUNNING      3
#define MENU_QUIT         4


typedef struct {
    int action;
} menu_t;

extern menu_t menu;


void menu_init();
void menu_close();
int menu_run();

#endif
