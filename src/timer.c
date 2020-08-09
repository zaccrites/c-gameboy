
#include "timer.h"
#include "memory.h"
#include "cpu.h"


#define MACHINE_CYCLE_FREQUENCY  1000000  // 1 MHz

#define DIV_INCREMENT_CYCLES               (MACHINE_CYCLE_FREQUENCY / 16384)
#define TIMER_INPUT_CLOCK_CYCLES_4096HZ    (MACHINE_CYCLE_FREQUENCY / 4096)
#define TIMER_INPUT_CLOCK_CYCLES_262144HZ  (MACHINE_CYCLE_FREQUENCY / 262144)
#define TIMER_INPUT_CLOCK_CYCLES_65536HZ   (MACHINE_CYCLE_FREQUENCY / 65536)
#define TIMER_INPUT_CLOCK_CYCLES_16384HZ   (MACHINE_CYCLE_FREQUENCY / 16384)


#define IO_REGISTER_DIV  0x04
static uint8_t io_handler_read_div_register(IoRegisterFuncContext context)
{
    struct Timer *timer = context;
    return timer->div;
}

static void io_handler_write_div_register(IoRegisterFuncContext context, uint8_t value)
{
    (void)value;
    struct Timer *timer = context;
    timer->div = 0;
}


#define IO_REGISTER_TIMA  0x05
static uint8_t io_handler_read_tima_register(IoRegisterFuncContext context)
{
    struct Timer *timer = context;
    return timer->tima;
}

static void io_handler_write_tima_register(IoRegisterFuncContext context, uint8_t value)
{
    struct Timer *timer = context;
    timer->tima = value;
}


#define IO_REGISTER_TMA  0x06
static uint8_t io_handler_read_tma_register(IoRegisterFuncContext context)
{
    struct Timer *timer = context;
    return timer->tma;
}

static void io_handler_write_tma_register(IoRegisterFuncContext context, uint8_t value)
{
    struct Timer *timer = context;
    timer->tma = value;
}


#define IO_REGISTER_TAC  0x07
static uint8_t io_handler_read_tac_register(IoRegisterFuncContext context)
{
    struct Timer *timer = context;
    uint8_t value = 0;
    if ( ! timer->stopped) value |= (1 << 3);
    switch (timer->cyclesPerTick)
    {
    case TIMER_INPUT_CLOCK_CYCLES_4096HZ:
        value |= 0x00;
        break;
    case TIMER_INPUT_CLOCK_CYCLES_262144HZ:
        value |= 0x01;
        break;
    case TIMER_INPUT_CLOCK_CYCLES_65536HZ:
        value |= 0x02;
        break;
    case TIMER_INPUT_CLOCK_CYCLES_16384HZ:
    default:
        value |= 0x03;
        break;
    }
    return value;
}

static void io_handler_write_tac_register(IoRegisterFuncContext context, uint8_t value)
{
    struct Timer *timer = context;
    timer->stopped = (value & (1 << 3)) == 0;
    switch (value & 0x03)
    {
    case 0:
        timer->cyclesPerTick = TIMER_INPUT_CLOCK_CYCLES_4096HZ;
        break;
    case 1:
        timer->cyclesPerTick = TIMER_INPUT_CLOCK_CYCLES_262144HZ;
        break;
    case 2:
        timer->cyclesPerTick = TIMER_INPUT_CLOCK_CYCLES_65536HZ;
        break;
    case 3:
    default:
        timer->cyclesPerTick = TIMER_INPUT_CLOCK_CYCLES_16384HZ;
        break;
    }
}


void timer_init(struct Timer *timer, struct Memory *memory)
{
    timer->stopped = true;
    timer->div = 0;
    timer->divIncrementCyclesLeft = DIV_INCREMENT_CYCLES;
    timer->tma = 0;
    timer->cyclesPerTick = TIMER_INPUT_CLOCK_CYCLES_4096HZ;
    timer->tima = 0;
    timer->timaIncrementCyclesLeft = timer->cyclesPerTick;
    memory_register_io_handler(
        memory,
        IO_REGISTER_DIV,
        io_handler_read_div_register,
        io_handler_write_div_register,
        timer
    );
    memory_register_io_handler(
        memory,
        IO_REGISTER_TIMA,
        io_handler_read_tima_register,
        io_handler_write_tima_register,
        timer
    );
    memory_register_io_handler(
        memory,
        IO_REGISTER_TMA,
        io_handler_read_tma_register,
        io_handler_write_tma_register,
        timer
    );
    memory_register_io_handler(
        memory,
        IO_REGISTER_TAC,
        io_handler_read_tac_register,
        io_handler_write_tac_register,
        timer
    );
}


void timer_tick(struct Timer *timer, struct Cpu *cpu, int cycles)
{
    timer->divIncrementCyclesLeft -= cycles;
    if (timer->divIncrementCyclesLeft <= 0)
    {
        timer->div += 1;
        timer->divIncrementCyclesLeft += DIV_INCREMENT_CYCLES;
    }

    if ( ! timer->stopped)
    {
        timer->timaIncrementCyclesLeft -= cycles;
    }
    if (timer->timaIncrementCyclesLeft <= 0)
    {
        if (timer->tima == 0xff)
        {
            timer->tima = timer->tma;
            cpu_request_interrupt(cpu, INTERRUPT_TIMER);
        }
        else
        {
            timer->tima += 1;
        }
    }
}

