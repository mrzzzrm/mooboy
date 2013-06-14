#ifndef SDL_VIDEO_H
#define SDL_VIDEO_H

#include <SDL/SDL.h>

void sdl_video_init();
void sdl_video_render(SDL_Surface *surface, SDL_Rect area);

#endif
