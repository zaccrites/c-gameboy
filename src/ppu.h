
#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <stdbool.h>


struct Memory;
struct Cpu;


enum Color
{
    COLOR_LIGHTEST = 0,
    COLOR_LIGHTER = 1,
    COLOR_DARKER = 2,
    COLOR_DARKEST = 3,
};


enum PpuMode
{
    PPU_MODE_HBLANK = 0,
    PPU_MODE_VBLANK = 1,
    PPU_MODE_OAM_SEARCH = 2,
    PPU_MODE_DRAWING = 3,
};


struct Ppu
{
    int cycleCounter;
    enum PpuMode mode;

    uint8_t currentLine;
    uint8_t currentLineCompare;
    uint8_t scrollX;
    uint8_t scrollY;
    uint8_t windowX;
    uint8_t windowY;

    struct
    {
        bool oamSearch;
        bool hBlank;
        bool vBlank;
        bool lyc;
    } stat;

    bool lcdEnable;
    bool windowTileMapSelect;
    bool windowDisplayEnable;
    bool backgroundAndWindowTileDataSelect;
    bool backgroundTileMapSelect;
    bool objectSize;
    bool objectDisplayEnable;
    bool backgroundDisplayEnable;

    enum Color backgroundPalette[4];
    enum Color objectPalette0[3];
    enum Color objectPalette1[3];

    struct Memory *memory;
};


void ppu_init(struct Ppu *ppu, struct Memory *memory);
bool ppu_tick(struct Ppu *ppu, struct Cpu *cpu, int cycles, uint8_t *pixelBuffer);


#endif
