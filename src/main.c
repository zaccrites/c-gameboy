
#include <stdio.h>
#include <stdbool.h>

#include "input.h"
#include "graphics.h"


int main(int argc, char **argv)
{
    printf("Hello world! \n");

    struct Graphics graphics;
    if ( ! graphics_init(&graphics))
    {
        fprintf(stderr, "Failed to start the graphics! \n");
        graphics_teardown(&graphics);
        return 1;
    }

    bool isRunning = true;
    while (isRunning)
    {
        struct InputState inputState = input_get_state();
        if (inputState.quit)
        {
            isRunning = false;
        }

        graphics_update(&graphics);

        // TODO: Measure elasped time and sleep to achieve 60 FPS.
        SDL_Delay(16);
    }

    graphics_teardown(&graphics);
    return 0;
}

