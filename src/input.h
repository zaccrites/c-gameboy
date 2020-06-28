
#ifndef INPUT_H
#define INPUT_H

#include <stdbool.h>

#include <SDL2/SDL.h>


struct InputState
{
    bool quit;
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
