#include "util.h"
#include <SDL/SDL_ttf.h>
#include "sys/sdl/input.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define FONT_SIZE 40
#define SCROLL_THRESHOLD 500
#define SCROLL_DELAY 20
#define SCROLL_NOT_PRESSED 0
#define SCROLL_TO_THRESHOLD 1
#define SCROLL_ACTIVE 2


static TTF_Font *font;

static void list_up(menu_list_t *list) {
    if(list->selected < 0) {
        return;
    }

    int new_selected = list->selected;
    while(new_selected > 0) {
        new_selected--;
        if(list->entries[new_selected]->is_visible && list->entries[new_selected]->type != MENU_LISTENTRY_SPACER) {
            list->selected = new_selected;
            break;
        }
    }

    if(list->selected < list->first_visible) {
        list->first_visible = list->selected;
    }
    list->last_scroll[0] = SDL_GetTicks();
}

static void list_down(menu_list_t *list) {
    if(list->selected < 0) {
        return;
    }

    int new_selected = list->selected;
    while(new_selected < list->num_entries - 1) {
        new_selected++;
        if(list->entries[new_selected]->is_visible && list->entries[new_selected]->type != MENU_LISTENTRY_SPACER) {
            list->selected = new_selected;
            break;
        }
    }

    if(list->selected >= list->first_visible + list->num_visible) {
        list->first_visible = list->selected - list->num_visible + 1;
    }
    list->last_scroll[1] = SDL_GetTicks();
}

void menu_util_init() {
    if(!TTF_WasInit()) {
        assert(!TTF_Init());

        int font_size = SDL_GetVideoSurface()->h / 15;
        font = TTF_OpenFont("data/Xolonium.ttf", font_size);
        assert(font != NULL);
    }
}

void menu_util_close() {
    TTF_CloseFont(font);
}

menu_label_t *menu_label(const char *text) {
    menu_label_t *label;
    SDL_Color unselected = {180, 180, 200}, selected = {120, 120, 255};

    label = malloc(sizeof(*label));
    label->surfaces[0] = TTF_RenderText_Blended(font, text, unselected);
    label->surfaces[1] = TTF_RenderText_Blended(font, text, selected);

    return label;
}

SDL_Surface *menu_text(const char *text) {
    SDL_Color color = {120, 120, 255};
    return TTF_RenderText_Blended(font, text, color);
}

void menu_free_label(menu_label_t *label) {
    SDL_FreeSurface(label->surfaces[0]);
    SDL_FreeSurface(label->surfaces[1]);
}

static void mark_word_end(char *text, int *c) {
    char *space = strchr(text, ' ');
    if(space == NULL) {
        *c = -1;
        return;
    }
    *space = '\0';
    *c += strlen(text) + 1;
}

menu_word_string_t *menu_word_string(const char *_text) {
    SDL_Color color = {120, 120, 255};
    menu_word_string_t *result = malloc(sizeof(*result)), *string;
    char *text = strdup(_text);
    int word = 0, next_word;

    for(string = result; word >= 0; string = string->next) {
        next_word = word;
        mark_word_end(&text[word], &next_word);
        string->word = TTF_RenderText_Blended(font, &text[word], color);

        word = next_word;
        string->next = word < 0 ? NULL : malloc(sizeof(*result));
    }
    free(text);

    return result;
}

void menu_free_word_string(menu_word_string_t *string) {
    for(; string != NULL;) {
        menu_word_string_t *tmp = string;
        SDL_FreeSurface(string->word);
        string = string->next;
        free(tmp);
    }
}

void menu_blit(SDL_Surface *s, int x, int y) {
    SDL_Rect r = {x, y, s->w, s->h};
    SDL_BlitSurface(s, NULL, SDL_GetVideoSurface(), &r);
}

void menu_blit_word_string(menu_word_string_t *string, int x, int y) {
    int cx, cy, sw, lh;

    cx = x;
    cy = y;
    TTF_GlyphMetrics(font, ' ', NULL, NULL, NULL, NULL, &sw);
    lh = TTF_FontLineSkip(font);

    for(; string != NULL;) {
        menu_blit(string->word, cx, cy);
        cx += string->word->w;
        cx += sw;

        if(string->next != NULL) {
            if(cx + string->next->word->w > SDL_GetVideoSurface()->w) {
                cx = x;
                cy += lh;
            }
        }

        string = string->next;
    }
}

menu_list_t *menu_new_list(const char *title) {
    menu_list_t *list;

    list = malloc(sizeof(*list));

    list->entries = NULL;
    list->num_entries = 0;
    list->title = menu_label(title);
    list->selected = -1;
    list->num_visible = 1;
    list->back_func = NULL;
    list->first_visible = 0;
    list->scroll_state[0] = SCROLL_NOT_PRESSED; list->scroll_state[1] = SCROLL_NOT_PRESSED;
    list->last_scroll[0] = 0; list->last_scroll[1] = 0;

    return list;
}

void menu_list_update(menu_list_t *list) {
    int d;
    for(d = 0; d < 2; d++) {
        if(list->scroll_state[d] == SCROLL_TO_THRESHOLD) {
            if(SDL_GetTicks() - list->last_scroll[d] > SCROLL_THRESHOLD) {
                list->scroll_state[d] = SCROLL_ACTIVE;
            }
        }
        if(list->scroll_state[d] == SCROLL_ACTIVE && SDL_GetTicks() - list->last_scroll[d] > SCROLL_DELAY) {
            if(d == 0) {
                list_up(list);
            }
            else {
                list_down(list);
            }
        }
    }
}

void menu_free_list(menu_list_t *list) {
    menu_clear_list(list);
    free(list);
}

