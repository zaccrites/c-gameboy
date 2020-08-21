
#include <SDL2/SDL.h>

#include "input.h"


#define KEYPAD_BUTTON_A       SDLK_s
#define KEYPAD_BUTTON_B       SDLK_a
#define KEYPAD_BUTTON_START   SDLK_RETURN
#define KEYPAD_BUTTON_SELECT  SDLK_RSHIFT
#define KEYPAD_DPAD_LEFT      SDLK_LEFT
#define KEYPAD_DPAD_RIGHT     SDLK_RIGHT
#define KEYPAD_DPAD_UP        SDLK_UP
#define KEYPAD_DPAD_DOWN      SDLK_DOWN


void input_update(struct InputState *input)
{
    input->quit = false;
    input->dumpMemory = false;

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_QUIT)
        {
            input->quit = true;
        }
        else if (event.type == SDL_KEYDOWN)
        {
            if (event.key.repeat)
            {
                continue;
            }

            switch (event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
            case SDLK_q:
                input->quit = true;
                break;
            case SDLK_m:
                input->dumpMemory = true;
                break;

            case KEYPAD_BUTTON_A:
                input->buttonA = true;
                break;
            case KEYPAD_BUTTON_B:
                input->buttonB = true;
                break;
            case KEYPAD_BUTTON_START:
                input->buttonStart = true;
                break;
            case KEYPAD_BUTTON_SELECT:
                input->buttonSelect = true;
                break;
            case KEYPAD_DPAD_LEFT:
                input->dpadLeft = true;
                break;
            case KEYPAD_DPAD_RIGHT:
                input->dpadRight = true;
                break;
            case KEYPAD_DPAD_UP:
                input->dpadUp = true;
                break;
            case KEYPAD_DPAD_DOWN:
                input->dpadDown = true;
                break;

            default:
                break;
            }
        }
        else if (event.type == SDL_KEYUP)
        {
            switch (event.key.keysym.sym)
            {
            case KEYPAD_BUTTON_A:
                input->buttonA = false;
                break;
            case KEYPAD_BUTTON_B:
                input->buttonB = false;
                break;
            case KEYPAD_BUTTON_START:
                input->buttonStart = false;
                break;
            case KEYPAD_BUTTON_SELECT:
                input->buttonSelect = false;
                break;
            case KEYPAD_DPAD_LEFT:
                input->dpadLeft = false;
                break;
            case KEYPAD_DPAD_RIGHT:
                input->dpadRight = false;
                break;
            case KEYPAD_DPAD_UP:
                input->dpadUp = false;
                break;
            case KEYPAD_DPAD_DOWN:
                input->dpadDown = false;
                break;

            default:
                break;
            }
        }
    }
}
