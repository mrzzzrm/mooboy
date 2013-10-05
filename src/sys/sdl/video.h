#ifndef SDL_VIDEO_H
#define SDL_VIDEO_H

#include <SDL/SDL.h>

void video_switch_display_mode();
void video_set_area(SDL_Rect rect);
void video_render(SDL_Surface *surface);
void video_init();
void video_close();

#endif
