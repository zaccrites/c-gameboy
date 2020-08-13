
#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>
#include <stdbool.h>

struct Memory;
struct Cpu;


struct Serial
{
    // Transient values
    uint8_t outgoingData;
    uint8_t incomingData;

    // Complete transfer bytes
    uint8_t receivedData;
    uint8_t dataToSend;

    bool startTransfer;
    bool internalClockSelect;
    bool transferInProgress;
    int transferStepCyclesRemaining;
    int transferStepsRemaining;
};


void serial_init(struct Serial *serial, struct Memory *memory);
bool serial_tick(struct Serial *serial, struct Cpu *cpu, int cycles);



#endif
