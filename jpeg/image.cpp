#include "image.h"




image* CompressJPEG(image* img, int quality)
{
    if (!img || img->first.size() < 11 || !(img->first[0] == TYPE_ImageColorful || img->first[0] == TYPE_ImageGrayscale || img->first[0] == TYPE_ImageColorfulY || img->first[0] == TYPE_ImageBW))
        return NULL;
    uint32_t img_width = *reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(img->first.data()) + 3),
        img_height = *reinterpret_cast<uint32_t*>(reinterpret_cast<uint8_t*>(img->first.data()) + 7);
    if (img->second.size() != (img_width * img_height * (img->first[0] == TYPE_ImageColorful || img->first[0] == TYPE_ImageColorfulY ? 3 : 1) + 7 * (img->first[0] == TYPE_ImageBW)) / (img->first[0] == TYPE_ImageBW ? 8 : 1) || img_width > 0xFFFF || img_height > 0xFFFF)
        return NULL;
    image* res = new image();
    if (!res)
        return NULL;
    struct EncodedVal
    {
        uint8_t RRRR, SSSS; // DC: RRRR=0xFF
        int16_t val;
    };
    std::vector<EncodedVal> encoded;
    INTERNAL_JPEG_HEADER header {};
    uint8_t c, components = header.components = (img->first[0] == TYPE_ImageColorful || img->first[0] == TYPE_ImageColorfulY ? 3 : 1);
    uint8_t component_samples_xm = 1 + components / 2, component_samples_ym = component_samples_xm;
    header.width = img_width; header.height = img_height;
    header.quality = quality;
    header.JPEG_meta_size = 30 + 273 * (components == 3 ? 2 : 1) + 5 * components;
    header.component_ids[0] = 1; header.component_ids[1] = 2; header.component_ids[2] = 3;
    header.component_samples[0] = header.component_samples[1] = 1 + components / 2;
    header.component_samples[2] = header.component_samples[3] = header.component_samples[4] = header.component_samples[5] = 1;
    header.component_QTable[0] = 0; header.component_QTable[1] = header.component_QTable[2] = 1;
    header.component_HTable[0][0] = header.component_HTable[0][1] = 0;
    header.component_HTable[1][0] = header.component_HTable[1][1] = header.component_HTable[2][0] = header.component_HTable[2][1] = 1;
