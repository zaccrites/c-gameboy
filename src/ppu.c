
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#include "ppu.h"
#include "memory.h"
#include "lcd.h"
#include "cpu.h"


#define MAX_OBJECTS           40
#define MAX_OBJECTS_PER_LINE  10


struct Object
{
    size_t index;
    uint8_t y;
    uint8_t x;
    uint8_t patternIndex;
    bool priority;
    bool yFlip;
    bool xFlip;
    bool palette;
    // FUTURE: CGB-only flags
};

static int object_object_render_sorting(const void *rawObj1, const void *rawObj2)
{
    // Objects with lower X coords are drawn over objects with
    // larger X coords. Therefore we sort the array in descending
    // X coords so that the final sprite drawn will be the one with the
    // smallest X coord.
    //
    // Objects with the same X coord will be sorted in descending index
    // number in OAM so that the sprite with the lowest number is drawn
    // first in that case.

    const struct Object *obj1 = rawObj1;
    const struct Object *obj2 = rawObj2;

    if (obj2->x < obj1->x)
    {
        return -1;
    }
    else if (obj1->x < obj2->x)
    {
        return 1;
    }
    else
    {
        if (obj2->index < obj1->index)
        {
            return -1;
        }
        else
        {
            // We assume that the indices will never be the same.
            return 1;
        }
    }
}




#define OBJECT_SIZE_8x16  true
#define OBJECT_SIZE_8x8   false



// Find which objects are visible on this scan line
static size_t oam_search(struct Ppu *ppu, struct Object *objects)
{
    // Assume that objects is at least of length MAX_OBJECTS_PER_LINE
    size_t objectsFound = 0;

    for (size_t i = 0; i < MAX_OBJECTS && objectsFound < MAX_OBJECTS_PER_LINE; i++)
    {
        uint8_t *rawObject = &ppu->memory->oam[4 * i];
        int16_t y = rawObject[0];

        if (y != 0)
        {
            // printf("OBJECT %lu X=0x%02x, Y=0x%02x \n", i, rawObject[1], rawObject[0]);
        }

        int16_t currentLine = ppu->currentLine;

        // For e.g. 16px tall objects, the first visible line is LY=0 when Y=16.
        int16_t objectHeight = (ppu->objectSize == OBJECT_SIZE_8x16) ? 16 : 8;
        bool currentlyAbove = currentLine < y - objectHeight * 2;
        bool currentlyBelow = currentLine >= y - objectHeight;
        // bool currentlyAbove = currentLine <= y - objectHeight;
        // bool currentlyBelow = currentLine >= y;
        bool objectVisible = ! (currentlyAbove || currentlyBelow);

        if (objectVisible  && false)
        {
            printf("currentLine = %d, y = %d, y - objectHeight = %d | currentlyAbove? = %d, currentlyBelow? = %d \n",
                currentLine, y, y - objectHeight,
                currentlyAbove, currentlyBelow);
        }

        if (objectVisible)
        {
            struct Object object = {
                .index = i,
                .y = y,
                .x = rawObject[1],
                .patternIndex = rawObject[2],
                .priority = (rawObject[3] & (1 << 7)) != 0,
                .yFlip = (rawObject[3] & (1 << 6)) != 0,
                .xFlip = (rawObject[3] & (1 << 5)) != 0,
                .palette = (rawObject[3] & (1 << 4)) != 0,
            };
            objects[objectsFound++] = object;
        }
    }
    return objectsFound;
}


static enum Color get_background_palette_color(struct Ppu *ppu, uint8_t colorNumber)
{
    assert(colorNumber < 4);
    return ppu->backgroundPalette[colorNumber];
}

static bool get_object_palette_color(struct Ppu *ppu, bool paletteFlag, uint8_t colorNumber, enum Color *color)
{
    assert(colorNumber < 4);
    if (colorNumber == 0)
    {
        // All object pixels with color number 0 are transparent.
        return false;
    }
    enum Color *palette = paletteFlag ? ppu->objectPalette1 : ppu->objectPalette0;
    *color = palette[colorNumber - 1];
    return true;
}



