
#include <SDL2/SDL.h>

#include "input.h"


struct InputState input_get_state(void)
{
    bool quit = false;

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
            default:
                break;
            }
        }
    }

    // Note: MUST process events before calling SDL_KeyKeyboardState()
    const uint8_t *keystate = SDL_GetKeyboardState(NULL);
    struct InputState inputState = {
        .quit = quit,

        .buttonA = keystate[SDL_SCANCODE_Z],
        .buttonB = keystate[SDL_SCANCODE_X],
        .buttonStart = keystate[SDL_SCANCODE_RETURN],
        .buttonSelect = keystate[SDL_SCANCODE_RSHIFT],
        .dpadLeft = keystate[SDL_SCANCODE_LEFT],
        .dpadRight = keystate[SDL_SCANCODE_RIGHT],
        .dpadUp = keystate[SDL_SCANCODE_UP],
        .dpadDown = keystate[SDL_SCANCODE_DOWN],
    };
    return inputState;
}
