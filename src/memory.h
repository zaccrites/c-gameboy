
#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stdbool.h>

#include "cartridge.h"


// In cartridge, fixed
#define MEMORY_ROM_BANK0_START     0x0000
#define MEMORY_ROM_BANK0_SIZE      0x4000
#define MEMORY_ROM_BANK0_END       (MEMORY_ROM_BANK0_START + MEMORY_ROM_BANK0_SIZE - 1)
// In cartridge, switchable
#define MEMORY_ROM_BANK1_START     0x4000
#define MEMORY_ROM_BANK1_SIZE      0x4000
#define MEMORY_ROM_BANK1_END       (MEMORY_ROM_BANK1_START + MEMORY_ROM_BANK1_SIZE)
// Switchable bank 0-1 in CGB mode
#define MEMORY_VRAM_START          0x8000
#define MEMORY_VRAM_SIZE           0x2000
#define MEMORY_VRAM_END            (MEMORY_VRAM_START + MEMORY_VRAM_SIZE - 1)
// In cartridge, switchable (if any)
#define MEMORY_EXTERNAL_RAM_START  0xa000
#define MEMORY_EXTERNAL_RAM_SIZE   0x2000
#define MEMORY_EXTERNAL_RAM_END    (MEMORY_EXTERNAL_RAM_START + MEMORY_EXTERNAL_RAM_SIZE - 1)
// Internal RAM, fixed
#define MEMORY_WRAM_BANK0_START    0xc000
#define MEMORY_WRAM_BANK0_SIZE     0x1000
#define MEMORY_WRAM_BANK0_END      (MEMORY_WRAM_BANK0_START + MEMORY_WRAM_BANK0_SIZE - 1)
// Internal RAM, switchable bank 1-7 in CGB mode
#define MEMORY_WRAM_BANK1_START    0xd000
#define MEMORY_WRAM_BANK1_SIZE     0x1000
#define MEMORY_WRAM_BANK1_END      (MEMORY_WRAM_BANK1_START + MEMORY_WRAM_BANK1_SIZE - 1)
// Echo of work RAM
#define MEMORY_WRAM_ECHO_START     0xe000
#define MEMORY_WRAM_ECHO_END       0xfdff
// Object Attribute Memory
#define MEMORY_OAM_START           0xfe00
#define MEMORY_OAM_SIZE            160
#define MEMORY_OAM_END             (MEMORY_OAM_START + MEMORY_OAM_SIZE - 1)
// Unusable
#define MEMORY_NOT_USABLE_START    0xfea0
#define MEMORY_NOT_USABLE_END      0xfeff
// I/O Ports
#define MEMORY_IO_START            0xff00
#define MEMORY_IO_SIZE             128
#define MEMORY_IO_END              (MEMORY_IO_START + MEMORY_IO_SIZE - 1)
// High RAM
#define MEMORY_HIGH_RAM_START      0xff80
#define MEMORY_HIGH_RAM_SIZE       127
#define MEMORY_HIGH_RAM_END        (MEMORY_HIGH_RAM_START + MEMORY_HIGH_RAM_SIZE - 1)
//
#define MEMORY_INTERRUPT_ENABLE_REGISTER_ADDRESS  0xffff


// Allow other systems to register callback functions on read or write
// of IO registers, along with the necessary context (e.g. a Cpu* or Ppu*).
typedef void *IoRegisterFuncContext;
typedef uint8_t (*IoRegisterReadFunc)(IoRegisterFuncContext);
typedef void (*IoRegisterWriteFunc)(IoRegisterFuncContext, uint8_t);

struct IoRegisterHandler
{
    IoRegisterReadFunc read;
    IoRegisterWriteFunc write;
    IoRegisterFuncContext context;
};

struct Memory
{
    uint8_t romBank0[MEMORY_ROM_BANK0_SIZE];
    uint8_t romBank1[MEMORY_ROM_BANK1_SIZE];
    uint8_t vram[MEMORY_VRAM_SIZE];
    uint8_t externalRam[MEMORY_EXTERNAL_RAM_SIZE];
    uint8_t wramBank0[MEMORY_WRAM_BANK0_SIZE];
    uint8_t wramBank1[MEMORY_WRAM_BANK1_SIZE];
    uint8_t oam[MEMORY_OAM_SIZE];
    uint8_t highRam[MEMORY_HIGH_RAM_SIZE];
    uint8_t interruptEnableRegister;

    struct IoRegisterHandler ioRegisterHandlers[MEMORY_IO_SIZE];
    struct IoRegisterHandler interruptEnableRegisterHandler;

    // IoRegisterReadFunc ioRegisterReadFuncs[MEMORY_IO_SIZE];
    // IoRegisterWriteFunc ioRegisterWriteFuncs[MEMORY_IO_SIZE];
    // IoRegisterFuncContext ioRegisterFuncContexts[MEMORY_IO_SIZE];
};


uint8_t memory_read_word(struct Memory *memory, uint16_t address);
uint16_t memory_read_dword(struct Memory *memory, uint16_t address);
void memory_write_word(struct Memory *memory, uint16_t address, uint8_t value);
void memory_write_dword(struct Memory *memory, uint16_t address, uint16_t value);

bool memory_init(struct Memory *memory, struct Cartridge *cartridge);
void memory_teardown(struct Memory *memory);
void memory_register_io_handler(struct Memory *memory, uint8_t reg, IoRegisterReadFunc readfunc, IoRegisterWriteFunc writefunc, IoRegisterFuncContext context);

#endif