static void get_color_rgb(enum Color color, uint8_t *r, uint8_t *g, uint8_t *b)
{
    switch (color)
    {
    case COLOR_LIGHTEST:
        *r = LCD_COLOR_LIGHTEST_R;
        *g = LCD_COLOR_LIGHTEST_G;
        *b = LCD_COLOR_LIGHTEST_B;
        break;
    case COLOR_LIGHTER:
        *r = LCD_COLOR_LIGHTER_R;
        *g = LCD_COLOR_LIGHTER_G;
        *b = LCD_COLOR_LIGHTER_B;
        break;
    case COLOR_DARKER:
        *r = LCD_COLOR_DARKER_R;
        *g = LCD_COLOR_DARKER_G;
        *b = LCD_COLOR_DARKER_B;
        break;
    case COLOR_DARKEST:
        *r = LCD_COLOR_DARKEST_R;
        *g = LCD_COLOR_DARKEST_G;
        *b = LCD_COLOR_DARKEST_B;
        break;
    default:
        assert(false);
    }
}


#define VRAM_TILE_PATTERN_TABLE0_INDEX  0x1000  // signed pattern 0
#define VRAM_TILE_PATTERN_TABLE1_INDEX  0x0000  // unsigned pattern 0

#define VRAM_TILE_BACKGROUND_MAP0_INDEX 0x1800
#define VRAM_TILE_BACKGROUND_MAP1_INDEX 0x1c00


#define BACKGROUND_TILE_WIDTH   8
#define BACKGROUND_TILE_HEIGHT  8
#define TILE_GRID_WIDTH         32
#define TILE_GRID_HEIGHT        32

