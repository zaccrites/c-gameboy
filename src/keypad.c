
#include <stdbool.h>

#include "keypad.h"
#include "memory.h"
#include "input.h"
#include "cpu.h"


static void update_state(struct Keypad *keypad)
{
    bool dpadSelected = (keypad->p1 & (1 << 5)) != 0;
    bool dpadRight = dpadSelected && keypad->inputState->dpadRight;
    bool dpadLeft = dpadSelected && keypad->inputState->dpadLeft;
    bool dpadUp = dpadSelected && keypad->inputState->dpadUp;
    bool dpadDown = dpadSelected && keypad->inputState->dpadDown;

    bool buttonsSelected = (keypad->p1 & (1 << 4)) != 0;
    bool buttonA = buttonsSelected && keypad->inputState->buttonA;
    bool buttonB = buttonsSelected && keypad->inputState->buttonB;
    bool buttonSelect = buttonsSelected && keypad->inputState->buttonSelect;
    bool buttonStart = buttonsSelected && keypad->inputState->buttonStart;

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
        cpu_request_interrupt(keypad->cpu, INTERRUPT_KEYPAD);
    }
}


#define IO_REGISTER_P1  0x00
static uint8_t io_handler_read_p1_register(IoRegisterFuncContext context)
{
    struct Keypad *keypad = context;
    update_state(keypad);
    return keypad->p1;
}

static void io_handler_write_p1_register(IoRegisterFuncContext context, uint8_t value)
{
    struct Keypad *keypad = context;
    update_state(keypad);
    keypad->p1 = (keypad->p1 & ~0x30) | (value & 0x30);
}


void keypad_init(struct Keypad *keypad, struct InputState *inputState, struct Cpu *cpu, struct Memory *memory)
{
    keypad->p1 = 0xcf;
    keypad->cpu = cpu;
    keypad->inputState = inputState;
    memory_register_io_handler(
        memory,
        IO_REGISTER_P1,
        io_handler_read_p1_register,
        io_handler_write_p1_register,
        keypad
    );
}


void keypad_tick(struct Keypad *keypad)
{
    update_state(keypad);
}
