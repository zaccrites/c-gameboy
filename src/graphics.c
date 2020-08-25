
#include "graphics.h"


void graphics_update(struct Graphics* graphics)
{
    if (graphics->sdlWindow != NULL)
    {
        SDL_UpdateTexture(graphics->sdlCanvasTexture, NULL, graphics->pixelBuffer, 4 * LCD_WIDTH);
        SDL_RenderCopy(graphics->sdlRenderer, graphics->sdlCanvasTexture, NULL, NULL);
        SDL_RenderPresent(graphics->sdlRenderer);
    }
}


bool graphics_init(struct Graphics *graphics, struct GraphicsOptions *options)
{
    // TODO: Error handling and reporting

    if (options->headless)
    {
        // FUTURE: Could possibly still record video and/or screenshots
        //   while running in headless mode, even if there's not SDL window.
        graphics->sdlWindow = NULL;
        graphics->sdlRenderer = NULL;
        graphics->sdlCanvasTexture = NULL;
        return true;
    }

    int windowWidth = LCD_WIDTH;
    int windowHeight = LCD_HEIGHT;
    if ( ! options->smallWindow)
    {
        windowWidth *= 4;
        windowHeight *= 4;
    }

    graphics->sdlWindow = SDL_CreateWindow(
        "GameBoy Emulator",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        windowWidth,
        windowHeight,
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
