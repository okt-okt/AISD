#pragma once
#include <stdio.h>
#include <math.h>
#include <vector>
using bytes = std::vector<uint8_t>;
using image = std::pair<bytes, bytes>; // header and data



enum AISD_RAW_TYPE : uint8_t
{
    TYPE_ImageBW          = 0x01, // black and white image (1 bit per pixel),          .AISD.RAW
    TYPE_ImageGrayscale   = 0x08, // grayscale image (8 bits per pixel),               .AISD.RAW
    TYPE_ImageColorful    = 0x03, // RGB image (24 bits per pixel),                    .AISD.RAW
    TYPE_ImageColorfulY   = 0x30, // YCbCr image (24 bits per pixel),                  .AISD.RAW
    TYPE_JPEG             = 0xA0, // JPEG file data,                                   .JPG or .JPEG
    TYPE_IncompatibleJPEG = 0xA1, // JPEG file data incompatible with current decoder, .JPG or .JPEG
};



inline uint64_t min(uint64_t a, uint64_t b)
{
    return a < b ? a : b;
}

inline uint64_t max(uint64_t a, uint64_t b)
{
    return a > b ? a : b;
}


inline int64_t clamp(int64_t val, int64_t min = 0, int64_t max = 255)
{
    return (val < min ? min : (val > max ? max : val));
}


inline uint8_t min_bytes(uint32_t val)
{
    return (val & (0xFFUL << 24) ? 4 : (val & (0xFFUL << 16) ? 3 : (val & 0xFF00 ? 2 : 1)));
}


inline uint8_t min_bits(uint32_t val)
{
    uint32_t res = 0, mask = 0xFFFF'FFFF;
    for (; val & mask; mask <<= 1)
        ++res;
    return res;
}


inline uint8_t category(int32_t val)
{
    return min_bits(val < 0 ? -val : val);
}


wchar_t uppercase(wchar_t c);


// if case_sensitive is false, then ending must be in uppercase
bool ends_with(const wchar_t* name, const wchar_t* ending, bool case_sensitive = true);


// returns 0 -> success, 1 -> argument error, 2 -> error opening/writing file, 3 -> wrong file extension
int SaveToFile(image* img, const wchar_t* file);


// returns 0 -> success, 1 -> error opening file, 2 -> unsufficient memory error, 3 -> file has wrong format
image* LoadFromFile(const wchar_t* file);


// writes count_bits bits from bits aligned to the right, bitmemory must be initialized to 0 before first call to bitwrite
// returns number of bits written if count_bits is not 0
// returns number of bits currently written within the last byte if count_bits is 0
// example: bitwrite(to, 0x410, 12, bitmemory), bitwrite(to, 0x3, 2, bitmemory); // will result in bitwrite == { 0x41, 0x0C }
// do not pass bitmemory used by bitread to bitwrite, and it is not recommended to use bitread and bitwrite on the same bytes object due to possible overleap
uint8_t bitwrite(bytes& to, uint32_t bits, uint8_t count_bits, uint16_t& bitmemory);


//// writes count_bits bits from bits aligned to the right, bitmemory must be initialized to 0 before first call to bitwrite; it also "stuffs" forming 0xFF bytes inserting 0x00 bytes after them
//// returns number of bits written if count_bits is not 0
//// returns number of bits currently written within the last byte if count_bits is 0
//// example: bitwrite(to, 0x410, 12, bitmemory), bitwrite(to, 0x3, 2, bitmemory); // will result in bitwrite == { 0x41, 0x0C }
//// do not pass bitmemory used by bitread to bitwrite, and it is not recommended to use bitread and bitwrite on the same bytes object due to possible overleap
//uint8_t bitwrite_ff(bytes& to, uint32_t bits, uint8_t count_bits, uint16_t& bitmemory);


// reads count bits from the beginning of from to to aligning them to the right, reading will delete bytes from from starting from the beginning; bitmemory must be initialized to 0 before first call to bitread
// returns number of bits read if count_bits is not 0
// returns number of bits unread (aligned to the right) within the first byte if count_bits is 0
// example: from = { 0x12, 0xE3 }; bitread(from, to, 7, bitmemory) /*to==0x09*/, bitread(from, to, 6, bitmemory) /*to==0x1C*/, bitread(from, to, 0, bitmemory) /*to==3*/;
// do not pass bitmemory used by bitread to bitwrite, and it is not recommended to use bitread and bitwrite on the same bytes object due to possible overleap
uint8_t bitread(bytes& from, uint32_t& to, uint8_t count, uint16_t& bitmemory);
