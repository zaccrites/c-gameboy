
#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>


struct InputState
{
    // Emulator controls
    bool quit;

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


struct InputState input_get_state(void);


#endif
