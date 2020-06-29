
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


void cartridge_get_header(const struct Cartridge *cartridge, struct CartridgeHeader *header)
{
    memcpy(header->title, &cartridge->data[0x0134], CARTRIDGE_HEADER_TITLE_LENGTH);
    header->title[CARTRIDGE_HEADER_TITLE_LENGTH] = '\0';
    memcpy(header->manufacturerCode, &cartridge->data[0x013f], CARTRIDGE_HEADER_MANUFACTURER_CODE_LENGTH);
    header->manufacturerCode[CARTRIDGE_HEADER_MANUFACTURER_CODE_LENGTH] = '\0';
    header->cgbFlag = cartridge->data[0x0143];

    // TODO
}
