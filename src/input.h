
#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>


struct InputState
{
    // Emulator controls
    bool quit;
    bool dumpMemory;

    // GameBoy controls
    bool buttonA;
    bool buttonB;
    bool buttonStart;
    bool buttonSelect;
    bool dpadLeft;
    bool dpadRight;
    bool dpadUp;
    bool dpadDown;
};


void input_update(struct InputState *input);


#endif
