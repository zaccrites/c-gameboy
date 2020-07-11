
#include <stdio.h>
#include <stdbool.h>

#include "input.h"
#include "graphics.h"
#include "cartridge.h"
#include "memory.h"
#include "cpu.h"
#include "ppu.h"
#include "keypad.h"


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

    struct Ppu ppu;
    ppu_init(&ppu, &memory);

    struct Cpu cpu;
    cpu_init(&cpu, &memory);

    struct Keypad keypad;
    keypad_init(&keypad, &memory);

    bool isRunning = true;
    while (isRunning)
    {
        // TODO
        if ( ! cpu_execute_next(&cpu))
        {
            isRunning = false;
        }
        int instructionCycles = 1;  // TODO: accurate number of cycles for each instruction
        bool enteringVBlank = ppu_tick(&ppu, &cpu, instructionCycles, graphics.pixelBuffer);

        if (enteringVBlank)
        {
            struct InputState inputState = input_get_state();
            if (inputState.quit)
            {
                isRunning = false;
            }

            if (inputState.buttonSelect)
            {
                FILE *f = fopen("vram.bin", "wb");
                fwrite(memory.vram, sizeof(memory.vram[0]), sizeof(memory.vram), f);
                fclose(f);
                printf("Wrote contents of VRAM to file \n");
            }

            keypad_tick(&keypad, &cpu);
            graphics_update(&graphics);

            // TODO: Measure elasped time and sleep to achieve 60 FPS.
            SDL_Delay(16);
        }
    }

cleanup_graphics:
    graphics_teardown(&graphics);
cleanup_memory:
    memory_teardown(&memory);
cleanup_cartridge:
    cartridge_teardown(&cartridge);

    return statusCode;
}