#define insert_internal_HTable(c,n,table) header.HTablesOffsets[c][n] = res->first.size()+4+sizeof(INTERNAL_JPEG_HEADER); res->first.insert(res->first.end(), table, table+sizeof(table));
    insert_internal_HTable(0, 0, HTableEncoded::LuminanceDC); insert_internal_HTable(1, 0, HTableEncoded::LuminanceAC);
    insert_internal_HTable(0, 1, HTableEncoded::ChrominanceDC); insert_internal_HTable(1, 1, HTableEncoded::ChrominanceAC);
    header.QTablesOffsets[1] = (header.QTablesOffsets[0] = res->first.size() + 4 + sizeof(INTERNAL_JPEG_HEADER)) + 64;
    res->first.resize(res->first.size() + 128);
    GenerateQTable(res->first.data() + header.QTablesOffsets[0] - 4 - sizeof(INTERNAL_JPEG_HEADER), quality, true);
    GenerateQTable(res->first.data() + header.QTablesOffsets[1] - 4 - sizeof(INTERNAL_JPEG_HEADER), quality, false);
    memcpy(header.HTablesCodesByLen[0][0], HTableEncoded::CodesByLen::LuminanceDC, 16);
    memcpy(header.HTablesCodesByLen[0][1], HTableEncoded::CodesByLen::ChrominanceDC, 16);
    memcpy(header.HTablesCodesByLen[1][0], HTableEncoded::CodesByLen::LuminanceAC, 16);
    memcpy(header.HTablesCodesByLen[1][1], HTableEncoded::CodesByLen::ChrominanceAC, 16);
    header.HTablesTotalLength[0][0] = header.HTablesTotalLength[0][1] = 12;
    header.HTablesTotalLength[1][0] = header.HTablesTotalLength[1][1] = 162;
    uint8_t* beg = img->second.data(), * end = img->second.data() + img->second.size(), * carets[3] {}, * caret;
    bytes uncompressed(img_width * img_height * components);
    res->first.insert(res->first.begin(), reinterpret_cast<uint8_t*>(&header), reinterpret_cast<uint8_t*>(&header) + sizeof(INTERNAL_JPEG_HEADER));
    res->first.insert(res->first.begin(), { TYPE_JPEG, uint8_t(4 + sizeof(INTERNAL_JPEG_HEADER)), uint8_t((4 + sizeof(INTERNAL_JPEG_HEADER)) >> 8), img->first[0] });

    // Color space conversion and separating components
    if (components == 3)
    {
        uint8_t buffer[3];
        carets[2] = (carets[1] = (carets[0] = uncompressed.data()) + uncompressed.size() / 3) + uncompressed.size() / 3;
        for (caret = beg; caret < end; caret += 3)
        {
            if (img->first[0] == TYPE_ImageColorful)
                RGB_to_YCbCr(caret, buffer);
            for (c = 0; c < components; ++c)
                *(carets[c]++) = buffer[c];
        }
        carets[2] = (carets[1] = (carets[0] = uncompressed.data()) + uncompressed.size() / 3) + uncompressed.size() / 3;
    }
    else if (img->first[0] == TYPE_ImageBW)
        for ((c = 0x80), (caret = beg), (carets[2] = (carets[1] = carets[0] = uncompressed.data()) + uint32_t(img_width) * img_height); caret < end; ++caret, (c = 0x80))
            for (; carets[1] < carets[2] && c; c >>= 1)
                *(carets[1]++) = (*caret & c ? 0xFF : 0);
    else
        carets[0] = (uncompressed = img->second).data();

    // According to "4:2:0 rule" downsample chromatic channels:
    for (c = 1; c < components; ++c)
        downsampling(carets[c], carets[c], img_width, img_height, 2);

    // Substract 128 to center values among 0 in order to prepare data to perform DCT on it:
    for (c = 0; c < components; ++c)
        for (end = (caret = carets[c]) + img_width * img_height / (c ? 4 : 1); caret < end; ++caret)
            *caret -= 128;

    // Work on MCU-s (4 Y blocks, 1 Cb block, 1 Cr block)
    {
        uint32_t uncompressed_width[3] { img_width, img_width / 2, img_width / 2 }, uncompressed_height[3] { img_height, img_height / 2, img_height / 2 },
            MCU_x {}, MCU_y {}, MCU_xm = (img_width + component_samples_xm * 8 - 1) / component_samples_xm / 8, MCU_ym = (img_height + component_samples_ym * 8 - 1) / component_samples_ym / 8, MCU_block_x, MCU_block_y, block_x, block_y, x, y;
        union
        {
            float DCT[64];
            int16_t Q[64];
            uint8_t table[64];
        } buffer;
        int16_t prevDC[3] {}, val, sum {};
        int8_t block[64], average {};
        uint8_t* table, dir, count = 1;

        for (MCU_y = 0; MCU_y < MCU_ym; ++MCU_y)
            for (MCU_x = 0; MCU_x < MCU_xm; ++MCU_x)
                for (c = 0; c < components; ++c)
                {
                    // Set quantization table for current MCU:
                    table = res->first.data() + header.QTablesOffsets[header.component_QTable[c]];

                    // For each block in MCU:
                    for (MCU_block_y = 0; MCU_block_y < header.component_samples[2 * c + 1]; ++MCU_block_y)
                        for (MCU_block_x = 0; MCU_block_x < header.component_samples[2 * c]; ++MCU_block_x)
                        {
                            block_x = (MCU_x * uint32_t(header.component_samples[2 * c]) + MCU_block_x) * 8;
                            block_y = (MCU_y * uint32_t(header.component_samples[2 * c + 1]) + MCU_block_y) * 8;

                            // Copy block (fill empty with average):
                            if (block_x >= uncompressed_width[c] || block_y >= uncompressed_height[c])
                                for (x = 0; x < 64; ++x)
                                    block[x] = average;
                            else
                            {
                                for (sum = count = y = 0; y < 8; ++y)
                                    for (x = 0; x < 8; ++x)
                                        if (block_x + x < uncompressed_width[c] && block_y + y < uncompressed_height[c])
                                            ++count, (sum += int16_t(block[x + y * 8] = reinterpret_cast<int8_t*>(carets[c])[(block_x + x) + (block_y + y) * uncompressed_width[c]]));
                                average = sum / count;
                                for (x = uncompressed_width[c] - block_x; x < 8; ++x)
                                    for (y = 0; y < 8; ++y)
                                        block[x + y * 8] = average;
                                for (y = uncompressed_height[c] - block_y; y < 8; ++y)
                                    for (x = 0; x < 8; ++x)
                                        block[x + y * 8] = average;
                            }

                            // DCT and quantization:
                            FDCT(block, buffer.DCT);
                            for (x = 0; x < 64; ++x)
                                buffer.Q[x] = int16_t(roundf(buffer.DCT[x] / float(uint32_t(table[x]))));

                            // DC encoding with DIFF and HA:
                            val = buffer.Q[0] - prevDC[c];
                            prevDC[c] = buffer.Q[0];
                            encoded.push_back({ 0xFF, category(val), val });

                            // AC-s in zigzag order:
                            x = 1; y = 0; dir = 0; count = 0;
                            do
                            {
                                // Encode AC with RLE for 0-s:
                                if (val = buffer.Q[x + y * 8])
                                {
                                    // ZRL:
                                    for (; count > 15; count -= 16)
                                        encoded.push_back({ 15, 0, 0 });
                                    encoded.push_back({ count, category(val), val });
                                    count = 0;
                                }
                                else
                                    ++count;
                            }
                            while (zigzag(x, y, dir));
                            // EOB:
                            if (count)
                                encoded.push_back({ 0, 0, 0 });
                        }
                }
        // Huffman coding
        {
            HuffmanCode const* tableDC = HTableLuminanceDC, * tableAC = HTableLuminanceAC;
            uint16_t bitmemory {}, sample = 0;
            for (auto caret = encoded.begin(); caret < encoded.end(); ++caret)
                if (caret->RRRR == 0xFF)
                {
                    if (sample++ < 4 || components == 1)
                        (tableDC = HTableLuminanceDC), (tableAC = HTableLuminanceAC);
                    else
                        (tableDC = HTableChrominanceDC), (tableAC = HTableChrominanceAC);
                    sample %= 6;
                    bitwrite(res->second, tableDC[caret->SSSS].code, tableDC[caret->SSSS].len, bitmemory);
                    bitwrite(res->second, caret->val - (caret->val < 0), caret->SSSS, bitmemory);
                }
                else
                {
                    val = Index_HTableAC(caret->RRRR, caret->SSSS);
                    bitwrite(res->second, tableAC[val].code, tableAC[val].len, bitmemory);
                    bitwrite(res->second, caret->val - (caret->val < 0), caret->SSSS, bitmemory);
                }
            if (bitmemory)
                bitwrite(res->second, 0xFF, 8 - bitwrite(res->second, 0, 0, bitmemory), bitmemory);
        }
    }

    return res;
}



