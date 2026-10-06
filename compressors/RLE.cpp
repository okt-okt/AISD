#include "compress.h"




dataset* CompressRLE(dataset* d, uint32_t ctrl_size, uint32_t symb_size)
{
    if (!d || (symb_size ? 0 : !d->first.size() || d->first[0] != TYPE_Text) || !(1 <= ctrl_size && ctrl_size <= 8 && symb_size <= 8))
        return NULL;
    uint64_t ctrl = 2 + d->first.size(), const ctrl_max = UINT64_MAX >> (8 * (8 - ctrl_size) + 1);
    dataset* res = new dataset { d->first, bytes()};
    uint8_t* caret = d->second.data(), * start, * end = d->second.data() + d->second.size(), read, read2;
    union
    {
        uint64_t symbols[2];
        uint8_t buffer[16];
    };
    if (!res)
        return NULL;
    res->first.insert(res->first.begin(), { TYPE_RLE, uint8_t(ctrl), uint8_t(ctrl >> 8), uint8_t(ctrl_size), uint8_t(symb_size) });
    bytes& r = res->second;

    auto read_next_symbol = [&]() -> uint64_t
        {
            if (caret >= end)
                return read = 0;
            uint64_t check = symb_size ? symb_size : *caret & 0xF8;
            if (symb_size)
                goto READ_SYMBOL;
            if (check & 0x80)
            {
                if (check == 0xC0)
                    check = 2;
                else if (check == 0xE0)
                    check = 3;
                else if (check == 0xF0)
                    check = 4;
                else
                    check = 1;
            READ_SYMBOL:
                memcpy(&check, caret, (read = min(end - caret, check)));
                caret += read;
                return check;
            }
            else
                return ((read = 1), *(caret++));
        };

    while (caret < end)
    {
        symbols[0] = read_next_symbol();
        read2 = read;
        symbols[1] = read_next_symbol();
        if (symb_size ? read2 < symb_size || read < symb_size : !read2 || !read)
        {
            ctrl = ctrl_max + 1 + 2;
            r.insert(r.end(), reinterpret_cast<uint8_t*>(&ctrl), reinterpret_cast<uint8_t*>(&ctrl) + ctrl_size);
            r.insert(r.end(), buffer, buffer + read2);
            r.insert(r.end(), buffer + 8, buffer + 8 + read);
        }
        else if (symbols[0] != symbols[1])
        {
            start = caret - read - read2;
            for (ctrl = 2;;)
            {
                symbols[ctrl & 1] = read_next_symbol();
                if (symb_size ? read == symb_size : read)
                {
                    if (symbols[0] != symbols[1])
                    {
                        if (++ctrl == ctrl_max)
                            break;
                    }
                    else
                    {
                        --ctrl;
                        caret -= read * 2;
                        break;
                    }
                }
                else
                {
                    ++ctrl;
                    break;
                }
            }
            ctrl += ctrl_max + 1;
            r.insert(r.end(), reinterpret_cast<uint8_t*>(&ctrl), reinterpret_cast<uint8_t*>(&ctrl) + ctrl_size);
            r.insert(r.end(), start, caret);
        }
        else
        {
            ctrl = 2;
            do
                symbols[1] = read_next_symbol();
            while ((symb_size ? read == symb_size : read) && symbols[0] == symbols[1] && ++ctrl < ctrl_max);
            if (ctrl < ctrl_max)
                caret -= read;
            r.insert(r.end(), reinterpret_cast<uint8_t*>(&ctrl), reinterpret_cast<uint8_t*>(&ctrl) + ctrl_size);
            r.insert(r.end(), buffer, buffer + read2);
        }
    }

    return res;
}



dataset* UncompressRLE(dataset* d)
{
    if (!d || d->first.size() < 5 || d->first[0] != TYPE_RLE)
        return NULL;
    dataset* res = new dataset();
    uint32_t const ctrl_size = d->first[3], const symb_size = d->first[4];
    uint64_t ctrl {}, read, const flag = 1ULL << (8 * ctrl_size - 1);
    uint8_t* caret = d->second.data(), * end = d->second.data() + d->second.size();
    union
    {
        uint64_t symbol;
        uint8_t buffer[8];
    };
    if (!res)
        return NULL;
    bytes& r = res->second;

    auto read_next_symbol = [&]() -> uint64_t
        {
            if (caret >= end)
                return read = 0;
            uint64_t check = symb_size ? symb_size : *caret & 0xF8;
            if (symb_size)
                goto READ_SYMBOL;
            if (check & 0x80)
            {
                if (check == 0xC0)
                    check = 2;
                else if (check == 0xE0)
                    check = 3;
                else if (check == 0xF0)
                    check = 4;
                else
                    check = 1;
            READ_SYMBOL:
                memcpy(&check, caret, (read = min(end - caret, check)));
                caret += read;
                return check;
            }
            else
                return ((read = 1), *(caret++));
        };

    while (caret < end)
    {
        memcpy(&ctrl, caret, (read = min(end - caret, ctrl_size)));
        caret += read;
        if (read == ctrl_size)
        {
            if (ctrl & flag)
            {
                if (symb_size)
                {
                    r.insert(r.end(), caret, caret + (read = min(end - caret, (ctrl - flag) * symb_size)));
                    caret += read;
                }
                else
                    for (ctrl -= flag; caret < end && ctrl; --ctrl)
                        (symbol = read_next_symbol()), r.insert(r.end(), buffer, buffer + read);
            }
            else if ((symbol = read_next_symbol()), (symb_size ? read == symb_size : read))
                for (; ctrl; --ctrl)
                    r.insert(r.end(), buffer, buffer + read);
        }
    }

    res->first.insert(res->first.begin(), d->first.begin() + 5, d->first.end());
    return res;
}
