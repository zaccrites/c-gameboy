
#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

struct Memory;


struct Timer
{
    uint8_t divRegister;
    int divIncrementCyclesLeft;
};


void timer_init(struct Timer *timer, struct Memory *memory);
void timer_tick(struct Timer *timer, int cycles);


#endif
