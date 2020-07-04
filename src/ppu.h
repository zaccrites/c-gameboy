
#ifndef PPU_H
#define PPU_H

#include <stdint.h>
#include <stdbool.h>


struct Memory;


enum Color
{
    COLOR_LIGHTEST = 0,
    COLOR_LIGHTER = 1,
    COLOR_DARKER = 2,
    COLOR_DARKEST = 3,
};


struct Ppu
{
    int cycleCounter;

    uint8_t currentLine;
    uint8_t currentLineCompare;
    uint8_t scrollX;
    uint8_t scrollY;
    uint8_t windowX;
    uint8_t windowY;

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

void ppu_tick(struct Ppu *ppu, int cycles);



#endif
