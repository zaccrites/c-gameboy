
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>

#include <SDL2/SDL.h>

#include "lcd.h"


#define WINDOW_WIDTH   (LCD_WIDTH * 4)
#define WINDOW_HEIGHT  (LCD_HEIGHT * 4)


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
