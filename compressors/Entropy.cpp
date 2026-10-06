#include "compress.h"
#include <map>




double Entropy(dataset* d, uint32_t symb_size)
{
    if (!d || !symb_size || symb_size > 8)
        return -INFINITY;
    std::map<uint64_t, uint32_t> Ni;
    uint64_t buf, Nall = 0;
    double result = 0.;
    uint8_t* caret = d->second.data(), * end = d->second.data() + d->second.size(), read;

    while (caret < end)
    {
        memcpy(&buf, caret, (read = min(end - caret, symb_size)));
        caret += read;
        ++(Ni[buf]), ++Nall;
    }
    for (auto& x : Ni)
        result += double(x.second) * log2(double(Nall) / double(x.second));

    return Nall ? result / double(Nall) : 0.;
}
