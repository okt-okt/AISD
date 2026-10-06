#include "compress.h"
#include <algorithm>




dataset* CompressLZ77(dataset* d, uint32_t buf_size)
{
    if (!d || !buf_size)
        return NULL;
    dataset* res = new dataset { d->first, bytes() };
    uint8_t* caret = d->second.data(), * end = d->second.data() + d->second.size(), * beg = caret, * check, size = min_bytes(buf_size);
    if (!res)
        return NULL;
    res->first.insert(res->first.begin(), { TYPE_LZ77, uint8_t(4 + d->first.size()), uint8_t((4 + d->first.size()) >> 8), 0, 0, 0, 0 });
    reinterpret_cast<uint32_t&>(res->first[3]) = buf_size;
    union
    {
        struct
        {
            uint32_t lenm, offm, len;
        };
        uint8_t buffer[12] {};
    };
    auto& r = res->second;
    for (; caret < end; offm = lenm = 0)
    {
        for (check = caret - min(caret - beg, buf_size); check < caret; ++check)
        {
            for (len = 0; check + len < caret && caret + len < end && *(caret + len) == *(check + len) && len < buf_size; ++len);
            if (len > lenm)
                (lenm = len), (offm = caret - check);
        }
        r.insert(r.end(), buffer, buffer + size), r.insert(r.end(), buffer + 4, buffer + 4 + size);
        if ((caret += lenm) < end)
            r.push_back(*(caret++));
    }

    return res;
}



dataset* UncompressLZ77(dataset* d)
{
    if (!d || d->first.size() < 7 || d->first[0] != TYPE_LZ77)
        return NULL;
    dataset* res = new dataset(bytes(d->first.begin() + 7, d->first.end()), bytes());
    uint32_t length {}, offset {};
    uint8_t* caret = d->second.data(), * end = d->second.data() + d->second.size(), read, size = min_bytes(reinterpret_cast<uint32_t&>(d->first[3]));
    if (!res)
        return NULL;

    while (caret < end)
    {
        memcpy(&length, caret, (read = min(end - caret, size)));
        caret += read;
        if (read == size)
        {
            memcpy(&offset, caret, (read = min(end - caret, size)));
            caret += read;
            if (read == size)
            {
                if (length)
                    res->second.insert(res->second.end(), res->second.end() - offset, res->second.end() - offset + length);
                if (caret < end)
                    res->second.push_back(*(caret++));
            }
        }
    }

    return res;
}



dataset* CompressLZSS(dataset* d, uint32_t buf_size)
{
    if (!d || !buf_size || (buf_size & (1UL << 31)))
        return NULL;
    dataset* res = new dataset { d->first, bytes() };
    uint8_t* caret = d->second.data(), * end = d->second.data() + d->second.size(), * beg = caret, * check, size = min_bytes(buf_size),
        minmatch = (buf_size & (0x80 << (size * 8 - 8)) ? ++size : size);
    if (!res)
        return NULL;
    res->first.insert(res->first.begin(), { TYPE_LZSS, uint8_t(4 + d->first.size()), uint8_t((4 + d->first.size()) >> 8), 0, 0, 0, 0 });
    reinterpret_cast<uint32_t&>(res->first[3]) = buf_size;
    union
    {
        struct
        {
            uint32_t lenm, offm, len, literal;
        };
        uint8_t buffer[16] {};
    };
    auto& r = res->second; 
    for (; caret < end; offm = lenm = 0)
    {
        for (check = caret - min(caret - beg, buf_size); check < caret; ++check)
        {
            for (len = 0; check + len < caret && caret + len < end && *(caret + len) == *(check + len) && len < buf_size; ++len);
            if (len > lenm)
                (lenm = len), (offm = caret - check);
        }
        if (lenm > minmatch)
        {
            if (literal)
            {
                r.insert(r.end(), buffer + 12, buffer + 12 + size), r.insert(r.end(), caret - literal, caret);
                literal = 0;
            }
            buffer[size - 1] |= 0x80;
            r.insert(r.end(), buffer, buffer + size), r.insert(r.end(), buffer + 4, buffer + 4 + size);
            buffer[size - 1] ^= 0x80;
        }
        else if ((literal += (lenm ? lenm : ++lenm)) + uint32_t(minmatch) + 1 >= buf_size)
        {
        WRITE_LITERAL:
            r.insert(r.end(), buffer + 12, buffer + 12 + size), r.insert(r.end(), caret - literal + lenm, caret + lenm);
            literal = 0;
        }
        caret += lenm;
    }
    if (literal)
        goto WRITE_LITERAL;
    return res;
}



dataset* UncompressLZSS(dataset* d)
{
    if (!d || d->first.size() < 7 || d->first[0] != TYPE_LZSS)
        return NULL;
    dataset* res = new dataset(bytes(d->first.begin() + 7, d->first.end()), bytes());
    uint32_t length {}, offset {}, flag, read;
    uint8_t* caret = d->second.data(), * end = d->second.data() + d->second.size(), size = min_bytes(reinterpret_cast<uint32_t&>(d->first[3]));
    if (!res)
        return NULL;
    flag = 0x80 << (8 * (reinterpret_cast<uint32_t&>(d->first[3]) & (0x80 << (size * 8 - 8)) ? size++ : (size - 1)));
    while (caret < end)
    {
        memcpy(&length, caret, (read = min(end - caret, size)));
        caret += read;
        if (read == size && length & flag)
        {
            memcpy(&offset, caret, (read = min(end - caret, size)));
            caret += read;
            if (read == size)
                res->second.insert(res->second.end(), res->second.end() - offset, res->second.end() - offset + (length ^ flag));
        }
        else
        {
            res->second.insert(res->second.end(), caret, caret + (read = min(end - caret, length)));
            caret += read;
        }
    }

    return res;
}



