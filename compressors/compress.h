#pragma once
#include "input.h"




enum AISD_RAW_TYPE : uint8_t
{
    TYPE_Text           = 0x00, // ASCII or ASCII-like text (1 byte),       .TXT
    TYPE_ImageBW        = 0x01, // black and white image (1 bit per pixel), .AISD.RAW
    TYPE_ImageGrayscale = 0x08, // grayscale image (8 bits per pixel),      .AISD.RAW
    TYPE_ImageColorful  = 0x03, // RGB image (24 bits per pixel),           .AISD.RAW
    TYPE_RLE            = 0xFE, // RLE compressed data,                     .AISD.RAW
    TYPE_MTF            = 0x3F, // MTF transformed data,                    .AISD.RAW
    TYPE_HA_Tree        = 0xAA, // Huffman encoded data (tree view),        .AISD.RAW
    TYPE_HA_Canon       = 0xCC, // Huffman encoded data (canonical codes),  .AISD.RAW
    TYPE_BWT            = 0xB8, // BWT transformed data,                    .AISD.RAW
    TYPE_LZ77           = 0x77, // LZ77 compressed data,                    .AISD.RAW
    TYPE_LZSS           = 0x9A, // LZSS compressed data,                    .AISD.RAW
    TYPE_LZ78           = 0x78, // LZ78 compressed data,                    .AISD.RAW
    TYPE_LZW            = 0x8A  // LZW compressed data,                     .AISD.RAW
};



// ctrl_size - Mc [1..8] includes unrepeating sequence flag, symb_size - Ms [1..8]
// returns 0 -> success, 1 -> invalid args/data
dataset* CompressRLE(dataset* d, uint32_t ctrl_size, uint32_t symb_size);


// returns 0 -> success, 1 -> uncompressing error
dataset* UncompressRLE(dataset* d);


dataset* TransformMTF(dataset* d);


dataset* DetransformMTF(dataset* d);


double Entropy(dataset* d, uint32_t symb_size);


dataset* CompressHA(dataset* d, bool use_Canon = true);


dataset* UncompressHA(dataset* d);


dataset* TransformBWT(dataset* d, uint32_t block_size, bool use_BWT2 = true);


dataset* DetransformBWT(dataset* d, bool use_iBWT2 = true);


dataset* CompressLZ77(dataset* d, uint32_t buf_size);


dataset* UncompressLZ77(dataset* d);


dataset* CompressLZSS(dataset* d, uint32_t buf_size);


dataset* UncompressLZSS(dataset* d);


dataset* CompressLZ78(dataset* d, uint32_t dict_size);


dataset* UncompressLZ78(dataset* d);


dataset* CompressLZW(dataset* d, uint32_t dict_size);


dataset* UncompressLZW(dataset* d);
