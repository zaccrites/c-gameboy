
#include <stdio.h>
#include <stdbool.h>

#include "input.h"
#include "graphics.h"
#include "cartridge.h"
#include "memory.h"
#include "cpu.h"
#include "ppu.h"
#include "keypad.h"


// // TODO: Find a better place for this.
// static uint8_t read_unimpl_io_reg(IoRegisterFuncContext context) { (void)context; return 0xff; }
// static void write_unimpl_io_reg(IoRegisterFuncContext context, uint8_t value) { (void)context; (void)value; }
// static void setup_unimplemented_io_registers(struct Memory *memory)
// {
//     uint8_t regnums[] = {
//         // Joypad
//         0x00,
//         // Serial
//         0x01, 0x02,
//         // DIV Register
//         0x04,
//         // Timer
//         0x05, 0x06, 0x07,
//         // Sound
//         0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e,
//         0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26,
//         0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,

//         // These IO devices don't exist, but can still be written to anyway.
//         // They should always return 0xff
//         0x1f,
//         0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x
//     };
//     for (size_t i = 0; i < sizeof(regnums) / sizeof(regnums[0]); i++)
//     {
//         memory_register_io_handler(
//             memory,
//             regnums[i],
//             read_unimpl_io_reg,
//             write_unimpl_io_reg,
//             NULL
//         );
//     }
// }


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
    // setup_unimplemented_io_registers(&memory);

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

    int cycles = 0;
    bool isRunning = true;
    while (isRunning)
    {


        // TODO
        if ( ! cpu_execute_next(&cpu))
        {
            isRunning = false;
        }
        int instructionCycles = 1;  // TODO: accurate number of cycles for each instruction
        cycles += instructionCycles;
        ppu_tick(&ppu, &cpu, instructionCycles);


        // TODO: Look for VBlank
        if (cycles == 1000000/60)
        {
            struct InputState inputState = input_get_state();
            if (inputState.quit)
            {
                isRunning = false;
            }

            keypad_tick(&keypad, &cpu);

            bool isVblank = ppu_get_mode(&ppu) == PPU_MODE_VBLANK;
            if (isVblank)
            {
                ppu_render_vram(&ppu, graphics.pixelBuffer);
                graphics_update(&graphics);
            }

            cycles = 0;

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

