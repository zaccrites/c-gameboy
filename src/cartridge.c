
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "cartridge.h"


bool cartridge_load(struct Cartridge *cartridge, const char *romPath)
{
    FILE *file = fopen(romPath, "rb");
    if (file == NULL)
    {
        return false;
    }

    fseek(file, 0, SEEK_END);
    cartridge->dataSize = ftell(file);
    cartridge->data = malloc(cartridge->dataSize);
    if (cartridge->data == NULL)
    {
        return false;
    }

    rewind(file);
    size_t bytesRead = fread(cartridge->data, 1, cartridge->dataSize, file);
    assert(bytesRead == cartridge->dataSize);
    fclose(file);
    return true;
}


void cartridge_teardown(struct Cartridge *cartridge)
{
    if (cartridge->data != NULL)
    {
        free(cartridge->data);
    }
}



static uint16_t cartridge_get_type(uint8_t cartridgeTypeCode)
{
    uint16_t result;
    switch (cartridgeTypeCode)
    {
    case 0x00:
        result = CARTRIDGE_TYPE_ROM;
        break;
    case 0x01:
        result = CARTRIDGE_TYPE_ROM | CARTRIDGE_TYPE_MBC1;
        break;
    // case 0x13:
    //     result = CARTRIDGE_TYPE_ROM | CARTRIDGE_TYPE_MBC3 | CARTRIDGE_TYPE_RAM | CARTRIDGE_TYPE_BATTERY;
    //     break;
    default:
        // TODO: Better error handling for unsupported cartridge types
        fprintf(stderr, "Unsupported cartridge type code 0x%02x \n", cartridgeTypeCode);
        assert(false);
    }
    assert(result & CARTRIDGE_TYPE_ROM);
    return result;
}


void cartridge_get_type_string(const struct CartridgeHeader *header, char *buffer, size_t buflen)
{
    assert(header->type & CARTRIDGE_TYPE_ROM);
    strncpy(buffer, "ROM", buflen);
    size_t length = 3;

    if (header->type & CARTRIDGE_TYPE_MBC1)
    {
        strncpy(buffer + length, "+MBC1", buflen - length);
        length += 5;
    }
    else if (header->type & CARTRIDGE_TYPE_MBC2)
    {
        strncpy(buffer + length, "+MBC2", buflen - length);
        length += 5;
    }
    else if (header->type & CARTRIDGE_TYPE_MBC3)
    {
        strncpy(buffer + length, "+MBC3", buflen - length);
        length += 5;
    }
    else if (header->type & CARTRIDGE_TYPE_MBC5)
    {
        strncpy(buffer + length, "+MBC5", buflen - length);
        length += 5;
    }

    if (header->type & CARTRIDGE_TYPE_RAM)
    {
        strncpy(buffer + length, "+RAM", buflen - length);
        length += 4;
    }

    if (header->type & CARTRIDGE_TYPE_BATTERY)
    {
        strncpy(buffer + length, "+BATTERY", buflen - length);
        length += 8;
    }
}

void cartridge_get_header(const struct Cartridge *cartridge, struct CartridgeHeader *header)
{
    memcpy(header->title, &cartridge->data[0x0134], CARTRIDGE_HEADER_TITLE_LENGTH);
    header->title[CARTRIDGE_HEADER_TITLE_LENGTH] = '\0';
    memcpy(header->manufacturerCode, &cartridge->data[0x013f], CARTRIDGE_HEADER_MANUFACTURER_CODE_LENGTH);
    header->manufacturerCode[CARTRIDGE_HEADER_MANUFACTURER_CODE_LENGTH] = '\0';
    header->cgbFlag = cartridge->data[0x0143];
    header->type = cartridge_get_type(cartridge->data[0x0147]);

    // TODO
}
