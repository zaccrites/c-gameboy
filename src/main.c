
#include <stdio.h>
#include <stdbool.h>

#include "options.h"
#include "input.h"
#include "graphics.h"
#include "cartridge.h"
#include "memory.h"
#include "cpu.h"
#include "ppu.h"
#include "keypad.h"
#include "dma.h"
#include "timer.h"


void dump_memory(struct Memory *memory)
{
    FILE *f;

    f = fopen("vram.bin", "wb");
    fwrite(memory->vram, sizeof(memory->vram[0]), sizeof(memory->vram), f);
    fclose(f);

    f = fopen("oam.bin", "wb");
    fwrite(memory->oam, sizeof(memory->oam[0]), sizeof(memory->oam), f);
    fclose(f);

    printf("Memory dumped to file \n");
}


int main(int argc, char **argv)
{
    struct Options options;
    int statusCode = parse_options(argc, argv, &options);
    if (statusCode != 0 || options.exitEarly)
    {
        return statusCode;
    }

    struct Cartridge cartridge;
    if ( ! cartridge_load(&cartridge, argv[1]))
    {
        fprintf(stderr, "error: failed to load the cartridge! \n");
        statusCode = 1;
        goto cleanup_cartridge;
    }

    struct CartridgeHeader cartridgeHeader;
    cartridge_get_header(&cartridge, &cartridgeHeader);
    char cartridgeTypeStringBuffer[64];
    cartridge_get_type_string(&cartridgeHeader, cartridgeTypeStringBuffer, sizeof(cartridgeTypeStringBuffer));
    printf("Loaded ROM \"%s\" (%ld bytes) (%s)\n", cartridgeHeader.title, cartridge.dataSize, cartridgeTypeStringBuffer);

    struct Memory memory;
    if ( ! memory_init(&memory, &cartridge))
    {
        fprintf(stderr, "error: failed to create the memory mapper! \n");
        statusCode = 1;
        goto cleanup_memory;
    }

    struct Graphics graphics;
    if ( ! graphics_init(&graphics, &options.graphics))
    {
        fprintf(stderr, "error: failed to start the graphics! \n");
        statusCode = 1;
        goto cleanup_graphics;
    }

    struct Ppu ppu;
    ppu_init(&ppu, &memory);

    struct Cpu cpu;
    cpu_init(&cpu, &memory);

    struct Keypad keypad;
    keypad_init(&keypad, &memory);

    struct Dma dma;
    dma_init(&dma, &memory);

    struct Timer timer;
    timer_init(&timer, &memory);

    struct InputState inputState = input_get_state();

    bool isRunning = true;
    while (isRunning)
    {
        // TODO
        if ( ! cpu_execute_next(&cpu))
        {
            isRunning = false;
        }
        int instructionCycles = 1;  // TODO: accurate number of cycles for each instruction
        keypad_tick(&keypad, &cpu, &inputState);
        dma_tick(&dma, instructionCycles);
        timer_tick(&timer, &cpu, instructionCycles);
        bool enteringVBlank = ppu_tick(&ppu, &cpu, instructionCycles, graphics.pixelBuffer);

        inputState = input_get_state();
        if (inputState.quit)
        {
            isRunning = false;
        }
        if (inputState.dumpMemory)
        {
            dump_memory(&memory);
        }

        if (enteringVBlank)
        {
            graphics_update(&graphics);

            // TODO: Measure elasped time and sleep to achieve 60 FPS.
            // SDL_Delay(16);
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