image* UncompressJPEG(image* img)
{
    if (!img || img->first.size() < (4 + sizeof(INTERNAL_JPEG_HEADER)) || img->first[0] != TYPE_JPEG)
        return NULL;
    INTERNAL_JPEG_HEADER* header = reinterpret_cast<INTERNAL_JPEG_HEADER*>(img->first.data() + 4);
    image* res = new image();
    if (!res)
        return NULL;
    res->first.insert(res->first.end(), { (header->components == 1 ? TYPE_ImageGrayscale : TYPE_ImageColorful), 8, 0, uint8_t(header->width), uint8_t(header->width >> 8), 0, 0, uint8_t(header->height), uint8_t(header->height >> 8), 0, 0 });
    res->second.resize(uint64_t(header->components) * header->width * header->height);

    // Initializing decompressing tables and variables:
    uint8_t const* QTables[3] {}, * HTables[3][2] {}, components = header->components;
    uint32_t compressed_width[3], compressed_height[3];
    //uint32_t off_bytes {};
    uint8_t offset_by_len[2][2][17] {}, c, bits = 8, * caret = img->second.data(), * end = caret + img->second.size(), dir, codesize, count = 1, s, n;
    uint16_t mincode[2][2][16], maxcode[2][2][16] {}, code;
    uint8_t component_samples_xm {}, component_samples_ym {};
    for (c = 0; c < components; ++c)
    {
        QTables[c] = img->first.data() + header->QTablesOffsets[header->component_QTable[c]];
        HTables[c][0] = img->first.data() + header->HTablesOffsets[0][header->component_HTable[c][0]];
        HTables[c][1] = img->first.data() + header->HTablesOffsets[1][header->component_HTable[c][1]];
        for (dir = 0; dir < 2; ++dir)
            if (!(offset_by_len[dir][n = header->component_HTable[c][dir]][16]))
                for (code = s = 0; s < 16; ++s)
                {
                    if (header->HTablesCodesByLen[dir][n][s])
                    {
                        mincode[dir][n][s] = code;
                        maxcode[dir][n][s] = (code += header->HTablesCodesByLen[dir][n][s]) - 1;
                    }
                    else
                        mincode[dir][n][s] = 0xFFFF;
                    code <<= 1;
                    offset_by_len[dir][n][s + 1] = offset_by_len[dir][n][s] + header->HTablesCodesByLen[dir][n][s];
                }
        if (component_samples_xm < header->component_samples[2 * c])
            component_samples_xm = header->component_samples[2 * c];
        if (component_samples_ym < header->component_samples[2 * c + 1])
            component_samples_ym = header->component_samples[2 * c + 1];
    }
    for (c = 0; c < components; ++c)
    {
        compressed_width[c] = uint32_t(header->width) * header->component_samples[2 * c] / component_samples_xm;
        compressed_height[c] = uint32_t(header->height) * header->component_samples[2 * c + 1] / component_samples_ym;
    }
    uint32_t MCU_x {}, MCU_y {}, MCU_xm = (header->width + component_samples_xm * 8 - 1) / component_samples_xm / 8, MCU_ym = (header->height + component_samples_ym * 8 - 1) / component_samples_ym / 8, MCU_block_x, MCU_block_y, block_x, block_y, x, y, xm, ym;
    int16_t prevDC[3] {}, val;
    uint8_t const* table;
    bytes compressed(uint64_t(header->components) * header->width * header->height);
    uint8_t* carets[3] { compressed.data(), compressed.data() + uint64_t(header->width) * header->height, compressed.data() + uint64_t(header->width) * header->height * 2 };
    union
    {
        int16_t itg[64];
        int8_t sgn[64];
        uint8_t uns[64];
    } block;
    float blockDCT[64];

    // Reading into blocks in MCU-s:
    auto read_code = [&](uint8_t component, uint8_t HTable_c)->bool // true -> error
        {
            n = header->component_HTable[component][HTable_c];
            code = codesize = 0;
            while (caret < end)
            {
                code = (code << 1) | uint16_t((*caret >> (--bits)) & 1);
                if (!bits)
                    ++caret, (bits = 8);
                if (code > maxcode[HTable_c][n][codesize] || mincode[HTable_c][n][codesize] > maxcode[HTable_c][n][codesize])
                {
                    if (++codesize == 16)
                        break;
                    continue;
                }
                code = HTables[component][HTable_c][offset_by_len[HTable_c][n][codesize] + (code - mincode[HTable_c][n][codesize])];
                return false;
            }
            return true;
        };
    auto read_val = [&](uint8_t SSSS)->bool // true -> error
        {
            val = 0;
            for (s = SSSS; s;)
                if (bits > s)
                {
                    val = val | uint16_t((*caret >> (bits -= s)) & (0xFF >> (8 - s)));
                    break;
                }
                else
                {
                    val = val | ((uint16_t(*caret) & (0xFF >> (8 - bits))) << (s - bits));
                    s -= bits;
                    bits = 8;
                    if (!(++caret < end) && s)
                        return true;
                }
            if (SSSS && !(val & (1 << (SSSS - 1))))
                val += (0xFFFF << SSSS) + 1;
            return false;
        };
    for (MCU_y = 0; MCU_y < MCU_ym; ++MCU_y)
        for (MCU_x = 0; MCU_x < MCU_xm; ++MCU_x)
            for (c = 0; c < components; ++c)
            {
                // For each block in MCU:
                for (MCU_block_y = 0; MCU_block_y < header->component_samples[2 * c + 1]; ++MCU_block_y)
                    for (MCU_block_x = 0; MCU_block_x < header->component_samples[2 * c]; ++MCU_block_x)
                    {
                        block_x = (MCU_x * uint32_t(header->component_samples[2 * c]) + MCU_block_x) * 8;
                        block_y = (MCU_y * uint32_t(header->component_samples[2 * c + 1]) + MCU_block_y) * 8;

                        // Decode DC:
                        read_code(c, 0);
                        read_val(code);
                        block.itg[0] = prevDC[c] += val;

                        // Decode AC-s:
                        x = 1; y = dir = 0;
                        do
                        {
                            read_code(c, 1);
                            read_val(code & 0xF);

                            if (code == 0x00)
                            {
                                if (block_x < compressed_width[c] && block_y < compressed_height[c])
                                    do
                                        block.itg[x + y * 8] = 0;
                                    while (zigzag(x, y, dir));
                                break;
                            }
                            for (s = code >> 4; s--;)
                            {
                                block.itg[x + y * 8] = 0;
                                if (!zigzag(x, y, dir))
                                    goto EXIT_FILL_BLOCK;
                            }
                            block.itg[x + y * 8] = val;
                        }
                        while (zigzag(x, y, dir));
                    EXIT_FILL_BLOCK:

                        // Analyze block contents if only it contains image data:
                        if (block_x < compressed_width[c] && block_y < compressed_height[c])
                        {
                            // Dequantization and inverse DCT:
                            for (y = 8; y--;)
                                for (x = 8; x--;)
                                    (val = x + y * 8), (blockDCT[val] = float(block.itg[val]) * QTables[c][val]);
                            IDCT(blockDCT, block.sgn);

                            // Add 128 and insert block into channel:
                            for ((y = 0), (xm = min(compressed_width[c] - block_x, 8)), (ym = min(compressed_height[c] - block_y, 8)); y < ym; ++y)
                                for (x = 0; x < xm; ++x)
                                    carets[c][(block_x + x) + (block_y + y) * compressed_width[c]] = block.uns[x + y * 8] + 128;
                        }
                    }
            }

    // Upsample channels:
    for (c = 0; c < components; ++c)
        resize(carets[c], compressed_width[c], compressed_height[c], res->second.data(), header->width, header->height, header->component_ids[c] - 1, components);

    // Color conversion:
    if (components == 3)
        for ((caret = res->second.data()), (end = caret + uint32_t(header->width) * header->height * 3); caret < end; caret += 3)
            YCbCr_to_RGB(caret, caret);

    return res;
}
