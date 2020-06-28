
#include "input.h"


struct InputState input_get_state(void)
{
    struct InputState state = {
        .quit = false,
        .buttonA = false,
        .buttonB = false,
        .buttonStart = false,
        .buttonSelect = false,
        .dpadLeft = false,
        .dpadRight = false,
        .dpadUp = false,
        .dpadDown = false,
    };

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            state.quit = true;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
            case SDLK_q:
                state.quit = true;
                break;

            case SDLK_UP:
                state.dpadUp = true;
                break;
            case SDLK_DOWN:
                state.dpadDown = true;
                break;
            case SDLK_LEFT:
                state.dpadLeft = true;
                break;
            case SDLK_RIGHT:
                state.dpadRight = true;
                break;

            case SDLK_z:
                state.buttonA = true;
                break;
            case SDLK_x:
                state.buttonB = true;
                break;

            case SDLK_RETURN:
                state.buttonStart = true;
                break;
            case SDLK_RSHIFT:
                state.buttonSelect = true;
                break;
            }
        }
    }

    return state;
}
