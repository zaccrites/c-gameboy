
#include <stdbool.h>

#include "keypad.h"
#include "memory.h"
#include "input.h"
#include "cpu.h"


#define IO_REGISTER_P1  0x00
static uint8_t io_handler_read_p1_register(IoRegisterFuncContext context)
{
    struct Keypad *keypad = context;
    return keypad->p1;
}

static void io_handler_write_p1_register(IoRegisterFuncContext context, uint8_t value)
{
    struct Keypad *keypad = context;
    keypad->p1 = (keypad->p1 & ~0x30) | (value & 0x30);
}


void keypad_init(struct Keypad *keypad, struct Memory *memory)
{
    keypad->p1 = 0xcf;
    memory_register_io_handler(
        memory,
        IO_REGISTER_P1,
        io_handler_read_p1_register,
        io_handler_write_p1_register,
        keypad
    );
}


void keypad_tick(struct Keypad *keypad, struct Cpu *cpu, struct InputState *inputState)
{
    bool dpadSelected = (keypad->p1 & (1 << 5)) != 0;
    bool dpadRight = dpadSelected && inputState->dpadRight;
    bool dpadLeft = dpadSelected && inputState->dpadLeft;
    bool dpadUp = dpadSelected && inputState->dpadUp;
    bool dpadDown = dpadSelected && inputState->dpadDown;

    bool buttonsSelected = (keypad->p1 & (1 << 4)) != 0;
    bool buttonA = buttonsSelected && inputState->buttonA;
    bool buttonB = buttonsSelected && inputState->buttonB;
    bool buttonSelect = buttonsSelected && inputState->buttonSelect;
    bool buttonStart = buttonsSelected && inputState->buttonStart;

    uint8_t outputBitsBefore = keypad->p1 & 0x0f;
    keypad->p1 |= 0x0f;
    if (dpadRight || buttonA)      { keypad->p1 &= ~(1 << 0); }
    if (dpadLeft  || buttonB)      { keypad->p1 &= ~(1 << 1); }
    if (dpadUp    || buttonSelect) { keypad->p1 &= ~(1 << 2); }
    if (dpadDown  || buttonStart)  { keypad->p1 &= ~(1 << 3); }
    uint8_t outputBitsAfter = keypad->p1 & 0x0f;

    uint8_t outputBitsHighToLow = outputBitsBefore & ~outputBitsAfter;
    if (outputBitsHighToLow)
    {
        cpu_request_interrupt(cpu, INTERRUPT_KEYPAD);
    }
}
