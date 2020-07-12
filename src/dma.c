
#include "dma.h"
#include "memory.h"

#define NUM_DMA_CYCLES  160
#define NUM_DMA_WORDS   MEMORY_OAM_SIZE


#define IO_REGISTER_DMA  0x46
static void io_handler_write_dma(IoRegisterFuncContext context, uint8_t value)
{
    struct Dma *dma = context;
    dma->cycleCounter = NUM_DMA_CYCLES;
    dma->sourceAddressStart = (uint16_t)value << 8;
    // TODO: lock memory
}


void dma_init(struct Dma *dma, struct Memory *memory)
{
    dma->cycleCounter = 0;

    dma->memory = memory;
    memory_register_io_handler(
        dma->memory,
        IO_REGISTER_DMA,
        NULL,
        io_handler_write_dma,
        dma
    );
}


void dma_tick(struct Dma *dma, int cycles)
{
    if (dma->cycleCounter > 0)
    {
        dma->cycleCounter -= cycles;
        if (dma->cycleCounter <= 0)
        {
            // TODO: Only copy these bytes as cycles tick by instead of all at once?
            for (uint16_t i = 0; i < NUM_DMA_WORDS; i++)
            {
                uint16_t sourceAddress = dma->sourceAddressStart + i;
                dma->memory->oam[i] = memory_read_word(dma->memory, sourceAddress);
            }
            // TODO: unlock memory
        }
    }
}


