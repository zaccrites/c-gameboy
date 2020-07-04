
#include "graphics.h"


void graphics_update(struct Graphics* graphics)
{
    SDL_UpdateTexture(graphics->sdlCanvasTexture, NULL, graphics->pixelBuffer, 4 * LCD_WIDTH);
    SDL_RenderCopy(graphics->sdlRenderer, graphics->sdlCanvasTexture, NULL, NULL);
    SDL_RenderPresent(graphics->sdlRenderer);
}


bool graphics_init(struct Graphics *graphics)
{
    // TODO: Error handling and reporting

    graphics->sdlWindow = SDL_CreateWindow(
        "GameBoy Emulator",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    if (graphics->sdlWindow == NULL)
    {
        return false;
    }

    graphics->sdlRenderer = SDL_CreateRenderer(graphics->sdlWindow, -1, SDL_RENDERER_ACCELERATED);
    if (graphics->sdlRenderer == NULL)
    {
        return false;
    }

    graphics->sdlCanvasTexture = SDL_CreateTexture(
        graphics->sdlRenderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        LCD_WIDTH,
        LCD_HEIGHT
    );
    if (graphics->sdlCanvasTexture == NULL)
    {
        return false;
    }

    for (unsigned int i = 0; i < sizeof(graphics->pixelBuffer); i += 4)
    {
        graphics->pixelBuffer[i + 0] = 237;  // B
        graphics->pixelBuffer[i + 1] = 149;  // G
        graphics->pixelBuffer[i + 2] = 100;  // R
        graphics->pixelBuffer[i + 3] = 255;  // A
    }
    graphics_update(graphics);

    return true;
}



void graphics_teardown(struct Graphics *graphics)
{
    if (graphics->sdlCanvasTexture != NULL)
    {
        SDL_DestroyTexture(graphics->sdlCanvasTexture);
        graphics->sdlCanvasTexture = NULL;
    }

    if (graphics->sdlRenderer != NULL)
    {
        SDL_DestroyRenderer(graphics->sdlRenderer);
        graphics->sdlRenderer = NULL;
    }

    if (graphics->sdlWindow != NULL)
    {
        SDL_DestroyWindow(graphics->sdlWindow);
        graphics->sdlWindow = NULL;
    }

    SDL_Quit();
}
