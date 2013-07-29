#include "util.h"
#include <SDL/SDL_ttf.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define FONT_SIZE 40

static TTF_Font *font;

void menu_util_init() {
    if(!TTF_WasInit()) {
        assert(!TTF_Init());
        font = TTF_OpenFont("data/basic5.ttf", FONT_SIZE);
        assert(font != NULL);
    }
}

void menu_util_close() {
    TTF_CloseFont(font);
}

menu_label_t *menu_label(const char *text) {
    menu_label_t *label;
    SDL_Color unselected   = {200, 150, 150}, selected = {255, 200, 200};

    label = malloc(sizeof(*label));
    label->surfaces[0] = TTF_RenderText_Blended(font, text, unselected);
    label->surfaces[1] = TTF_RenderText_Blended(font, text, selected);

    return label;
}

void menu_free_label(menu_label_t *label) {
    SDL_FreeSurface(label->surfaces[0]);
    SDL_FreeSurface(label->surfaces[1]);
}

void menu_blit(SDL_Surface *s, int x, int y) {
    SDL_Rect r = {x, y, s->w, s->h};
    SDL_BlitSurface(s, NULL, SDL_GetVideoSurface(), &r);
}


menu_list_t *menu_new_list(const char *title) {
    menu_list_t *list;

    list = malloc(sizeof(*list));

    list->entries = NULL;
    list->num_entries = 0;
    list->title = menu_label(title);
    list->selected = 0;
    list->num_visible = 1;
    list->first_visible = 0;

    return list;
}

void menu_free_list(menu_list_t *list) {

}

void menu_new_listentry(menu_list_t *list, const char *name, int id) {
    menu_listentry_t *entry;

    entry = malloc(sizeof(*entry));

    entry->text = menu_label(name);
    entry->val = NULL;
    entry->id = id;
    entry->is_visible = 1;

    list->entries = realloc(list->entries, sizeof(*list->entries) * (++list->num_entries));
    list->entries[list->num_entries-1] = entry;
}

int menu_listentry_index(menu_list_t *list, int key) {
    int e;
    for(e = 0; e < list->num_entries; e++) {
        if(list->entries[e]->id == key) {
            return e;
        }
    }
    assert(0);
}

void menu_listentry_val(menu_list_t *list, int key, const char *val) {
    menu_label_t **label = &list->entries[menu_listentry_index(list, key)]->val;

    if(*label != NULL) {
        menu_free_label(*label);
    }
    *label = menu_label(val);
}


void menu_listentry_val_int(menu_list_t *list, int key, int ival) {
    char val[64];
    sprintf(val, "%i\n", ival);
    menu_listentry_val(list, key, val);
}

int menu_list_selected_id(menu_list_t *list) {
    return list->entries[list->selected]->id;
}

void menu_list_select_first(menu_list_t *list) {
    int e;
    for(e = 0; e < list->num_entries; e++) {
        if(list->entries[e]->is_visible) {
            list->selected = e;
            return;
        }
    }
}

void menu_list_set_visible(menu_list_t *list, int id, int visible) {
    int e;
    for(e = 0; e < list->num_entries; e++) {
        if(list->entries[e]->id == id) {
            list->entries[e]->is_visible = visible;
            return;
        }
    }
}

void menu_draw_list(menu_list_t *list) {
    int e, l;
    int last_visible;
    int top_margin, bottom_margin, left_margin, right_margin;
    int line_height;
    SDL_Surface *screen;

    top_margin = 60;
    bottom_margin = 20;
    left_margin = 20;
    right_margin = 20;
    line_height = 45;
    screen = SDL_GetVideoSurface();

    list->num_visible = 8;

    menu_blit(list->title->surfaces[0], 0, 0);

    last_visible = list->first_visible + list->num_visible - 1;
    last_visible = min(last_visible, list->num_entries - 1);
    for(e = list->first_visible, l = 0; e <= last_visible; e++) {
        if(list->entries[e]->is_visible) {
            menu_blit(list->entries[e]->text->surfaces[list->selected == e ? 1 : 0],
                      left_margin, top_margin + l * line_height);

            if(list->entries[e]->val != NULL) {
                SDL_Surface *val = list->entries[e]->val->surfaces[list->selected == e ? 1 : 0];
                menu_blit(val, screen->w - right_margin - val->w, top_margin + l * line_height);
            }
            l++;
        }
    }
}

void menu_list_up(menu_list_t *list) {
    int new_selected = list->selected;
    while(new_selected > 0) {
        new_selected--;
        if(list->entries[new_selected]->is_visible) {
            list->selected = new_selected;
            break;
        }
    }

    if(list->selected < list->first_visible) {
        list->first_visible = list->selected;
    }
}

void menu_list_down(menu_list_t *list) {
    int new_selected = list->selected;
    while(new_selected < list->num_entries - 1) {
        new_selected++;
        if(list->entries[new_selected]->is_visible) {
            list->selected = new_selected;
            break;
        }
    }

    if(list->selected >= list->first_visible + list->num_visible) {
        list->first_visible = list->selected - list->num_visible + 1;
    }
}
