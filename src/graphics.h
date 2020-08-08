
#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <stdbool.h>

#include <SDL2/SDL.h>

#include "lcd.h"


struct GraphicsOptions
{
    bool smallWindow;
};


struct Graphics
{
    SDL_Window* sdlWindow;
    SDL_Renderer* sdlRenderer;
    SDL_Texture* sdlCanvasTexture;
    uint8_t pixelBuffer[4 * LCD_WIDTH * LCD_HEIGHT];
};


bool graphics_init(struct Graphics *graphics, struct GraphicsOptions *options);
void graphics_update(struct Graphics* graphics);
void graphics_teardown(struct Graphics *graphics);


#endif
