
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>

#include <SDL2/SDL.h>


#define SCREEN_WIDTH   160
#define SCREEN_HEIGHT  144

#define WINDOW_WIDTH   (SCREEN_WIDTH * 4)
#define WINDOW_HEIGHT  (SCREEN_HEIGHT * 4)


struct Graphics
{
    SDL_Window* sdlWindow;
    SDL_Renderer* sdlRenderer;
    SDL_Texture* sdlCanvasTexture;
};


void graphics_update(struct Graphics* graphics);
bool graphics_init(struct Graphics *graphics);
void graphics_teardown(struct Graphics *graphics);


#endif
