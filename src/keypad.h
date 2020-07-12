
#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>

struct Memory;
struct Cpu;
struct InputState;


struct Keypad
{
    uint8_t p1;
};


void keypad_init(struct Keypad *keypad, struct Memory *memory);
void keypad_tick(struct Keypad *keypad, struct Cpu *cpu, struct InputState *inputState);


#endif
