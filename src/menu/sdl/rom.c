#include "rom.h"
#include "menu/menu.h"
#include "sys/sys.h"
#include "core/moo.h"
#include "util/config.h"
#include "util.h"
#include <dirent.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

typedef struct {
    int is_file;
    char name[64];
} direntry_t;

typedef struct selected_element_s {
    char *path;
    char *selected;
    struct selected_element_s *next;
} selected_element_t;

static int finished;
static char cwd[512] = "";
static menu_list_t *list = NULL;
static direntry_t **direntries = NULL;
static selected_element_t *selected_elements = NULL;

static int poll_dir();


static void save_selected_element() {
    selected_element_t *e;

    if(list->selected < 0) {
        return;
    }
    for(e = selected_elements; e != NULL; e = e->next) {
        if(strcmp(e->path, cwd) == 0) {
            free(e->selected);
            e->selected = strdup(direntries[list->selected]->name);
            return;
        }
        if(e->next == NULL) {
            break;
        }
    }
    e = malloc(sizeof(*e));
    e->path = strdup(cwd);
    e->selected = strdup(direntries[list->selected]->name);
    e->next = selected_elements;
    selected_elements = e;
}

static void load_selected_element() {
    selected_element_t *e;
    int d;
    char *selected = NULL;

    for(e = selected_elements; e != NULL; e = e->next) {
        if(strcmp(e->path, cwd) == 0) {
            selected = e->selected;
            break;
        }
    }

    if(selected != NULL) {
        for(d = 0; d < list->num_entries; d++) {
            if(strcmp(selected, direntries[d]->name) == 0) {
                list->selected = d;
                return;
            }
        }
    }
}

static void to_base_dir() {
    int l;

    if(!getcwd(cwd, sizeof(cwd))) {
        moo_errorf("Couldn't fetch CWD");
        return;
    }

    l = strlen(cwd);
    if(cwd[l - 1] != '/') {
        cwd[l] = '/';
        cwd[l + 1] = '\0';
    }
}

static int is_romfile(const char *path) {
    const char *dot = strrchr(path, '.');
    if(dot != NULL) {
        return strcasecmp(dot + 1, "gb") == 0 || strcasecmp(dot + 1, "gbc") == 0;
    }
    return 0;
}

static void back() {
    finished = 1;
}

static void save_dir() {
    FILE *file = fopen("romdir.txt", "w");
    fprintf(file, "%s", cwd);
    fclose(file);
}

static int load_dir() {
    FILE *file = fopen("romdir.txt", "r");
    if(file == NULL) {
        return 0;
    }
    fgets(cwd, sizeof(cwd), file);
    if(!feof(file)) {
        fclose(file);
        return 0;
    }
    fclose(file);
    DIR *dir = opendir(cwd);
    if(dir == NULL) {
        return 0;
    }
    closedir(dir);
    return 1;
}

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
            if(strcasecmp(direntries[e]->name, direntries[e+1]->name) > 0) {
                swap_entries(e);
                swapped = 1;
            }
        }
    } while(swapped);
}

static void clear() {
    if(direntries != NULL) {
        int e;
        for(e = 0; e < list->num_entries; e++) {
            free(direntries[e]);
        }
        free(direntries);
    }
    direntries = NULL;
    menu_clear_list(list);
}

static void load_rom() {
    char new_rompath[sizeof(sys.rompath)];

    snprintf(new_rompath, sizeof(new_rompath), "%s%s", cwd, direntries[list->selected]->name);
    moo_load_rom(new_rompath);
    moo_begin();
    finished = 1;
}

static void change_dir() {
    save_selected_element();

    strcpy(&cwd[strlen(cwd)], direntries[list->selected]->name);
    save_dir();
    poll_dir();
}

static void parent_dir() {
    int c;
    save_selected_element();
    for(c = (int)strlen(cwd) - 2; c >=0 && cwd[c] != '/'; c--) {
    }
    if(c <= 0) {
        sprintf(cwd, "/");
    }
    else {
        cwd[c+1] = '\0';
    }
    save_dir();
    if(!poll_dir()) {
        to_base_dir();
    }
}

static int poll_dir() {
    DIR *dir;
    struct dirent *ent;
    int e;

    clear();

    dir = opendir(cwd);
    if(dir == NULL) {
        return 0;
    }

    for(e = 0; (ent = readdir(dir)) != NULL;) {
        direntry_t *direntry;
        if(strcmp(ent->d_name, ".") == 0) {
            continue;
        }

        direntry = malloc(sizeof(*direntry));
        direntry->is_file = ent->d_type == DT_REG;

        if(strcmp(ent->d_name, "..") == 0) {
            snprintf(direntry->name, sizeof(direntry->name), "%s", ent->d_name);
            menu_new_listentry_button(list, direntry->name, e, parent_dir);
        }
        else {
            if(direntry->is_file) {
                if(is_romfile(ent->d_name)) {
                    strncpy(direntry->name, ent->d_name, sizeof(direntry->name));
                    menu_new_listentry_button(list, direntry->name, e, load_rom);
                }
                else {
                    continue;
                }
            }
            else {
                if(ent->d_name[0] != '.') {
                    snprintf(direntry->name, sizeof(direntry->name), "%s/", ent->d_name);
                    menu_new_listentry_button(list, direntry->name, e, change_dir);
                }
                else {
                    continue;
                }
            }
        }
        direntries = realloc(direntries, sizeof(*direntries) * (list->num_entries));
        direntries[list->num_entries - 1] = direntry;
        e++;
    }

    closedir(dir);
    sort_entries();

    load_selected_element();

    return 1;
}

static int select_beginning_with_from(menu_list_t *list, int from, char first_char) {
    int e;
    for(e = from; e < list->num_entries; e++) {
        if(direntries[e]->name[0] == first_char) {
            menu_list_select(list, e);
            return 1;
        }
    }
    return 0;
}

static void select_beginning_with(char first_char) {
    if(!select_beginning_with_from(list, list->selected + 1, first_char)) {
        select_beginning_with_from(list, 0, first_char);
    }
}

static void rom_input_event(int type, int key) {
    menu_list_input(list, type, key);

    if(type == SDL_KEYDOWN) {
        switch(key) {
            case SDLK_RIGHT:
                if(!direntries[list->selected]->is_file && strcmp(direntries[list->selected]->name, "..")) {
                    change_dir();
                }
            break;
            case SDLK_LEFT:
                parent_dir();
            break;
        }
        if(isprint(key)) {
            select_beginning_with(key);
        }
    }
}

void menu_rom_init() {
    if(!load_dir()) {
        to_base_dir();
    }

    list = menu_new_list("Choose ROM");
    list->back_func = back;
}

void menu_rom_close() {
    selected_element_t *e;

    menu_free_list(list);

    for(e = selected_elements; e != NULL; e = e->next) {
        free(e->path);
        free(e->selected);
    }
}

void menu_rom() {
    finished = 0;

    poll_dir();

    while(!finished && (moo.state & MOO_RUNNING_BIT)) {
        draw();
        sys_handle_events(rom_input_event);
        menu_list_update(list);
    }
}

