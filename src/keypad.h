
#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>

struct Memory;
struct Cpu;
struct InputState;


struct Keypad
{
    uint8_t p1;
    struct Cpu *cpu;
    struct InputState *inputState;
};


void keypad_init(struct Keypad *keypad, struct InputState *inputState, struct Cpu *cpu, struct Memory *memory);
void keypad_tick(struct Keypad *keypad);


#endif