static void ppu_render_line(struct Ppu *ppu, uint8_t *pixelBuffer)
{
    // TODO: window

    struct Object objects[MAX_OBJECTS_PER_LINE];
    size_t numObjectsOnLine = oam_search(ppu, objects);
    qsort(objects, numObjectsOnLine, sizeof(struct Object), object_object_render_sorting);

    // Find the effective Y-coordinate for this scan line.
    // From that, find the associated Y-coordinate for the tile grid,
    // as well as the pixel Y-coordinate for the tile for this scanline.
    uint8_t y0 = ppu->currentLine;
    uint8_t y = y0 + ppu->scrollY;
    size_t tileCoordY = y / BACKGROUND_TILE_HEIGHT;
    size_t tilePixelCoordY = y % BACKGROUND_TILE_HEIGHT;

    for (uint8_t x0 = 0; x0 < LCD_WIDTH; x0++)
    {
        uint8_t backgroundColorNumber = 0;
        enum Color pixelColor = COLOR_LIGHTEST;

        if (ppu->backgroundDisplayEnable)
        {
            // Similarly, find the effective X-coordinate for this pixel, etc.
            uint8_t x = x0 + ppu->scrollX;
            size_t tileCoordX = x / BACKGROUND_TILE_WIDTH;
            size_t tilePixelCoordX = x % BACKGROUND_TILE_WIDTH;

            // Once we have the tile grid XY-coordinates,
            // we can identify the tile grid index for this pixel's tile
            // (starting with zero at the top-left,
            // increasing from left-to-right and top-to-bottom).
            size_t tileIndex = (tileCoordY * TILE_GRID_WIDTH) + tileCoordX;

            // Select the desired tile map.
            uint8_t *tileMap = &ppu->memory->vram[
                ppu->backgroundTileMapSelect
                    ? VRAM_TILE_BACKGROUND_MAP1_INDEX
                    : VRAM_TILE_BACKGROUND_MAP0_INDEX
            ];

            // Find the tile pattern of interest
            uint8_t *tilePattern;
            if (ppu->backgroundAndWindowTileDataSelect)
            {
                uint8_t *tilePatternTable = &ppu->memory->vram[VRAM_TILE_PATTERN_TABLE1_INDEX];
                uint8_t tilePatternNumber = tileMap[tileIndex];
                tilePattern = &tilePatternTable[tilePatternNumber * (2 * BACKGROUND_TILE_WIDTH)];
            }
            else
            {
                uint8_t *tilePatternTable = &ppu->memory->vram[VRAM_TILE_PATTERN_TABLE0_INDEX];
                int8_t tilePatternNumber = ((int8_t*)tileMap)[tileIndex];
                tilePattern = &tilePatternTable[tilePatternNumber * (2 * BACKGROUND_TILE_WIDTH)];
            }

            // Get the tile pattern line of interest,
            // and the color number of the pixel of interest.
            uint8_t *tilePatternLine = tilePattern + 2 * tilePixelCoordY;
            backgroundColorNumber = (
                ((tilePatternLine[1] & (1 << (7 - tilePixelCoordX))) << 1) |
                (tilePatternLine[0] & (1 << (7 - tilePixelCoordX)))
            ) >> (7 - tilePixelCoordX);
            pixelColor = get_background_palette_color(ppu, backgroundColorNumber);
        }

        // TODO: clean up sprite code
        // TODO: 8x16 mode tile selection

        // Now look at the objects for this pixel
        if (ppu->objectDisplayEnable)
        {
            for (size_t i = 0; i < numObjectsOnLine; i++)
            {
                // We already know that the object is visible.
                // The question is which object pixel aligns with the screen pixel
                // we're looking at now, if any.

                int16_t currentLine = ppu->currentLine;
                int16_t objectWidth = 8;  // constant
                int16_t objectHeight = (ppu->objectSize == OBJECT_SIZE_8x16) ? 16 : 8;
                // bool currentlyAbove = currentLine <= y - objectHeight;
                // bool currentlyBelow = currentLine >= y;

                const struct Object *obj = &objects[i];
                int16_t objectPixelY = (currentLine - obj->y) + 2*objectHeight;
                int16_t objectPixelX = ((int16_t)x0 - obj->x) + objectWidth;
                if (objectPixelX >= 0 && objectPixelX < 8)
                {
                    size_t tilePatternIndex = obj->patternIndex;
                    uint8_t *tilePixelData = &ppu->memory->vram[16 * tilePatternIndex];  // TODO: may need to change this for 8x16 sprites
                    uint8_t *tileLinePixelData = tilePixelData + 2 * (uint8_t)(obj->yFlip ? (7 - objectPixelY) : objectPixelY);  // TODO: may need to change this for 8x16 sprites
                    uint8_t tileByte0 = tileLinePixelData[0];
                    uint8_t tileByte1 = tileLinePixelData[1];

                    // TODO: clean up, combine with BG version of this
                    uint8_t xShift = obj->xFlip ? objectPixelX : (7 - objectPixelX);
                    uint8_t objectColorNumber = (
                            ((tileByte1 & (1 << xShift)) << 1) |
                            (tileByte0 & (1 << xShift))
                    ) >> xShift;

                    enum Color objectPixelColor;
                    bool isNotTransparent = get_object_palette_color(ppu, obj->palette, objectColorNumber, &objectPixelColor);
                    if (isNotTransparent && ( ! obj->priority || backgroundColorNumber == 0))
                    {
                        // The "priority" flag is kind of an inversion--
                        // if it is set, the object's will only draw on top
                        // of background pixels of color number 0.
                        pixelColor = objectPixelColor;
                    }
                }
            }
        }

        uint8_t r, g, b;
        get_color_rgb(pixelColor, &r, &g, &b);

        size_t i = y0 * LCD_WIDTH + x0;
        pixelBuffer[4 * i + 0] = b;
        pixelBuffer[4 * i + 1] = g;
        pixelBuffer[4 * i + 2] = r;
        pixelBuffer[4 * i + 3] = 0xff;
    }
}



#define CYCLES_MODE_OAM_SEARCH  20
#define CYCLES_MODE_DRAWING     43
#define CYCLES_MODE_HBLANK      51
#define CYCLES_PER_LINE         (CYCLES_MODE_OAM_SEARCH + CYCLES_MODE_DRAWING + CYCLES_MODE_HBLANK)

#define NUM_VBLANK_LINES        10


