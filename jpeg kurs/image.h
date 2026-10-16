#pragma once
#include "compress.h"
#include "transform.hpp"




struct INTERNAL_JPEG_HEADER
{
    uint16_t width, height, JPEG_meta_size;
    uint8_t quality;
    uint8_t components : 4;
    uint8_t component_samples[6]; // samples of component in horizontal and vertical in an MCU
    uint8_t component_ids[3], component_QTable[3], component_HTable[3][2], HTablesCodesByLen[2][2][16], HTablesTotalLength[2][2];
    uint16_t QTablesOffsets[4], HTablesOffsets[2][2];
};



// Chroma subsampling 4:2:0 (Y:Cb:Cr = 2:1:1)
// Sequential interleaved encoding ([MCU1: Y0 Y1 Y2 Y3 Cb Cr] [MCU2: Y0 Y1 Y2 Y3 Cb Cr] ...)
image* CompressJPEG(image* img, int quality, uint8_t YCbCr_HTable[3][2]);



image* UncompressJPEG(image* img);
