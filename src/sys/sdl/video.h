#ifndef SDL_VIDEO_H
#define SDL_VIDEO_H

#include <SDL/SDL.h>

void video_init();
void video_switch_display_mode();
void video_render(SDL_Surface *surface, SDL_Rect area);

#endif
