
#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>


struct Cartridge
{
    uint8_t *data;
    size_t dataSize;
};


#define CARTRIDGE_HEADER_TITLE_LENGTH  11
#define CARTRIDGE_HEADER_MANUFACTURER_CODE_LENGTH  4

struct CartridgeHeader
{
    char title[CARTRIDGE_HEADER_TITLE_LENGTH + 1];
    char manufacturerCode[CARTRIDGE_HEADER_MANUFACTURER_CODE_LENGTH + 1];
    uint8_t cgbFlag;
};


void cartridge_get_header(const struct Cartridge *cartridge, struct CartridgeHeader *header);
bool cartridge_load(struct Cartridge *cartridge, const char *romPath);
void cartridge_teardown(struct Cartridge *cartridge);


#endif
