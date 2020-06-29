
#include <stdio.h>
#include <stdbool.h>

#include "input.h"
#include "graphics.h"
#include "cartridge.h"
#include "memory.h"


int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Error: please provide path to ROM file \n");
        return 1;
    }

    int statusCode = 0;

    struct Cartridge cartridge;
    if ( ! cartridge_load(&cartridge, argv[1]))
    {
        fprintf(stderr, "Error: Failed to load the cartridge! \n");
        statusCode = 1;
        goto cleanup_cartridge;
    }

    struct CartridgeHeader cartridgeHeader;
    cartridge_get_header(&cartridge, &cartridgeHeader);
    printf("Loaded ROM \"%s\" (%ld bytes)\n", cartridgeHeader.title, cartridge.dataSize);

    struct Memory memory;
    if ( ! memory_init(&memory, &cartridge))
    {
        fprintf(stderr, "Error: Failed to create the memory mapper! \n");
        statusCode = 1;
        goto cleanup_memory;
    }

    struct Graphics graphics;
    if ( ! graphics_init(&graphics))
    {
        fprintf(stderr, "Error: Failed to start the graphics! \n");
        statusCode = 1;
        goto cleanup_graphics;
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

cleanup_graphics:
    graphics_teardown(&graphics);
cleanup_memory:
    memory_teardown(&memory);
cleanup_cartridge:
    cartridge_teardown(&cartridge);

    return statusCode;
}

