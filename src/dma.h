
#ifndef DMA_H
#define DMA_H

#include <stdint.h>

struct Memory;


struct Dma
{
    int cycleCounter;
    uint16_t sourceAddressStart;
    struct Memory *memory;
};


void dma_init(struct Dma *dma, struct Memory *memory);
void dma_tick(struct Dma *dma, int cycles);


#endif
