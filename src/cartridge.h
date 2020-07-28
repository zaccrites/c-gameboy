
#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>


#define CARTRIDGE_TYPE_ROM      (1 << 0)
#define CARTRIDGE_TYPE_MBC1     (1 << 1)
#define CARTRIDGE_TYPE_MBC2     (1 << 2)
#define CARTRIDGE_TYPE_MBC3     (1 << 4)
#define CARTRIDGE_TYPE_MBC5     (1 << 5)
#define CARTRIDGE_TYPE_RAM      (1 << 6)
#define CARTRIDGE_TYPE_BATTERY  (1 << 7)



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
    uint16_t type;
};


void cartridge_get_header(const struct Cartridge *cartridge, struct CartridgeHeader *header);
bool cartridge_load(struct Cartridge *cartridge, const char *romPath);
void cartridge_teardown(struct Cartridge *cartridge);

void cartridge_get_type_string(const struct CartridgeHeader *header, char *buffer, size_t buflen);


#endif
