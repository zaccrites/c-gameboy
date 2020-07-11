
#include <assert.h>  // for assert
#include <stddef.h>  // for NULL
#include <stdio.h>

#include "memory.h"


uint8_t memory_read_word(struct Memory *memory, uint16_t address)
{
    if (address <= MEMORY_ROM_BANK0_END)
    {
        return memory->romBank0[address - MEMORY_ROM_BANK0_START];
    }
    else if (address <= MEMORY_ROM_BANK1_END)
    {
        return memory->romBank1[address - MEMORY_ROM_BANK1_START];
    }
    else if (address <= MEMORY_VRAM_END)
    {
        return memory->vram[address - MEMORY_VRAM_START];
    }
    else if (address <= MEMORY_EXTERNAL_RAM_END)
    {
        return memory->externalRam[address - MEMORY_EXTERNAL_RAM_START];
    }
    else if (address <= MEMORY_WRAM_BANK0_END)
    {
        return memory->wramBank0[address - MEMORY_WRAM_BANK0_START];
    }
    else if (address <= MEMORY_WRAM_BANK1_END)
    {
        return memory->wramBank1[address - MEMORY_WRAM_BANK1_START];
    }
    else if (address <= MEMORY_WRAM_ECHO_END)
    {
        return memory_read_word(memory, address - MEMORY_WRAM_ECHO_START + MEMORY_WRAM_BANK0_START);
    }
    else if (address <= MEMORY_OAM_END)
    {
        return memory->oam[address - MEMORY_OAM_START];
    }
    else if (address <= MEMORY_NOT_USABLE_END)
    {
        return 0x00;
    }
    else if (address <= MEMORY_IO_END)
    {
        struct IoRegisterHandler* handler = &memory->ioRegisterHandlers[address - MEMORY_IO_START];
        return (handler->read == NULL) ? 0xff : handler->read(handler->context);
    }
    else if (address <= MEMORY_HIGH_RAM_END)
    {
        return memory->highRam[address - MEMORY_HIGH_RAM_START];
    }
    else
    {
        assert(address == MEMORY_INTERRUPT_ENABLE_REGISTER_ADDRESS);
        struct IoRegisterHandler *handler = &memory->interruptEnableRegisterHandler;
        assert(handler->read != NULL);
        return handler->read(handler->context);
    }
}


void memory_write_word(struct Memory *memory, uint16_t address, uint8_t value)
{
    if (address <= MEMORY_ROM_BANK0_END)
    {
        memory->romBank0[address - MEMORY_ROM_BANK0_START] = value;
    }
    else if (address <= MEMORY_ROM_BANK1_END)
    {
        memory->romBank1[address - MEMORY_ROM_BANK1_START] = value;
    }
    else if (address <= MEMORY_VRAM_END)
    {
        memory->vram[address - MEMORY_VRAM_START] = value;
    }
    else if (address <= MEMORY_EXTERNAL_RAM_END)
    {
        memory->externalRam[address - MEMORY_EXTERNAL_RAM_START] = value;
    }
    else if (address <= MEMORY_WRAM_BANK0_END)
    {
        memory->wramBank0[address - MEMORY_WRAM_BANK0_START] = value;
    }
    else if (address <= MEMORY_WRAM_BANK1_END)
    {
        memory->wramBank1[address - MEMORY_WRAM_BANK1_START] = value;
    }
    else if (address <= MEMORY_WRAM_ECHO_END)
    {
        memory_write_word(memory, address - MEMORY_WRAM_ECHO_START + MEMORY_WRAM_BANK0_START, value);
    }
    else if (address <= MEMORY_OAM_END)
    {
        memory->oam[address - MEMORY_OAM_START] = value;
    }
    else if (address <= MEMORY_NOT_USABLE_END)
    {
        // Do nothing
    }
    else if (address <= MEMORY_IO_END)
    {
        struct IoRegisterHandler *handler = &memory->ioRegisterHandlers[address - MEMORY_IO_START];
        if (handler->write != NULL)
        {
            handler->write(handler->context, value);
        }
    }
    else if (address <= MEMORY_HIGH_RAM_END)
    {
        memory->highRam[address - MEMORY_HIGH_RAM_START] = value;
    }
    else
    {
        assert(address == MEMORY_INTERRUPT_ENABLE_REGISTER_ADDRESS);
        struct IoRegisterHandler *handler = &memory->interruptEnableRegisterHandler;
        assert(handler->write != NULL);
        handler->write(handler->context, value);
    }
}



uint16_t memory_read_dword(struct Memory *memory, uint16_t address)
{
    uint16_t lowByte = (uint16_t)memory_read_word(memory, address);
    uint16_t highByte = (uint16_t)memory_read_word(memory, address + 1);
    return (highByte << 8) | lowByte;
}


void memory_write_dword(struct Memory *memory, uint16_t address, uint16_t value)
{
    uint8_t lowByte = value & 0xff;
    uint8_t highByte = value >> 8;
    memory_write_word(memory, address, (uint16_t)lowByte);
    memory_write_word(memory, address + 1, (uint16_t)highByte);
}


bool memory_init(struct Memory *memory, struct Cartridge *cartridge)
{
    assert(cartridge->dataSize == MEMORY_ROM_BANK0_SIZE + MEMORY_ROM_BANK1_SIZE);
    memcpy(memory->romBank0, &cartridge->data[MEMORY_ROM_BANK0_START], MEMORY_ROM_BANK0_SIZE);
    memcpy(memory->romBank1, &cartridge->data[MEMORY_ROM_BANK1_START], MEMORY_ROM_BANK1_SIZE);

    for (size_t i = 0; i < MEMORY_IO_SIZE; i++)
    {
        memory->ioRegisterHandlers[i].context = NULL;
        memory->ioRegisterHandlers[i].read = NULL;
        memory->ioRegisterHandlers[i].write = NULL;
    }
    memory->interruptEnableRegisterHandler.context = NULL;
    memory->interruptEnableRegisterHandler.read = NULL;
    memory->interruptEnableRegisterHandler.write = NULL;

    return true;
}


void memory_teardown(struct Memory *memory)
{
    // Free any dynamically allocated banks
    (void)memory;
}


void memory_register_io_handler(struct Memory *memory, uint8_t reg, IoRegisterReadFunc readfunc, IoRegisterWriteFunc writefunc, IoRegisterFuncContext context)
{
    struct IoRegisterHandler *handler;
    if (reg == 0xff)
    {
        handler = &memory->interruptEnableRegisterHandler;
    }
    else
    {
        assert(reg < MEMORY_IO_SIZE);
        handler = &memory->ioRegisterHandlers[reg];
    }

    assert(handler->context == NULL);
    handler->read = readfunc;
    handler->write = writefunc;
    handler->context = context;
}

