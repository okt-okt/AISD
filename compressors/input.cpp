#include "compress.h"




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



int SaveToFile(dataset* d, const wchar_t* file)
{
    if (!d || !file)
        return 1;
    FILE* f {};
    int hr = 0;
    uint32_t start = 0;

    if (d->first.size())
        switch (d->first[0])
        {
            case TYPE_Text:
                if (ends_with(file, L".TXT", false))
                    start = 3;
                else if (!ends_with(file, L".AISD.RAW", false))
                    hr = 3;
                break;

            default:
                if (!ends_with(file, L".AISD.RAW", false))
                    hr = 3;
        }

    if (!hr && (_wfopen_s(&f, file, L"wb") ||
        (d->first.size() > start && fwrite(d->first.data() + start, 1, d->first.size() - start, f) < d->first.size() - start) ||
        (d->second.size() && fwrite(d->second.data(), 1, d->second.size(), f) < d->second.size())
        ))
        hr = 2;
    if (f)
        fclose(f);
    return hr;
}



dataset* LoadFromFile(const wchar_t* file)
{
    if (!file)
        return NULL;
    FILE* f {};
    dataset* res = new dataset();
    int hr = res ? 0 : 2;
    uint32_t read;
    uint8_t buffer[256] = { TYPE_Text, 0, 0 };

    if (!hr && _wfopen_s(&f, file, L"rb"))
        hr = 1;
    if (!hr && ends_with(file, L".TXT", false))
        res->first.insert(res->first.begin(), buffer, buffer + 3);
    else if (!hr && ends_with(file, L".AISD.RAW", false))
    {
        if (fread_s(buffer, 3, 1, 3, f) < 3)
            hr = 3;
        else
        {
            res->first.insert(res->first.begin(), buffer, buffer + 3);
            res->first.resize(3 + (read = *reinterpret_cast<uint16_t*>(buffer + 1)));
            if (read && fread_s(res->first.data() + 3, read, 1, read, f) < read)
                hr = 3;
        }
    }
    if (!hr)
        do
            if (read = fread_s(buffer, 256, 1, 256, f))
                res->second.insert(res->second.end(), buffer, buffer + read);
        while (read == 256);

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