bool ppu_tick(struct Ppu *ppu, struct Cpu *cpu, int cycles, uint8_t *pixelBuffer)
{
    // TODO: LCD disabled? Would have to blank the whole display and reset
    // the line/cycle counter and PPU state machine.

    bool enteringVBlank = false;

    // Advance the line or pixel count, update VRAM/OAM lock state, etc.
    ppu->cycleCounter += cycles;
    switch (ppu->mode)
    {
    case PPU_MODE_OAM_SEARCH:
        if (ppu->cycleCounter >= CYCLES_MODE_OAM_SEARCH)
        {
            ppu->cycleCounter = 0;
            ppu->mode = PPU_MODE_DRAWING;
            // TODO: VRAM/OAM lock
        }
        break;
    case PPU_MODE_DRAWING:
        if (ppu->cycleCounter >= CYCLES_MODE_DRAWING)
        {
            ppu->cycleCounter = 0;
            ppu->mode = PPU_MODE_HBLANK;
            // TODO: VRAM/OAM lock
        }
        break;
    case PPU_MODE_HBLANK:
        if (ppu->cycleCounter >= CYCLES_MODE_HBLANK)
        {
            ppu->cycleCounter = 0;
            ppu_render_line(ppu, pixelBuffer);
            ppu->currentLine += 1;
            if (ppu->currentLine >= LCD_HEIGHT)
            {
                ppu->mode = PPU_MODE_VBLANK;
                cpu_request_interrupt(cpu, INTERRUPT_VBLANK);
                enteringVBlank = true;
            }
            else
            {
                ppu->mode = PPU_MODE_OAM_SEARCH;
            }
        }
        break;
    case PPU_MODE_VBLANK:
        if (ppu->cycleCounter > CYCLES_PER_LINE)
        {
            ppu->cycleCounter = 0;
            ppu->currentLine += 1;
            if (ppu->currentLine >= LCD_HEIGHT + NUM_VBLANK_LINES)
            {
                ppu->currentLine = 0;
                ppu->mode = PPU_MODE_OAM_SEARCH;
            }
        }
        break;
    default:
        assert(false);
        break;
    }

    return enteringVBlank;
}


#define IO_REGISTER_LCD_CONTROL  0x40
static uint8_t io_handler_read_lcd_control(IoRegisterFuncContext context)
{
    struct Ppu *ppu = context;
    uint8_t value = 0;
    if (ppu->lcdEnable) { value |= (1 << 7); }
    if (ppu->windowTileMapSelect) { value |= (1 << 6); }
    if (ppu->windowDisplayEnable) { value |= (1 << 5); }
    if (ppu->backgroundAndWindowTileDataSelect) { value |= (1 << 4); }
    if (ppu->backgroundTileMapSelect) { value |= (1 << 3); }
    if (ppu->objectSize) { value |= (1 << 2); }
    if (ppu->objectDisplayEnable) { value |= (1 << 1); }
    if (ppu->backgroundDisplayEnable) { value |= (1 << 0); }
    return value;
}
static void io_handler_write_lcd_control(IoRegisterFuncContext context, uint8_t value)
{
    struct Ppu *ppu = context;
    ppu->lcdEnable = (value & (1 << 7)) != 0;
    ppu->windowTileMapSelect = (value & (1 << 6)) != 0;
    ppu->windowDisplayEnable = (value & (1 << 5)) != 0;
    ppu->backgroundAndWindowTileDataSelect = (value & (1 << 4)) != 0;
    ppu->backgroundTileMapSelect = (value & (1 << 3)) != 0;
    ppu->objectSize = (value & (1 << 2)) != 0;
    ppu->objectDisplayEnable = (value & (1 << 1)) != 0;
    ppu->backgroundDisplayEnable = (value & (1 << 0)) != 0;
}


#define IO_REGISTER_LCDC_STATUS  0x41
static uint8_t io_handler_read_lcdc_status(IoRegisterFuncContext context)
{
    // TODO
    struct Ppu *ppu = context;
    (void)ppu;
    return 0;
}
static void io_handler_write_lcdc_status(IoRegisterFuncContext context, uint8_t value)
{
    // TODO
    struct Ppu *ppu = context;
    (void)value;
    (void)ppu;
}


