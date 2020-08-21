
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

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
#include "serial.h"


static void dump_memory(struct Memory *memory)
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


static FILE* open_serial_log_file(const char *path)
{
    assert(path != NULL);
    return fopen(path, "wb");
}

static void teardown_serial_log_file(FILE* file)
{
    if (file != NULL)
    {
        fclose(file);
    }
}


int main(int argc, char **argv)
{
    struct Options options;
    int statusCode = parse_options(argc, argv, &options);
    if (statusCode != 0 || options.exitEarly)
    {
        return statusCode;
    }

    FILE *serialLogFile = NULL;
    if (options.serialOutPath != NULL)
    {
        serialLogFile = open_serial_log_file(options.serialOutPath);
        if (serialLogFile == NULL)
        {
            statusCode = 1;
            fprintf(stderr, "error: failed to open serial log file \n");
            goto cleanup_serial;
        }
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

    // TODO: Headless mode affects graphics output AND the input system.
    // Basically we can't use SDL at all.

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

    struct Dma dma;
    dma_init(&dma, &memory);

    struct Timer timer;
    timer_init(&timer, &memory);

    struct Serial serial;
    serial_init(&serial, &memory);

    struct InputState inputState;
    input_update(&inputState);

    struct Keypad keypad;
    keypad_init(&keypad, &inputState, &cpu, &memory);

    uint32_t frameStartTime = SDL_GetTicks();
    bool isRunning = true;
    while (isRunning)
    {
        if ( ! cpu_execute_next(&cpu))
        {
            isRunning = false;
        }

        int instructionCycles = 1;  // TODO: accurate number of cycles for each instruction
        keypad_tick(&keypad);
        dma_tick(&dma, instructionCycles);
        timer_tick(&timer, &cpu, instructionCycles);

        bool enteringVBlank = ppu_tick(&ppu, &cpu, instructionCycles, graphics.pixelBuffer);

        bool serialTransferComplete = serial_tick(&serial, &cpu, instructionCycles);
        if (serialTransferComplete && serialLogFile != NULL)
        {
            fwrite(&serial.outgoingData, sizeof(serial.outgoingData), 1, serialLogFile);
        }

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
            input_update(&inputState);
            graphics_update(&graphics);

            uint32_t frameMs = SDL_GetTicks() - frameStartTime;
            uint32_t targetMs = 1000 / 60;
            if (targetMs > frameMs)
            {
                SDL_Delay(targetMs - frameMs);
            }

            // TODO: Add flag for turning diagnostics like this on and off
            // frameMs = SDL_GetTicks() - frameStartTime;
            // {
            //     printf("frame took %d ms \n", frameMs);
            // }
            frameStartTime = SDL_GetTicks();
        }
    }

cleanup_graphics:
    graphics_teardown(&graphics);
cleanup_memory:
    memory_teardown(&memory);
cleanup_cartridge:
    cartridge_teardown(&cartridge);
cleanup_serial:
    teardown_serial_log_file(serialLogFile);

    return statusCode;
}

