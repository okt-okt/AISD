#include "compress.h"
#include <algorithm>




std::vector<bytes> count_sort(std::vector<bytes>& matr, uint32_t col, uint32_t* save)
{
    std::vector<bytes> res(matr.size());
    uint32_t offsets[256] {}, i;
    for (i = 0; i < matr.size(); ++i)
        ++(offsets[matr[i][col]]);
    for (i = 1; i < 256; ++i)
        offsets[i] += offsets[i - 1];
    for (i = matr.size() - 1; i != -1; --i)
    {
        res[--offsets[matr[i][col]]] = matr[i];
        if (save && i == *save)
            (*save = offsets[matr[i][col]]), (save = NULL);
    }
    return res;
}



std::pair<bytes, uint32_t> BWT(bytes block)
{
    std::vector<bytes> matr(block.size());
    uint32_t orig = 0, i;
    matr[0] = block;
    for (i = 1; i < block.size(); ++i)
        (matr[i] = bytes(matr[0].end() - i, matr[0].end())), matr[i].insert(matr[i].end(), matr[0].begin(), matr[0].end() - i);
    for (i = block.size() - 1; i != -1; --i)
        matr = count_sort(matr, i, &orig);
    for (uint32_t i = 0; i < block.size(); ++i)
        matr[0][i] = { matr[i][block.size() - 1] };
    return { matr[0], orig };
}



std::pair<bytes, uint32_t> BWT2(bytes block)
{
    bytes res(block.size());
    uint32_t* suffixes = new uint32_t[block.size()];
    uint32_t index, i, pos;
    uint8_t ca, cb;
    for (i = 0; i < block.size(); ++i)
        suffixes[i] = i;
    std::sort(suffixes, suffixes + block.size(), [&](uint32_t a, uint32_t b)
        {
            for (i = 0; i < block.size(); ++i)
                if ((ca = block[(a + i) % block.size()]) != (cb = block[(b + i) % block.size()]))
                    return ca < cb;
            return false;
        });

    for (pos = i = 0; i < block.size(); ++i)
    {
        if (suffixes[i])
            res[pos++] = block[suffixes[i] - 1];
        else
            res[index = pos++] = block[block.size() - 1];
    }
    delete[] suffixes;
    return { res, index };
}



bytes iBWT(bytes& block, uint32_t index)
{
    std::vector<bytes> matr(block.size(), bytes(block.size()));
    uint32_t unfilled = block.size() - 1, i;
    while (unfilled != -1)
    {
        for (i = 0; i < block.size(); ++i)
            matr[i][unfilled] = block[i];
        for ((i = block.size() - 1), --unfilled; i != unfilled; --i)
            matr = count_sort(matr, i, NULL);
    }
    return matr[index];
}



bytes iBWT2(bytes& block, uint32_t index)
{
    bytes res(block.size());
    uint32_t count[256] {}, * LF = new uint32_t[block.size()] {}, i, sum, buf;
    for (i = 0; i < block.size(); ++i)
        ++count[block[i]];
    for (sum = i = 0; i < 256; ++i)
        (buf = count[i]), (count[i] = sum), (sum += buf);
    for (i = 0; i < block.size(); ++i)
        LF[i] = count[block[i]]++;
    res[block.size() - 1] = block[buf = index];
    for (i = block.size() - 2; i != -1; --i)
        res[i] = block[buf = LF[buf]];
    delete[] LF;
    return res;
}



dataset* TransformBWT(dataset* d, uint32_t block_size, bool use_BWT2)
{
    if (!d || !block_size || block_size == -1)
        return NULL;
    dataset* res = new dataset { d->first, bytes() };
    std::pair<bytes, uint32_t> buf;
    uint32_t read;
    uint8_t* caret = d->second.data(), * end = d->second.data() + d->second.size(), size = min_bytes(block_size);
    if (!res)
        return NULL;
    res->first.insert(res->first.begin(), { TYPE_BWT, uint8_t(4 + d->first.size()), uint8_t((4 + d->first.size()) >> 8), 0, 0, 0, 0 });
    reinterpret_cast<uint32_t&>(res->first[3]) = block_size;

    while (caret < end)
    {
        buf = (use_BWT2 ? BWT2(bytes(caret, caret + (read = min(end - caret, block_size)))) : BWT(bytes(caret, caret + (read = min(end - caret, block_size)))));
        caret += read;
        res->second.insert(res->second.end(), reinterpret_cast<uint8_t*>(&(buf.second)), reinterpret_cast<uint8_t*>(&(buf.second)) + size);
        res->second.insert(res->second.end(), buf.first.begin(), buf.first.end());
    }
    return res;
}



dataset* DetransformBWT(dataset* d, bool use_iBWT2)
{
    if (!d || d->first.size() < 7 || d->first[0] != TYPE_BWT)
        return NULL;
    dataset* res = new dataset(bytes(d->first.begin() + 7, d->first.end()), bytes());
    uint32_t index {}, block_size = reinterpret_cast<uint32_t&>(d->first[3]), read;
    bytes buf(block_size);
    uint8_t* caret = d->second.data(), * end = d->second.data() + d->second.size(), size = min_bytes(block_size);
    if (!res)
        return NULL;

    while (caret < end)
    {
        memcpy(&index, caret, (read = min(end - caret, size)));
        caret += read;
        if (read == size)
        {
            memcpy(buf.data(), caret, (read = min(end - caret, block_size)));
            caret += read;
            if (read < block_size)
                buf.resize(read);
            if (read)
            {
                buf = (use_iBWT2 ? iBWT2(buf, index) : iBWT(buf, index));
                res->second.insert(res->second.end(), buf.begin(), buf.end());
            }
        }
    }
    return res;
}