#define IO_REGISTER_SCROLL_Y  0x42
static uint8_t io_handler_read_scroll_y(IoRegisterFuncContext context)
{
    struct Ppu *ppu = context;
    return ppu->scrollY;
}
static void io_handler_write_scroll_y(IoRegisterFuncContext context, uint8_t value)
{
    struct Ppu *ppu = context;
    ppu->scrollY = value;
}

#define IO_REGISTER_SCROLL_X  0x43
static uint8_t io_handler_read_scroll_x(IoRegisterFuncContext context)
{
    struct Ppu *ppu = context;
    return ppu->scrollX;
}
static void io_handler_write_scroll_x(IoRegisterFuncContext context, uint8_t value)
{
    struct Ppu *ppu = context;
    ppu->scrollX = value;
}

#define IO_REGISTER_LCDC_YCOORD 0x44
static uint8_t io_handler_read_ly(IoRegisterFuncContext context)
{
    struct Ppu *ppu = context;
    return ppu->currentLine;
}
static void io_handler_write_ly(IoRegisterFuncContext context, uint8_t value)
{
    (void)value;
    struct Ppu *ppu = context;
    ppu->currentLine = 0;
    ppu->cycleCounter = 0;
    ppu->mode = PPU_MODE_OAM_SEARCH;
}

#define IO_REGISTER_LY_COMPARE  0x45
static uint8_t io_handler_read_lyc(IoRegisterFuncContext context)
{
    struct Ppu *ppu = context;
    return ppu->currentLineCompare;
}
static void io_handler_write_lyc(IoRegisterFuncContext context, uint8_t value)
{
    struct Ppu *ppu = context;
    ppu->currentLineCompare = value;
}


#define IO_REGISTER_BACKGROUND_PALETTE  0x47
static uint8_t io_handler_read_background_palette(IoRegisterFuncContext context)
{
    struct Ppu *ppu = context;
    uint8_t value = 0;
    value |= (uint8_t)ppu->backgroundPalette[3] << 6;
    value |= (uint8_t)ppu->backgroundPalette[2] << 4;
    value |= (uint8_t)ppu->backgroundPalette[1] << 2;
    value |= (uint8_t)ppu->backgroundPalette[0] << 0;
    return value;
}
static void io_handler_write_background_palette(IoRegisterFuncContext context, uint8_t value)
{
    struct Ppu *ppu = context;
    ppu->backgroundPalette[3] = (enum Color)((value >> 6) & 0x03);
    ppu->backgroundPalette[2] = (enum Color)((value >> 4) & 0x03);
    ppu->backgroundPalette[1] = (enum Color)((value >> 2) & 0x03);
    ppu->backgroundPalette[0] = (enum Color)((value >> 0) & 0x03);
}


#define IO_REGISTER_OBJECT_PALETTE0  0x48
static uint8_t io_handler_read_object_palette0(IoRegisterFuncContext context)
{
    struct Ppu *ppu = context;
    uint8_t value = 0;
    value |= (uint8_t)ppu->objectPalette0[2] << 6;
    value |= (uint8_t)ppu->objectPalette0[1] << 4;
    value |= (uint8_t)ppu->objectPalette0[0] << 2;
    // lowest two bits are unused (transparent)
    return value;
}
static void io_handler_write_object_palette0(IoRegisterFuncContext context, uint8_t value)
{
    struct Ppu *ppu = context;
    ppu->objectPalette0[2] = (enum Color)((value >> 6) & 0x03);
    ppu->objectPalette0[1] = (enum Color)((value >> 4) & 0x03);
    ppu->objectPalette0[0] = (enum Color)((value >> 2) & 0x03);
    // lowest two bits are unused (transparent)
}


#define IO_REGISTER_OBJECT_PALETTE1  0x49
static uint8_t io_handler_read_object_palette1(IoRegisterFuncContext context)
{
    struct Ppu *ppu = context;
    uint8_t value = 0;
    value |= (uint8_t)ppu->objectPalette1[2] << 6;
    value |= (uint8_t)ppu->objectPalette1[1] << 4;
    value |= (uint8_t)ppu->objectPalette1[0] << 2;
    // lowest two bits are unused (transparent)
    return value;
}
static void io_handler_write_object_palette1(IoRegisterFuncContext context, uint8_t value)
{
    struct Ppu *ppu = context;
    ppu->objectPalette1[2] = (enum Color)((value >> 6) & 0x03);
    ppu->objectPalette1[1] = (enum Color)((value >> 4) & 0x03);
    ppu->objectPalette1[0] = (enum Color)((value >> 2) & 0x03);
    // lowest two bits are unused (transparent)
}