dataset* CompressLZ78(dataset* d, uint32_t dict_size)
{
    if (!d || !dict_size)
        return NULL;
    std::vector<bytes> keys { {} };
    std::vector<bytes>::iterator found;
    bytes current, candi;
    dataset* res = new dataset { d->first, bytes() };
    uint32_t next = 1, buf;
    uint8_t size = min_bytes(dict_size - 1);
    if (!res)
        return NULL;
    res->first.insert(res->first.begin(), { TYPE_LZ78, uint8_t(4 + d->first.size()), uint8_t((4 + d->first.size()) >> 8), 0, 0, 0, 0 });
    reinterpret_cast<uint32_t&>(res->first[3]) = dict_size;

    for (char c : d->second)
    {
        (candi = current).push_back(c);
        if ((found = std::find(keys.begin(), keys.end(), candi)) != keys.end())
            current = candi;
        else
        {
            buf = std::find(keys.begin(), keys.end(), current) - keys.begin();
            res->second.insert(res->second.end(), reinterpret_cast<uint8_t*>(&buf), reinterpret_cast<uint8_t*>(&buf) + size);
            res->second.push_back(c);
            if (next < dict_size)
                keys.push_back(candi), ++next;
            current.clear();
        }
    }
    if (!current.empty())
    {
        buf = std::find(keys.begin(), keys.end(), current) - keys.begin();
        res->second.insert(res->second.end(), reinterpret_cast<uint8_t*>(&buf), reinterpret_cast<uint8_t*>(&buf) + size);
    }

    return res;
}



dataset* UncompressLZ78(dataset* d)
{
    if (!d || d->first.size() < 7 || d->first[0] != TYPE_LZ78)
        return NULL;
    std::vector<bytes> values { {} };
    dataset* res = new dataset(bytes(d->first.begin() + 7, d->first.end()), bytes());
    uint32_t index {}, next = 1, dict_size = reinterpret_cast<uint32_t&>(d->first[3]);
    uint8_t* caret = d->second.data(), * end = d->second.data() + d->second.size(), read, size = min_bytes(dict_size - 1);
    if (!res)
        return NULL;

    while (caret < end)
    {
        memcpy(&index, caret, (read = min(end - caret, size)));
        caret += read;
        if (read == size)
        {
            if (index >= values.size())
                index = 0;
            res->second.insert(res->second.end(), values[index].begin(), values[index].end());
            if (caret < end)
            {
                res->second.push_back(*caret);
                if (next < dict_size)
                    values.push_back(values[index]), values.back().push_back(*caret), ++next;
                ++caret;
            }
            else if (next < dict_size)
                values.push_back(values[index]), ++next;
        }
    }

    return res;
}



dataset* CompressLZW(dataset* d, uint32_t dict_size)
{
    if (!d || !dict_size || dict_size < 256)
        return NULL;
    std::vector<bytes> keys(256);
    std::vector<bytes>::iterator found;
    bytes current, candi;
    dataset* res = new dataset { d->first, bytes() };
    uint32_t next = 1, buf;
    uint8_t size = min_bytes(dict_size - 1);
    if (!res)
        return NULL;
    for (next = 0; next < 256; ++next)
        keys[next] = { uint8_t(next) };
    res->first.insert(res->first.begin(), { TYPE_LZW, uint8_t(4 + d->first.size()), uint8_t((4 + d->first.size()) >> 8), 0, 0, 0, 0 });
    reinterpret_cast<uint32_t&>(res->first[3]) = dict_size;

    for (uint8_t c : d->second)
    {
        (candi = current).push_back(c);
        if ((found = std::find(keys.begin(), keys.end(), candi)) != keys.end())
            current = candi;
        else
        {
            buf = std::find(keys.begin(), keys.end(), current) - keys.begin();
            res->second.insert(res->second.end(), reinterpret_cast<uint8_t*>(&buf), reinterpret_cast<uint8_t*>(&buf) + size);
            if (next < dict_size)
                keys.push_back(candi), ++next;
            current = { c };
        }
    }
    if (!current.empty())
    {
        buf = std::find(keys.begin(), keys.end(), current) - keys.begin();
        res->second.insert(res->second.end(), reinterpret_cast<uint8_t*>(&buf), reinterpret_cast<uint8_t*>(&buf) + size);
    }

    return res;
}



dataset* UncompressLZW(dataset* d)
{
    if (!d || d->first.size() < 7 || d->first[0] != TYPE_LZW)
        return NULL;
    std::vector<bytes> values(256);
    bytes candi;
    dataset* res = new dataset(bytes(d->first.begin() + 7, d->first.end()), bytes());
    uint32_t prev {}, curr {}, dict_size = reinterpret_cast<uint32_t&>(d->first[3]);
    uint8_t* caret = d->second.data(), * end = d->second.data() + d->second.size(), read, size = min_bytes(dict_size - 1);
    for (curr = 0; curr < 256; ++curr)
        values[curr] = { uint8_t(curr) };
    if (!res)
        return NULL;

    memcpy(&prev, caret, (read = min(end - caret, size)));
    caret += read;
    if (read == size)
        res->second.insert(res->second.end(), values[prev].begin(), values[prev].end());
    for (curr = 0; caret < end;)
    {
        memcpy(&curr, caret, (read = min(end - caret, size)));
        caret += read;
        if (read == size)
        {
            if (curr < values.size())
                candi = values[curr];
            else
                (candi = values[prev]).push_back(values[prev][0]);
            res->second.insert(res->second.end(), candi.begin(), candi.end());
            if (values.size() < dict_size)
                values.push_back(values[prev]), values.back().push_back(candi[0]);
            prev = curr;
        }
    }

    return res;
}
