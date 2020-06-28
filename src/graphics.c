
#include "graphics.h"


void graphics_update(struct Graphics* graphics)
{
    // TODO

    unsigned char data[4 * SCREEN_WIDTH * SCREEN_HEIGHT];
    for (unsigned int i = 0; i < sizeof(data); i += 4)
    {
        data[i + 0] = 237;  // B
        data[i + 1] = 149;  // G
        data[i + 2] = 100;  // R
        data[i + 3] = 255;  // A
    }

    SDL_UpdateTexture(graphics->sdlCanvasTexture, NULL, data, 4 * SCREEN_WIDTH);
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
        SCREEN_WIDTH,
        SCREEN_HEIGHT
    );
    if (graphics->sdlCanvasTexture == NULL)
    {
        return false;
    }

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
