
#include <assert.h>  // for assert
#include <stddef.h>  // for NULL

#include "memory.h"


static uint8_t* decode_raw_address(struct Memory *memory, uint16_t address)
{
    if (address <= MEMORY_ROM_BANK0_END)
    {
        return &memory->romBank0[address - MEMORY_ROM_BANK0_START];
    }
    else if (address <= MEMORY_ROM_BANK1_END)
    {
        return &memory->romBank1[address - MEMORY_ROM_BANK1_START];
    }
    else if (address <= MEMORY_VRAM_END)
    {
        return &memory->vram[address - MEMORY_VRAM_START];
    }
    else if (address <= MEMORY_EXTERNAL_RAM_END)
    {
        return &memory->externalRam[address - MEMORY_EXTERNAL_RAM_START];
    }
    else if (address <= MEMORY_WRAM_BANK0_END)
    {
        return &memory->wramBank0[address - MEMORY_WRAM_BANK0_START];
    }
    else if (address <= MEMORY_WRAM_BANK1_END)
    {
        return &memory->wramBank1[address - MEMORY_WRAM_BANK1_START];
    }
    else if (address <= MEMORY_ECHO_END)
    {
        // Emulate work ram echo if needed
        assert(false);
        return NULL;
    }
    else if (address <= MEMORY_OAM_END)
    {
        return &memory->oam[address - MEMORY_OAM_START];
    }
    else if (address <= MEMORY_IO_END)
    {
        // TODO
        assert(false);
        return NULL;
    }
    else if (address <= MEMORY_HIGH_RAM_END)
    {
        return &memory->highRam[address - MEMORY_HIGH_RAM_START];
    }
    else
    {
        assert(address == MEMORY_INTERRUPT_ENABLE_REGISTER_ADDRESS);
        return &memory->interruptEnableRegister;
    }
}


uint8_t memory_read_byte(struct Memory *memory, uint16_t address)
{
    return *decode_raw_address(memory, address);
}


uint16_t memory_read_word(struct Memory *memory, uint16_t address)
{
    uint16_t lowByte = memory_read_byte(memory, address);
    uint16_t highByte = memory_read_byte(memory, address + 1);
    return (highByte << 8) | lowByte;
}


void memory_write_byte(struct Memory *memory, uint16_t address, uint8_t value)
{
    *decode_raw_address(memory, address) = value;
}


void memory_write_word(struct Memory *memory, uint16_t address, uint16_t value)
{
    uint8_t lowByte = value & 0xff;
    uint8_t highByte = value << 8;
    memory_write_byte(memory, address, lowByte);
    memory_write_byte(memory, address + 1, highByte);
}


bool memory_init(struct Memory *memory, struct Cartridge *cartridge)
{
    // TODO: Allocate memory banks for memory mapper
}


void memory_teardown(struct Memory *memory)
{
    // TODO: Free dynamically allocated memory banks
}