#define IO_REGISTER_WINDOW_Y  0x4a
static uint8_t io_handler_read_window_y(IoRegisterFuncContext context)
{
    struct Ppu *ppu = context;
    return ppu->windowY;
}
static void io_handler_write_window_y(IoRegisterFuncContext context, uint8_t value)
{
    struct Ppu *ppu = context;
    ppu->windowY = value;
}

#define IO_REGISTER_WINDOW_X  0x4b
static uint8_t io_handler_read_window_x(IoRegisterFuncContext context)
{
    struct Ppu *ppu = context;
    return ppu->windowX;
}
static void io_handler_write_window_x(IoRegisterFuncContext context, uint8_t value)
{
    struct Ppu *ppu = context;
    ppu->windowX = value;
}



void ppu_init(struct Ppu *ppu, struct Memory *memory)
{
    ppu->cycleCounter = 0;
    ppu->mode = PPU_MODE_OAM_SEARCH;

    ppu->currentLine = 0;
    ppu->scrollX = 0;
    ppu->scrollY = 0;
    ppu->windowX = 0;
    ppu->windowY = 0;

    ppu->lcdEnable = true;
    ppu->windowTileMapSelect = false;
    ppu->windowDisplayEnable = false;
    ppu->backgroundAndWindowTileDataSelect = true;
    ppu->backgroundTileMapSelect = false;
    ppu->objectSize = false;
    ppu->objectDisplayEnable = false;
    ppu->backgroundDisplayEnable = true;

    ppu->memory = memory;
    memory_register_io_handler(
        ppu->memory,
        IO_REGISTER_LCD_CONTROL,
        io_handler_read_lcd_control,
        io_handler_write_lcd_control,
        ppu
    );
    memory_register_io_handler(
        ppu->memory,
        IO_REGISTER_LCDC_STATUS,
        io_handler_read_lcdc_status,
        io_handler_write_lcdc_status,
        ppu
    );
    memory_register_io_handler(
        ppu->memory,
        IO_REGISTER_SCROLL_X,
        io_handler_read_scroll_x,
        io_handler_write_scroll_x,
        ppu
    );
    memory_register_io_handler(
        ppu->memory,
        IO_REGISTER_SCROLL_Y,
        io_handler_read_scroll_y,
        io_handler_write_scroll_y,
        ppu
    );
    memory_register_io_handler(
        ppu->memory,
        IO_REGISTER_LCDC_YCOORD,
        io_handler_read_ly,
        io_handler_write_ly,
        ppu
    );
    memory_register_io_handler(
        ppu->memory,
        IO_REGISTER_LY_COMPARE,
        io_handler_read_lyc,
        io_handler_write_lyc,
        ppu
    );
    memory_register_io_handler(
        ppu->memory,
        IO_REGISTER_BACKGROUND_PALETTE,
        io_handler_read_background_palette,
        io_handler_write_background_palette,
        ppu
    );
    memory_register_io_handler(
        ppu->memory,
        IO_REGISTER_OBJECT_PALETTE0,
        io_handler_read_object_palette0,
        io_handler_write_object_palette0,
        ppu
    );
    memory_register_io_handler(
        ppu->memory,
        IO_REGISTER_OBJECT_PALETTE1,
        io_handler_read_object_palette1,
        io_handler_write_object_palette1,
        ppu
    );
    memory_register_io_handler(
        ppu->memory,
        IO_REGISTER_WINDOW_X,
        io_handler_read_window_x,
        io_handler_write_window_x,
        ppu
    );
    memory_register_io_handler(
        ppu->memory,
        IO_REGISTER_WINDOW_Y,
        io_handler_read_window_y,
        io_handler_write_window_y,
        ppu
    );
}
