#include "compress.h"




dataset* TransformMTF(dataset* d)
{
    if (!d)
        return NULL;
    dataset* res = new dataset { d->first, bytes() };
    uint32_t freq[256] {};
    uint8_t symbols[256] {}, * caret;
    for (; symbols[255] < 255; ++symbols[255])
        symbols[symbols[255]] = symbols[255];
    for (auto s : d->second)
        ++freq[s];
    
    if (!res)
        return NULL;
    res->first.insert(res->first.begin(), { TYPE_MTF, uint8_t(1 + d->first.size()), uint8_t((1 + d->first.size()) >> 8), 0 });
    res->second.reserve(d->second.size());

    for (auto s : d->second)
    {
        for (caret = symbols; *caret != s; ++caret);
        res->second.push_back(uint8_t(caret - symbols));
        for (; caret != symbols; --caret)
            *caret = *(caret - 1);
        *symbols = s;
    }

    return res;
}



dataset* DetransformMTF(dataset* d)
{
    if (!d || !d->first.size() || d->first[0] != TYPE_MTF)
        return NULL;
    dataset* res = new dataset { bytes(d->first.begin() + 4, d->first.end()), bytes() };
    uint8_t symbols[256] {}, * caret;
    for (; symbols[255] < 255; ++symbols[255])
        symbols[symbols[255]] = symbols[255];
    if (!res)
        return NULL;
    res->second.reserve(d->second.size());

    for (auto s : d->second)
    {
        res->second.push_back(s = *(caret = symbols + s));
        for (; caret != symbols; --caret)
            *caret = *(caret - 1);
        *symbols = s;
    }

    return res;
}
