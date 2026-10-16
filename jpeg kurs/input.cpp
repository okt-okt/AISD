#include "image.h"




wchar_t uppercase(wchar_t c)
{
    return (L'a' <= c && c <= L'z') ? c - (L'a' - L'A') : c;
}



bool ends_with(const wchar_t* name, const wchar_t* ending, bool case_sensitive)
{
    const wchar_t* caret = name, * check = ending;
    for (; *caret; ++caret);
    for (; *check; ++check);
    for (; (case_sensitive ? *caret : uppercase(*caret)) == *check && caret != name && check != ending; --check, --caret);
    return check == ending && (case_sensitive ? *caret : uppercase(*caret)) == *check;
}



int SaveToFile(image* img, const wchar_t* file)
{
    if (!img || !file)
        return 1;
    FILE* f {};
    int hr = 0;
    uint32_t start = 0, x = 0, y = 0, count;
    bytes buffer;
    INTERNAL_JPEG_HEADER* header;
    uint8_t* caret {}, * beg {}, * end {}, c, s, n, dir = 1, * table;
#define bufferize buffer.insert(buffer.end(), 
#define split(val16) uint8_t((val16)>>8), uint8_t(val16)
#define connect(high4, low4) uint8_t(((high4)<<4)|((low4)&0xF))
#define stop )

    if (img->first.size())
        switch (img->first[0])
        {
            case TYPE_JPEG:
                if (ends_with(file, L".JPG", false) || ends_with(file, L".JPEG", false))
                {
                    if (_wfopen_s(&f, file, L"wb"))
                        hr = 2;
                    if (!hr)
                    {
                        header = reinterpret_cast<INTERNAL_JPEG_HEADER*>(img->first.data() + 4);
                        // SOI marker:
                        bufferize { split(0xFFD8) } stop;
                        // DQT segment:
                        for (count = c = 0; c < header->components; ++c)
                        {
                            for (s = 0; s < c && header->component_QTable[s] != header->component_QTable[c]; ++s);
                            if (s == c)
                                ++count;
                        }
                        bufferize { split(0xFFDB), split(2 + 65 * count) } stop;
                        for (c = 0; c < header->components; ++c)
                        {
                            for (s = 0; s < c && header->component_QTable[s] != header->component_QTable[c]; ++s);
                            if (s == c)
                            {
                                bufferize { connect(0, header->component_QTable[c]) } stop;
                                table = img->first.data() + header->QTablesOffsets[header->component_QTable[c]];
                                x = y = 0; dir = 1;
                                do
                                    bufferize { static_cast<uint8_t>(table[x + y * 8]) } stop;
                                while (zigzag(x, y, dir));
                            }
                        }                        
                        // SOF0 marker:
                        bufferize { split(0xFFC0), split(8 + 3 * header->components), 8, split(header->height), split(header->width), header->components } stop;
                        for (c = 0; c < header->components; ++c)
                            bufferize { header->component_ids[c], connect(header->component_samples[2 * c], header->component_samples[2 * c + 1]), header->component_QTable[c] } stop;
                        // DHT segment:
                        for (count = dir = 0; dir < 2; ++dir)
                            for (c = 0; c < header->components; ++c)
                            {
                                n = header->component_HTable[c][dir];
                                for (s = 0; s < c && header->component_HTable[s][dir] != n; ++s);
                                if (s == c)
                                    count += 16 + 1 + header->HTablesTotalLength[dir][n];
                            }
                        bufferize { split(0xFFC4), split(count + 2) } stop;
                        for (count = dir = 0; dir < 2; ++dir)
                            for (c = 0; c < header->components; ++c)
                            {
                                n = header->component_HTable[c][dir];
                                for (s = 0; s < c && header->component_HTable[s][dir] != n; ++s);
                                if (s == c)
                                {
                                    bufferize { connect(dir, n) } stop;
                                    table = img->first.data() + header->HTablesOffsets[dir][n];
                                    bufferize header->HTablesCodesByLen[dir][n], header->HTablesCodesByLen[dir][n] + 16 stop;
                                    bufferize table, table + header->HTablesTotalLength[dir][n] stop;
                                }
                            }                        
                        // SOS marker:
                        bufferize { split(0xFFDA), split(6 + 2 * header->components), header->components } stop;
                        for (c = 0; c < header->components; ++c)
                            bufferize { header->component_ids[c], connect(header->component_HTable[c][0], header->component_HTable[c][1])} stop;
                        bufferize { 0, 63, connect(0, 0) } stop;

                        if (fwrite(buffer.data(), 1, buffer.size(), f) < buffer.size())
                            hr = 2;
                        buffer.clear();
                        buffer.insert(buffer.end(), { 0, 0xFF, 0xD9 });
                    }
                    // Entropy coded data and EOI marker in the end:
                    if (!hr)
                        for (end = (beg = caret = img->second.data()) + img->second.size(); caret < end;)
                            if (*(caret++) == 0xFF)
                            {
                                if (fwrite(beg, 1, caret - beg, f) < caret - beg || !fwrite(buffer.data(), 1, 1, f))
                                    hr = 2;
                                beg = caret;
                            }
                    if (!hr && ((caret != beg && fwrite(beg, 1, caret - beg, f) < caret - beg) || (fwrite(buffer.data() + 1, 1, 2, f) < 2)))
                        hr = 2;
                }
                else
                    hr = 3;
                break;

            case TYPE_IncompatibleJPEG:
                if (!ends_with(file, L".JPG", false) && !ends_with(file, L".JPEG", false))
                    hr = 3;
                start = img->first.size();
                goto DEFAULT_SAVE;

            default:
                if (!ends_with(file, L".AISD.RAW", false))
                    hr = 3;
            DEFAULT_SAVE:
                if (!hr && (_wfopen_s(&f, file, L"wb") ||
                    (img->first.size() > start && fwrite(img->first.data() + start, 1, img->first.size() - start, f) < img->first.size() - start) ||
                    (img->second.size() && fwrite(img->second.data(), 1, img->second.size(), f) < img->second.size())
                    ))
                    hr = 2;
        }

    if (f)
        fclose(f);
    return hr;
}



