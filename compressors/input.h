#pragma once
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <vector>
using std::wcout;
using std::wcin;
using std::endl;
using bytes = std::vector<uint8_t>;
using dataset = std::pair<bytes, bytes>; // header and data




inline uint64_t min(uint64_t a, uint64_t b)
{
    return a < b ? a : b;
}

inline uint64_t max(uint64_t a, uint64_t b)
{
    return a > b ? a : b;
}


inline uint8_t min_bytes(uint32_t val)
{
    return (val & (0xFFUL << 24) ? 4 : (val & (0xFFUL << 16) ? 3 : (val & 0xFF00 ? 2 : 1)));
}


wchar_t uppercase(wchar_t c);


// if case_sensitive is false, then ending must be in uppercase
bool ends_with(const wchar_t* name, const wchar_t* ending, bool case_sensitive = true);


// returns 0 -> success, 1 -> argument error, 2 -> error opening/writing file, 3 -> wrong file extension
int SaveToFile(dataset* d, const wchar_t* file);


// returns 0 -> success, 1 -> error opening file, 2 -> unsufficient memory error, 3 -> file has wrong format
dataset* LoadFromFile(const wchar_t* file);


// writes count_bits bits from bits aligned to the right, bitmemory must be initialized to 0 before first call to bitwrite
// returns number of bits written if count_bits is not 0
// returns number of bits currently written within the last byte if count_bits is 0
// example: bitwrite(to, 0x410, 12, bitmemory), bitwrite(to, 0x3, 2, bitmemory); // will result in bitwrite == { 0x41, 0x0C }
// do not pass bitmemory used by bitread to bitwrite, and it is not recommended to use bitread and bitwrite on the same bytes object due to possible overleap
uint8_t bitwrite(bytes& to, uint32_t bits, uint8_t count_bits, uint16_t& bitmemory);


// reads count bits from the beginning of from to to aligning them to the right, reading will delete bytes from from starting from the beginning; bitmemory must be initialized to 0 before first call to bitread
// returns number of bits read if count_bits is not 0
// returns number of bits unread (aligned to the right) within the first byte if count_bits is 0
// example: from = { 0x12, 0xE3 }; bitread(from, to, 7, bitmemory) /*to==0x09*/, bitread(from, to, 6, bitmemory) /*to==0x1C*/, bitread(from, to, 0, bitmemory) /*to==3*/;
// do not pass bitmemory used by bitread to bitwrite, and it is not recommended to use bitread and bitwrite on the same bytes object due to possible overleap
uint8_t bitread(bytes& from, uint32_t& to, uint8_t count, uint16_t& bitmemory);
