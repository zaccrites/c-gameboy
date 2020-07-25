
#include "timer.h"
#include "memory.h"


// Each machine cycle is clocked at 1 MHz.
// The DIV register increments 16,384 times per second,
// so we have to increment the register once every ~61 machine cycles.
#define DIV_INCREMENT_CYCLES  61


#define IO_REGISTER_DIV  0x04
static uint8_t io_handler_read_div_register(IoRegisterFuncContext context)
{
    struct Timer *timer = context;
    return timer->divRegister;
}

static void io_handler_write_div_register(IoRegisterFuncContext context, uint8_t value)
{
    (void)value;
    struct Timer *timer = context;
    timer->divRegister = 0;
}


void timer_init(struct Timer *timer, struct Memory *memory)
{
    timer->divRegister = 0;
    timer->divIncrementCyclesLeft = DIV_INCREMENT_CYCLES;
    memory_register_io_handler(
        memory,
        IO_REGISTER_DIV,
        io_handler_read_div_register,
        io_handler_write_div_register,
        timer
    );
}


void timer_tick(struct Timer *timer, int cycles)
{
    timer->divIncrementCyclesLeft -= cycles;
    if (timer->divIncrementCyclesLeft <= 0)
    {
        timer->divRegister += 1;
        timer->divIncrementCyclesLeft += DIV_INCREMENT_CYCLES;
    }
}