image* LoadFromFile(const wchar_t* file)
{
    if (!file)
        return NULL;
    bytes filedata;
    FILE* f {};
    image* res = new image();
    INTERNAL_JPEG_HEADER* header;
    int hr = res ? 0 : 2;
    uint32_t read, s, x, y;
    uint16_t val;
    uint8_t buffer[256], * caret, * end, * endb, dir;

    if (!hr && _wfopen_s(&f, file, L"rb"))
        hr = 1;
    if (!hr && ends_with(file, L".AISD.RAW", false))
    {
        if (fread_s(buffer, 3, 1, 3, f) < 3)
            hr = 3;
        else
        {
            res->first.insert(res->first.begin(), buffer, buffer + 3);
            res->first.resize(3 + (read = *reinterpret_cast<uint16_t*>(buffer + 1)));
            if (read && fread_s(res->first.data() + 3, read, 1, read, f) < read)
                hr = 3;
            else
            {
            READ_TO_END:
                do
                    if (read = fread_s(buffer, 256, 1, 256, f))
                        res->second.insert(res->second.end(), buffer, buffer + read);
                while (read == 256);
            }
        }
    }
    else if (!hr && (ends_with(file, L".JPG", false) || ends_with(file, L".JPEG", false)))
    {
        INTERNAL_JPEG_HEADER header {};
    #define getval(ptr) ((uint16_t(*(ptr))<<8)|uint16_t(*((ptr)+1)))
    #define eof(ptr) ((ptr) > end)
        do
            if (read = fread_s(buffer, 256, 1, 256, f))
                filedata.insert(filedata.end(), buffer, buffer + read);
        while (read == 256);
        end = (caret = filedata.data()) + filedata.size();

        // SOI marker:
        if (filedata.size() < 2 || !caret || !(caret[0] == 0xFF && caret[1] == 0xD8))
            hr = 3;
        caret += 2;
        while (!hr && caret < end)
        {
            if (*caret == 0xFF)
            {
                if (++caret < end)
                    switch (*caret)
                    {
                        case 0xDB:
                            // found DQT marker:
                            if (eof(++caret + 1) || (val = getval(caret)) < 11 || eof(caret + val))
                                hr = 3;
                            if (hr)
                                break;
                            if ((val -= 2) % 65 != 0)
                                goto INCOMPATIBLE_JPEG;
                            for (endb = (caret += 2) + val; caret < endb;)
                            {
                                if ((*caret) >> 4 != 0 || ((*caret) & 0xF) > 3)
                                    goto INCOMPATIBLE_JPEG;
                                header.QTablesOffsets[*(caret++)] = (val = res->first.size()) + 4 + sizeof(INTERNAL_JPEG_HEADER);
                                res->first.resize(val + 64, 0);
                                x = y = 0; dir = 1;
                                do
                                    res->first[val + x + y * 8] = *(caret++);
                                while (zigzag(x, y, dir));
                            }
                            break;

                        case 0xC0:
                            // found SOF0 marker:
                            if (eof(++caret + 1) || (val = getval(caret)) < 11 || (val - 8) % 3 != 0 || eof(caret + val) || *(caret + 7) * 3 + 8 != val)
                                hr = 3;
                            if (hr)
                                break;
                            if (*(caret + 2) != 8 || !(header.width = getval(caret + 5)) || ((header.components = *(caret + 7)) != 1 && header.components != 3))
                                goto INCOMPATIBLE_JPEG;
                            header.height = getval(caret + 3);
                            for ((caret += 8), (y = x = 0); x < header.components; ++x)
                            {
                                if ((val = caret[3 * x] - 1) < 3 && !(header.component_samples[2 * val]))
                                {
                                    ++y;
                                    if ((s = header.component_samples[2 * val] = caret[3 * x + 1] >> 4) > 4 || !s ||
                                        (s = header.component_samples[2 * val + 1] = caret[3 * x + 1] & 0xF) > 4 || !s ||
                                        (header.component_QTable[val] = caret[3 * x + 2]) > 3)
                                        goto INCOMPATIBLE_JPEG;
                                }
                            }
                            if (y != header.components)
                                goto INCOMPATIBLE_JPEG;
                            caret += 3 * header.components;
                            break;

                        case 0xC1: case 0xC2: case 0xC3: case 0xC5: case 0xC6: case 0xC7:
                        case 0xC9: case 0xCA: case 0xCB: case 0xCD: case 0xCE: case 0xCF:
                            // found SOF1-15 marker:
                            goto INCOMPATIBLE_JPEG;

                        case 0xC4:
                            // found DHT marker:
                            if (eof(++caret + 1) || (val = getval(caret)) < 2 || eof(caret + val))
                                hr = 3;
                            endb = caret + val;
                            caret += 2;
                            while (!hr && caret < endb)
                            {
                                if (caret + 17 > endb || (x = (*caret >> 4)) > 1)
                                    hr = 3;
                                else if ((y = (*caret & 0xF)) > 1)
                                    goto INCOMPATIBLE_JPEG;
                                else
                                    for (++caret, (header.HTablesTotalLength[x][y] = val = 0); val < 16; ++val)
                                        header.HTablesTotalLength[x][y] += header.HTablesCodesByLen[x][y][val] = *(caret++);
                                if (!hr && caret + header.HTablesTotalLength[x][y] > endb)
                                    hr = 3;
                                else if (!hr && header.HTablesTotalLength[x][y])
                                {
                                    header.HTablesOffsets[x][y] = res->first.size() + 4 + sizeof(INTERNAL_JPEG_HEADER);
                                    res->first.insert(res->first.end(), caret, caret + header.HTablesTotalLength[x][y]);
                                    caret += header.HTablesTotalLength[x][y];
                                }
                            }
                            break;

                        case 0xD9:
                            // found EOI marker:
                            caret = end;
                            break;

                        case 0xDA:
                            // found SOS marker:
                            if (eof(++caret + 1) || (val = getval(caret)) < 3 || eof(caret + val) || !(s = *(caret + 2)) || s > 4 || val != 6 + 2 * s)
                                hr = 3;
                            if (hr)
                                break;
                            if (*(caret + 2) != header.components)
                                goto INCOMPATIBLE_JPEG;
                            for ((caret += 3), (y = x = 0); x < header.components; ++x)
                            {
                                if ((val = (header.component_ids[x] = caret[2 * x]) - 1) < 3)
                                {
                                    ++y;
                                    if ((header.component_HTable[val][0] = caret[2 * x + 1] >> 4) > 1 ||
                                        (header.component_HTable[val][1] = caret[2 * x + 1] & 0xF) > 1)
                                        goto INCOMPATIBLE_JPEG;
                                }
                            }
                            caret += 2 * header.components;
                            if (y != header.components ||
                                *caret != 0 || *(caret + 1) != 63 || *(caret + 2) != 0)
                                goto INCOMPATIBLE_JPEG;
                            res->second.reserve(end - (caret += 3));
                            while (caret < end)
                            {
                                if (*caret == 0xFF)
                                {
                                    if (*(caret + 1) == 0x00) // byte stuffing
                                        (caret += 2), res->second.insert(res->second.end(), 0xFF);
                                    else if (*(caret + 1) == 0xD9) // EOI marker
                                        caret = end;
                                    else if (0xD0 <= *(caret + 1) && *(caret + 1) <= 0xD7)
                                        caret += 2;
                                    else
                                        goto INCOMPATIBLE_JPEG;
                                }
                                else
                                    res->second.insert(res->second.end(), *(caret++));
                            }
                            break;

                        case 0xDC:
                            // found DNL marker:
                            if (eof(caret + 4) || getval(caret + 1) != 4 || getval(caret + 3) == 0 || (header.height && header.height != getval(caret + 3)))
                                hr = 3;
                            else
                                header.height = getval(caret + 3);
                            caret += 5;
                            break;

                        case 0x00:
                            // found byte stuffing
                            res->second.insert(res->second.end(), 0xFF);
                            ++caret;
                            break;

                        default:
                            // found unknown marker:
                            if (eof(++caret + 1) || eof(caret + getval(caret)))
                                hr = 3;
                            else
                                caret += getval(caret);
                    }
                else
                    goto INCOMPATIBLE_JPEG;
            }
            else
                ++caret;
        }
    #undef get_value
        if (!hr && res->first[0] == TYPE_JPEG && (!header.width || !header.height || !header.components))
            hr = 3;
        header.JPEG_meta_size = filedata.size() - res->second.size();
        if (header.components == 1)
            header.component_samples[0] = header.component_samples[1] = 1;
        if (!hr && res->first[0] == TYPE_JPEG && (header.components == 1 ? header.component_ids[0] != 1 : (header.components == 3 ? header.component_ids[0] + header.component_ids[1] + header.component_ids[2] != 6 || !(header.component_ids[0] != header.component_ids[1] && header.component_ids[1] != header.component_ids[2] && header.component_ids[0] != header.component_ids[2]) : true)))
        {
        INCOMPATIBLE_JPEG:
            res->first.insert(res->first.begin(), { TYPE_IncompatibleJPEG, 0, 0 });
            res->second = std::move(filedata);
        }
        else
        {
            res->first.insert(res->first.begin(), reinterpret_cast<uint8_t*>(&header), reinterpret_cast<uint8_t*>(&header) + sizeof(INTERNAL_JPEG_HEADER));
            val = res->first.size() + 1 + sizeof(INTERNAL_JPEG_HEADER);
            res->first.insert(res->first.begin(), { TYPE_JPEG, uint8_t(val), uint8_t(val >> 8), 0 });
        }
    }

    if (hr && res)
        (delete res), (res = NULL);
    if (f)
        fclose(f);
    return res;
}



