#include "rom.h"
#include "sys/sys.h"
#include "util/io.h"
#include "core/emu.h"
#include "menu.h"
#include "util.h"
#include <dirent.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    int is_file;
    char name[NAME_MAX + 1];
} direntry_t;

static int finished;
static char cwd[256];
static menu_list_t *list = NULL;
static direntry_t **direntries = NULL;


static void draw() {
    SDL_FillRect(SDL_GetVideoSurface(), NULL, 0);
    menu_draw_list(list);
    SDL_Flip(SDL_GetVideoSurface());
}

static void swap_entries(int e) {
    direntry_t *tmp_e = direntries[e];
    menu_listentry_t *tmp_le = list->entries[e];

    direntries[e] = direntries[e+1];
    direntries[e+1] = tmp_e;

    list->entries[e] = list->entries[e+1];
    list->entries[e+1] = tmp_le;
}

static void sort_entries() {
    int swapped, e;

    do {
        swapped = 0;
        for(e  = 0; e < list->num_entries - 1; e++) {
            if(direntries[e]->is_file && !direntries[e+1]->is_file) {
                swap_entries(e);
                swapped = 1;
            }
            if(!direntries[e]->is_file && direntries[e+1]->is_file) {
                continue;
            }
            if(strcmp(direntries[e]->name, direntries[e+1]->name) > 0) {
                swap_entries(e);
                swapped = 1;
            }
        }
    } while(swapped);
}

static void clear() {
    if(list != NULL) {
        menu_free_list(list);
    }
    list = menu_new_list("Choose ROM");
    if(direntries != NULL) {
        free(direntries);
    }
    direntries = NULL;
}


static void poll_dir() {
    DIR *dir;
    struct dirent *ent;
    int e;

    clear();
    printf("%s\n", cwd);
    dir = opendir(cwd);
    assert(dir);
    for(e = 0; (ent = readdir(dir)) != NULL; e++) {
        direntry_t *direntry;

        if(strcmp(ent->d_name, ".") == 0) {
            continue;
        }

        direntry = malloc(sizeof(*direntry));
        direntry->is_file = ent->d_type == DT_REG;

        if(strcmp(ent->d_name, "..") == 0) {
            sprintf(direntry->name, "%s", ent->d_name);
        }
        else {
            sprintf(direntry->name, "%s%s", ent->d_name, direntry->is_file ? "" : "/");
        }

        menu_new_listentry(list, direntry->name, e);
        direntries = realloc(direntries, sizeof(*direntries) * (list->num_entries));
        direntries[list->num_entries - 1] = direntry;
    }
    sort_entries();
}

static void change_dir(const char *name) {
    if(strcmp(name, "..") == 0) {
        int c;
        for(c = (int)strlen(cwd) - 2; c >=0 && cwd[c] != '/'; c--) {
        }
        if(c <= 0) {
            sprintf(cwd, "/");
        }
        else {
            cwd[c+1] = '\0';
        }
    }
    else {
        strcpy(&cwd[strlen(cwd)], name);
    }
    poll_dir();
}

static void error() {
    printf("Error\n");
    assert(0);
}

static void load_rom(const char *name) {
    u8 *romdata;
    size_t romsize;

    sprintf(sys.rompath, "%s%s", cwd, name);

    printf("Loading ROM: %s\n", sys.rompath);
    if((romdata = io_load_binary(sys.rompath, &romsize)) == NULL) {
        error();
    }
    printf("Firing emu with romdata (size=%i)\n", romsize);
    emu_load_rom(romdata, romsize);
    free(romdata);

    sys.in_menu = 0;
    sys.rom_loaded = 1;
}

static void rom_input_event(int type, int key) {
    if(type == SDL_KEYDOWN) {
        switch(key) {
            case SDLK_UP: menu_list_up(list); break;
            case SDLK_DOWN: menu_list_down(list); break;
            case SDLK_RETURN:
                if(direntries[list->selected]->is_file) {
                    load_rom(direntries[list->selected]->name);
                }
                else {
                    change_dir(direntries[list->selected]->name);
                }
            break;
        }
        if(key == KEY_BACK) {
            finished = 1;
        }
    }
}

void menu_rom_init() {
    int l;

    assert(getcwd(cwd, sizeof(cwd)));
    l = strlen(cwd);
    if(cwd[l - 1] != '/') {
        cwd[l] = '/';
        cwd[l + 1] = '\0';
    }
}

void menu_rom_close() {
    menu_free_list(list);
}

void menu_rom() {
    finished = 0;

    poll_dir();

    while(sys.running && sys.in_menu && !finished) {
        sys_handle_events(rom_input_event);
        draw();
    }
}

