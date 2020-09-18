
#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>
#include <stdbool.h>

struct Memory;
struct Cpu;


struct Timer
{
    uint8_t div;
    int divIncrementCyclesLeft;
    uint8_t tma;

    uint8_t cyclesPerTick;
    bool stopped;

    uint8_t tima;
    int timaIncrementCyclesLeft;
};


void timer_init(struct Timer *timer, struct Memory *memory);
void timer_tick(struct Timer *timer, struct Cpu *cpu, int cycles);


#endif
