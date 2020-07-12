
#include <SDL2/SDL.h>

#include "input.h"


struct InputState input_get_state(void)
{
    bool quit = false;
    bool dumpMemory = false;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            quit = true;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
            case SDLK_q:
                quit = true;
                break;
            case SDLK_m:
                if ( ! event.key.repeat)
                {
                    dumpMemory = true;
                }
                break;
            default:
                break;
            }
        }
    }

    // Note: Must process events before calling SDL_GetKeyboardState()
    //       in order to get up-to-date keyboard state.
    const uint8_t *keystate = SDL_GetKeyboardState(NULL);
    struct InputState inputState = {
        .quit = quit,
        .dumpMemory = dumpMemory,

        .buttonA = keystate[SDL_SCANCODE_S],
        .buttonB = keystate[SDL_SCANCODE_A],
        .buttonStart = keystate[SDL_SCANCODE_RETURN],
        .buttonSelect = keystate[SDL_SCANCODE_RSHIFT],
        .dpadLeft = keystate[SDL_SCANCODE_LEFT],
        .dpadRight = keystate[SDL_SCANCODE_RIGHT],
        .dpadUp = keystate[SDL_SCANCODE_UP],
        .dpadDown = keystate[SDL_SCANCODE_DOWN],
    };
    return inputState;
}
