
#include <assert.h>

#include "ppu.h"
#include "memory.h"
#include "lcd.h"
#include "cpu.h"


#define MAX_OBJECTS_PER_LINE  10

struct Object
{
    int x;
};

// // Find which objects are visible on this scan line
// static size_t oam_search(struct Ppu *ppu, struct Object *objects)
// {
//     // Assume that objects is at least of length MAX_OBJECTS_PER_LINE

//     // TODO
//     (void)ppu;
//     (void)objects;
//     return 0;
// }


// // Draw a single line
// void ppu_render_scanline(struct Ppu *ppu, uint8_t *buffer)
// {

// }






enum PpuMode ppu_get_mode(struct Ppu *ppu)
{
    if (ppu->currentLine >= LCD_HEIGHT)
    {
        return PPU_MODE_VBLANK;
    }

    // TODO
    return PPU_MODE_DRAWING;

}

#include <stdio.h>

#define VRAM_TILE_DATA_INDEX            0
#define VRAM_TILE_PATTERN_TABLE0_INDEX  VRAM_TILE_DATA_INDEX
#define VRAM_TILE_PATTERN_TABLE1_INDEX  (VRAM_TILE_DATA_INDEX + 0x0800)

#define VRAM_TILE_BACKGROUND_MAP0_INDEX 0x1800
#define VRAM_TILE_BACKGROUND_MAP1_INDEX 0x1c00


void ppu_render_vram(struct Ppu *ppu, uint8_t *buffer)
{
    // TODO: Render line by line properly, not all at once.
    for (uint8_t y0 = 0; y0 < LCD_HEIGHT; y0++)
    {
        uint8_t y = y0 + ppu->scrollY;
        size_t tileCoordY = y / 8;
        size_t tilePixelCoordY = y % 8;

        for (uint8_t x0 = 0; x0 < LCD_WIDTH; x0++)
        {
            uint8_t x = x0 + ppu->scrollX;
            size_t tileCoordX = x / 8;
            size_t tilePixelCoordX = x % 8;

            size_t tileIndex = (tileCoordY * 32) + tileCoordX;
            size_t tilePatternIndex = ppu->memory->vram[VRAM_TILE_BACKGROUND_MAP0_INDEX + tileIndex];  // TODO: other tile map
            uint8_t *tilePixelData = &ppu->memory->vram[16 * tilePatternIndex];
            uint8_t *tileLinePixelData = tilePixelData + 2 * tilePixelCoordY;
            uint8_t tileByte0 = tileLinePixelData[0];
            uint8_t tileByte1 = tileLinePixelData[1];

            // TODO: convert raw number into *chosen pallete color*
            enum Color pixelColor = (enum Color)(
                (
                    ((tileByte0 & (1 << (7 - tilePixelCoordX))) << 1) |
                    (tileByte1 & (1 << (7 - tilePixelCoordX)))
                ) >> (7 - tilePixelCoordX)
            );

            uint8_t r;
            uint8_t g;
            uint8_t b;
            switch (pixelColor)
            {
            case COLOR_LIGHTEST:
                r = LCD_COLOR_LIGHTEST_R;
                g = LCD_COLOR_LIGHTEST_G;
                b = LCD_COLOR_LIGHTEST_B;
                break;
            case COLOR_LIGHTER:
                r = LCD_COLOR_LIGHTER_R;
                g = LCD_COLOR_LIGHTER_G;
                b = LCD_COLOR_LIGHTER_B;
                break;
            case COLOR_DARKER:
                r = LCD_COLOR_DARKER_R;
                g = LCD_COLOR_DARKER_G;
                b = LCD_COLOR_DARKER_B;
                break;
            case COLOR_DARKEST:
                r = LCD_COLOR_DARKEST_R;
                g = LCD_COLOR_DARKEST_G;
                b = LCD_COLOR_DARKEST_B;
                break;
            default:
                printf("uh oh, color is 0x%x \n", pixelColor);
                assert(false);
            }

            size_t i = y0 * LCD_WIDTH + x0;
            buffer[4 * i + 0] = b;
            buffer[4 * i + 1] = g;
            buffer[4 * i + 2] = r;
            buffer[4 * i + 3] = 0xff;

        }
    }
}




void ppu_tick(struct Ppu *ppu, struct Cpu *cpu, int cycles)
{
    // Advance the line or pixel count, update VRAM/OAM lock state, etc.
    ppu->cycleCounter += cycles;

    // TODO: more accurate timing
    // for now just advance the current line every 456 clocks
    if (ppu->cycleCounter >= 456)
    {
        ppu->currentLine += 1;
        if (ppu->currentLine > 153)
        {
            ppu->currentLine = 0;
        }
        else if (ppu->currentLine > LCD_HEIGHT)
        {
            cpu_request_interrupt(cpu, INTERRUPT_VBLANK);
            // printf("VBLANK! \n");
        }
        ppu->cycleCounter = 0;
    }
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
    value |= (uint8_t)ppu->backgroundPalette[3] << 4;
    value |= (uint8_t)ppu->backgroundPalette[3] << 2;
    value |= (uint8_t)ppu->backgroundPalette[3] << 0;
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
