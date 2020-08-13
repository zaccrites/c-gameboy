
#include "serial.h"
#include "memory.h"
#include "cpu.h"


// When the internal clock is selected, bits are shifted in and out at 8192 Hz.
#define MACHINE_CYCLE_FREQUENCY      1000000  // 1 MHz
#define SERIAL_STEP_CYCLES_INTERNAL  (MACHINE_CYCLE_FREQUENCY / 8192)


#define IO_REGISTER_SB  0x01
static uint8_t io_handler_read_sb_register(IoRegisterFuncContext context)
{
    struct Serial *serial = context;
    return serial->receivedData;
}

static void io_handler_write_sb_register(IoRegisterFuncContext context, uint8_t value)
{
    struct Serial *serial = context;
    serial->dataToSend = value;
}


#define IO_REGISTER_SC  0x02
static uint8_t io_handler_read_sc_register(IoRegisterFuncContext context)
{
    struct Serial *serial = context;
    uint8_t value = 0;
    if (serial->startTransfer) { value |= (1 << 7); }
    if (serial->internalClockSelect) { value |= (1 << 0); }
    return value;
}

static void io_handler_write_sc_register(IoRegisterFuncContext context, uint8_t value)
{
    struct Serial *serial = context;
    serial->startTransfer = (value & (1 << 7)) != 0;
    serial->internalClockSelect = (value & (1 << 0)) != 0;
}


void serial_init(struct Serial *serial, struct Memory *memory)
{
    serial->outgoingData = 0x00;
    serial->incomingData = 0x00;
    serial->receivedData = 0x00;
    serial->dataToSend = 0x00;

    serial->startTransfer = false;
    serial->internalClockSelect = false;
    serial->transferInProgress = false;
    serial->transferStepCyclesRemaining = 0;
    serial->transferStepsRemaining = 0;

    memory_register_io_handler(
        memory,
        IO_REGISTER_SB,
        io_handler_read_sb_register,
        io_handler_write_sb_register,
        serial
    );
    memory_register_io_handler(
        memory,
        IO_REGISTER_SC,
        io_handler_read_sc_register,
        io_handler_write_sc_register,
        serial
    );
}


bool serial_tick(struct Serial *serial, struct Cpu *cpu, int cycles)
{
    // FUTURE: Support a link partner, possibly as another emulator
    //   connected via TCP/IP. Currently only zeros are shifted in
    //   and there is no external clock signal.

    if (serial->startTransfer && ! serial->transferInProgress)
    {
        serial->transferInProgress = true;
        serial->transferStepsRemaining = 8;
        serial->transferStepCyclesRemaining = SERIAL_STEP_CYCLES_INTERNAL;
        serial->outgoingData = serial->dataToSend;
        serial->incomingData = 0x00;
    }

    bool transferComplete = false;
    if (serial->transferInProgress)
    {
        bool doTransferStep = false;
        if (serial->internalClockSelect)
        {
            serial->transferStepCyclesRemaining -= cycles;
            doTransferStep = serial->transferStepCyclesRemaining <= 0;
        }
        else
        {
            // External clock not supported
        }

        if (doTransferStep)
        {
            uint8_t incomingBit = 0;
            uint8_t outgoingBit = (serial->outgoingData & (1 << 7)) >> 7;

            serial->incomingData = (serial->incomingData << 1) | incomingBit;
            serial->outgoingData = (serial->outgoingData << 1) | outgoingBit;
            serial->transferStepCyclesRemaining += SERIAL_STEP_CYCLES_INTERNAL;
            serial->transferStepsRemaining -= 1;
        }

        if (serial->transferStepsRemaining <= 0)
        {
            cpu_request_interrupt(cpu, INTERRUPT_SERIAL);
            serial->transferInProgress = false;
            serial->startTransfer = false;
            transferComplete = true;
        }
    }
    return transferComplete;
}