void menu_clear_list(menu_list_t *list) {
    int e;
    for(e = 0; e < list->num_entries; e++) {
        free(list->entries[e]);
    }
    free(list->entries);
    list->entries = NULL;
    list->num_entries = 0;
    list->first_visible = 0;
    list->selected = -1;
}

static menu_listentry_t *new_listentry(menu_list_t *list, const char *text, int id, int type, void *func) {
    menu_listentry_t *entry;

    entry = malloc(sizeof(*entry));

    entry->text = text != NULL ? menu_label(text) : NULL;
    entry->val = NULL;
    entry->id = id;
    entry->is_visible = 1;
    entry->type = type;

    switch(type) {
        case MENU_LISTENTRY_BUTTON: entry->func.accept = (void (*)(void))func; break;
        case MENU_LISTENTRY_SELECTION: entry->func.change = (void (*)(int))func; break;
    }
    list->entries = realloc(list->entries, sizeof(*list->entries) * (++list->num_entries));
    list->entries[list->num_entries-1] = entry;

    if(list->selected < 0) {
        list->selected = 0;
    }

    return entry;
}

void menu_new_listentry_button(menu_list_t *list, const char *name, int id, void (*accept_func)(void)) {
    new_listentry(list, name, id, MENU_LISTENTRY_BUTTON, accept_func);
}

void menu_new_listentry_selection(menu_list_t *list, const char *name, int id, void (*change_func)(int)) {
    new_listentry(list, name, id, MENU_LISTENTRY_SELECTION, change_func);
}

void menu_new_listentry_spacer(menu_list_t *list) {
    new_listentry(list, NULL, -1, MENU_LISTENTRY_SPACER, NULL);
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
    snprintf(val, sizeof(val), "%i", ival);
    menu_listentry_val(list, key, val);
}

int menu_list_selected_id(menu_list_t *list) {
    if(list->selected < 0) {
        return -1;
    }
    else {
        return list->entries[list->selected]->id;
    }
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

static void hide_entry(menu_list_t *list, int idx) {
    int e;
    list->entries[idx]->is_visible = 0;
    if(list->selected != idx) {
        return;
    }

    for(e = idx - 1; e >= 0; e--) {
        if(list->entries[e]->is_visible) {
            list->selected = e;
            return;
        }
    }
    for(e = idx + 1; e < list->num_entries; e++) {
        if(list->entries[e]->is_visible) {
            list->selected = e;
            return;
        }
    }
    return;
}

void menu_listentry_visible(menu_list_t *list, int id, int visible) {
    int e;
    for(e = 0; e < list->num_entries; e++) {
        if(list->entries[e]->id == id) {
            if(visible) {
                list->entries[e]->is_visible = 1;
            }
            else {
                hide_entry(list, e);
            }
            return;
        }
    }
}

void menu_draw_list(menu_list_t *list) {
    int e, l;
    int last_visible;
    int top_margin, left_margin, right_margin, bottom_margin;
    int line_height;
    SDL_Surface *screen;

    top_margin = TTF_FontHeight(font) * 1.5;
    left_margin = SDL_GetVideoSurface()->w / 40;
    right_margin = SDL_GetVideoSurface()->w / 40;
    bottom_margin = TTF_FontHeight(font) * 1.5;
    line_height = TTF_FontHeight(font);
    screen = SDL_GetVideoSurface();

    list->num_visible = (SDL_GetVideoSurface()->h - top_margin - bottom_margin) / line_height;

    menu_blit(list->title->surfaces[0], 0, 0);

    last_visible = list->first_visible + list->num_visible - 1;
    last_visible = min(last_visible, list->num_entries - 1);
    for(e = list->first_visible, l = 0; e <= last_visible; e++) {
        if(list->entries[e]->is_visible) {
            if(list->entries[e]->type != MENU_LISTENTRY_SPACER) {
                menu_blit(list->entries[e]->text->surfaces[list->selected == e ? 1 : 0],
                          left_margin, top_margin + l * line_height);

                if(list->entries[e]->val != NULL) {
                    SDL_Surface *val = list->entries[e]->val->surfaces[list->selected == e ? 1 : 0];
                    menu_blit(val, screen->w - right_margin - val->w, top_margin + l * line_height);
                }
            }
            l++;
        }
    }
}

void menu_list_input(menu_list_t *list, int type, int key) {
    int dir;
    if(type == SDL_KEYDOWN) {
        switch(key) {
            case SDLK_UP:
                list_up(list);
                list->scroll_state[0] = SCROLL_TO_THRESHOLD;
            break;
            case SDLK_DOWN:
                list_down(list);
                list->scroll_state[1] = SCROLL_TO_THRESHOLD;
            break;
            case SDLK_LEFT: dir = -1;
            case SDLK_RIGHT: dir = 1;
                if(list->selected >= 0) {
                    if(list->entries[list->selected]->func.change != NULL) {
                        list->entries[list->selected]->func.change(dir);
                    }
                }
            break;
        }

        if(key == input.keys.accept) {
            if(list->selected >= 0) {
                if(list->entries[list->selected]->func.accept != NULL) {
                    list->entries[list->selected]->func.accept();
                }
            }
        }
        if(key == input.keys.back) {
            if(list->selected >= 0) {
                if(list->back_func != NULL) {
                    list->back_func();
                }
            }
        }
    }
    if(type == SDL_KEYUP) {
        switch(key) {
            case SDLK_UP: list->scroll_state[0] = SCROLL_NOT_PRESSED; break;
            case SDLK_DOWN: list->scroll_state[1] = SCROLL_NOT_PRESSED; break;
        }
    }
}