uint8_t bitwrite(bytes& to, uint32_t bits, uint8_t count_bits, uint16_t& bitmemory)
{
    if (count_bits > 32)
        return 0;
    if (!count_bits)
        return bitmemory;
    uint8_t wrote, total = 0;

    if (bitmemory)
    {
        wrote = 8 - bitmemory;
        to[to.size() - 1] = to[to.size() - 1] | (uint8_t(count_bits >= wrote ? (bits >> (count_bits - wrote)) : (bits << (8 - bitmemory - (wrote = count_bits)))) & (0xFF >> bitmemory));
        count_bits -= wrote;
        total += wrote;
        bitmemory = (bitmemory + wrote) % 8;
    }
    while (count_bits)
    {
        to.push_back(uint8_t(count_bits >= 8 ? (bits >> (count_bits - (wrote = 8))) : (bits << (8 - (bitmemory = wrote = count_bits)))));
        count_bits -= wrote;
        total += wrote;
    }

    return total;
}



uint8_t bitread(bytes& from, uint32_t& to, uint8_t count, uint16_t& bitmemory)
{
    if (count > 32)
        return 0;
    if (!count)
        return bitmemory >> 8;
    uint8_t read, total = to = 0;
    if (bitmemory & 0xFF00)
    {
        if (count >= (bitmemory >> 8))
        {
            to = bitmemory & 0xFF;
            count -= (total = bitmemory >> 8);
            bitmemory = 0;
        }
        else
        {
            to = (bitmemory >> ((bitmemory >> 8) - (total = count))) & (0xFF >> (8 - count));
            bitmemory -= uint16_t(count) << 8;
            count = 0;
        }
    }
    while (count && from.size())
    {
        read = (count >= 8 ? 8 : count);
        to = (to << read) | uint32_t(from[0] >> (8 - read));
        bitmemory = (uint16_t(8 - read) << 8) | (from[0] & (0xFF >> read));
        total += read;
        count -= read;
        from.erase(from.begin());
    }
    return total;
}
