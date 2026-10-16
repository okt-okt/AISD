#pragma once
#include "input.h"




struct HuffmanCode
{
    uint8_t id, len;
    uint16_t code;
};



uint8_t const QTableLuminance[64] = {
    16,  11,  10,  16,  24,  40,  51,  61,
    12,  12,  14,  19,  26,  58,  60,  55,
    14,  13,  16,  24,  40,  57,  69,  56,
    14,  17,  22,  29,  51,  87,  80,  62,
    18,  22,  37,  56,  68, 109, 103,  77,
    24,  35,  55,  64,  81, 104, 113,  92,
    49,  64,  78,  87, 103, 121, 120, 101,
    72,  92,  95,  98, 112, 100, 103,  99,
};


uint8_t const QTableChrominance[64] = {
    17, 18, 24, 47, 99, 99, 99, 99,
    18, 21, 26, 66, 99, 99, 99, 99,
    24, 26, 56, 99, 99, 99, 99, 99,
    47, 66, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
    99, 99, 99, 99, 99, 99, 99, 99,
};



#define Index_HTableAC(run, size) (run*10 + size + (run == 0xF))



inline void GenerateQTable(uint8_t result[64], int quality, bool is_luminance)
{
    uint8_t const* const table = is_luminance ? QTableLuminance : QTableChrominance;
    if (quality < 1)
        return;
    else if (quality < 50)
        for (uint32_t i = 0; i < 64; ++i)
            result[i] = clamp(ceilf((float(uint32_t(table[i])) * 5000.f / float(quality)) / 100.f), 1, 255);
    else
        for (uint32_t i = 0; i < 64; ++i)
            result[i] = clamp(ceilf((float(uint32_t(table[i])) * (200.f - 2.f * float(quality))) / 100.f), 1, 255);
}
