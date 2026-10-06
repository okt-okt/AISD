#include "compress.h"
#include <algorithm>




struct tree
{
    struct code
    {
        uint32_t bits;
        uint16_t len;
        uint8_t symb;
    };

    tree* l, * r;
    uint32_t freq;
    uint8_t symb;

    tree() : freq(0), symb(0), l(NULL), r(NULL)
    {}
    tree(uint8_t symb, uint32_t freq) : freq(freq), symb(symb), l(NULL), r(NULL)
    {}
    tree(tree* l, tree* r) : freq(l->freq + r->freq), symb(0), l(l), r(r)
    {}

    void calc(code table[256], uint32_t pbits = 0, uint16_t len = 0)
    {
        if (l || r)
            l->calc(table, (pbits << 1), len + 1), r->calc(table, (pbits << 1) | 1, len + 1);
        else
            (table[symb].bits = pbits), (table[symb].len = len);
    }

    void lens(code table[256], uint16_t& max_len, uint16_t inherited_len = 0)
    {
        if (l || r)
            l->lens(table, max_len, ++inherited_len), r->lens(table, max_len, inherited_len);
        else if (max_len < (table[symb].len = inherited_len))
            max_len = inherited_len;
    }

    uint32_t construct(bytes& data, uint32_t pos)
    {
        if (data[pos])
            symb = data[++pos];
        else
            (pos = (l = new tree())->construct(data, pos + 1)), (pos = (r = new tree())->construct(data, pos + 1));
        return pos;
    }

    void destruct(bytes* out)
    {
        if (l || r)
        {
            if (out)
                out->push_back(0);
            l->destruct(out), r->destruct(out);
        }
        else
        {
            if (out)
                out->push_back(1), out->push_back(symb);
            delete this;
        }
    }
};



void append_tree(std::vector<tree*>& list, tree* root)
{
    std::vector<tree*>::iterator beg = list.begin(), end = list.end(), caret = beg + list.size() / 2;

    while (caret != beg && caret != end)
    {
        if (root->freq < (*caret)->freq)
            end = caret;
        else
            beg = caret;
        caret = beg + (end - beg) / 2;
    }
    list.insert((caret != list.end() && (*caret)->freq < root->freq ? end : beg), root);
}



void build_Canon(tree::code table[256], uint16_t max_len)
{
    uint32_t* buffer = new uint32_t[max_len] {}, i, code = 0, & buf = buffer[max_len - 1];
    for (i = 0; i < 256; ++i)
        if (table[i].len)
            ++(buffer[table[i].len - 1]);
    for (i = 0; i < max_len; ++i)
    {
        buf = buffer[i];
        buffer[i] = code;
        code = (code + buf) << 1;
    }
    for (i = 0; i < 256; ++i)
        if (table[i].len)
            table[i].bits = buffer[table[i].len - 1]++;
    delete[] buffer;
}



void group_lens(tree::code table[256], uint16_t max_len)
{
    tree::code input[256];
    uint32_t* indices = new uint32_t[++max_len] {}, i;
    memcpy(input, table, sizeof(tree::code) * 256);
    for (i = 0; i < 256; ++i)
        ++(indices[input[i].len]);
    for (i = 1; i < max_len; ++i)
        indices[i] += indices[i - 1];
    for (i = 255; i < 256; --i)
        table[--indices[input[i].len]] = input[i];
    for (i = 1; i < max_len; ++i)
        table[i - 1].len = indices[i];
    table[i - 1].len = 256;
    delete[] indices;
}


tree::code saved[256];


dataset* CompressHA(dataset* d, bool use_Canon)
{
    if (!d)
        return NULL;
    dataset* res = new dataset { { (use_Canon ? TYPE_HA_Canon : TYPE_HA_Tree), 0, 0, 0 }, bytes() };
    if (!res)
        return NULL;
    std::vector<tree*> list;
    list.reserve(256);
    tree* tree_ptr {};
    union
    {
        uint32_t freq[256] {};
        tree::code table[256];
    };
    uint16_t m, max_len {};

    for (auto c : d->second)
        ++freq[c];
    for (m = 0; m < 256; ++m)
        if (freq[m])
            append_tree(list, new tree(m, freq[m]));
    while (list.size() > 1)
    {
        tree_ptr = new tree(list[0], list[1]);
        list.erase(list.begin()); list.erase(list.begin());
        append_tree(list, tree_ptr);
    }

    for (m = 0; m < 256; ++m)
        table[m] = { 0, 0, uint8_t(m) };
    if (use_Canon && tree_ptr)
    {
        tree_ptr->lens(table, max_len);
        for (m = 0; m < 256; ++m)
            res->first.push_back(table[m].len);
        build_Canon(table, max_len);
        memcpy(saved, table, sizeof(table));
    }
    else if (tree_ptr)
        tree_ptr->calc(table);
    if (tree_ptr)
        tree_ptr->destruct(use_Canon ? NULL : &(res->first));
        
    m = 0;
    for (auto c : d->second)
        bitwrite(res->second, table[c].bits, table[c].len, m);

    res->first[3] = bitwrite(res->second, 0, 0, m);
    reinterpret_cast<uint16_t&>(res->first[1]) = res->first.size() - 3 + d->first.size();
    res->first.insert(res->first.end(), d->first.begin(), d->first.end());

    return res;
}



dataset* UncompressHA(dataset* d)
{
    if (!d || !d->first.size() || (d->first[0] != TYPE_HA_Tree && d->first[0] != TYPE_HA_Canon))
        return NULL;
    dataset* res = new dataset();
    union
    {
        struct
        {
            tree* codes, * ptr;
        };
        struct
        {
            uint32_t code;
            uint16_t len, m, max_len;
        };
    };
    if (!res)
        return NULL;
    codes = ptr = (d->first[0] == TYPE_HA_Tree && d->first.size() > 4 ? new tree() : NULL);
    uint8_t* caret = d->second.data(), * end = caret + d->second.size(), * last = (d->second.size() ? caret + d->second.size() - 1 : caret), flag = 0x80, last_flag = 0x101UL >> d->first[3];
    if (d->first[0] == TYPE_HA_Tree)
    {
        if (!codes)
        {
            res->first.insert(res->first.end(), d->first.begin() + 4, d->first.end());
            return res;
        }
        res->first.insert(res->first.end(), d->first.begin() + codes->construct(d->first, 4) + 1, d->first.end());

        while (caret < end)
            if (caret != last)
            {
                for (; flag; flag >>= 1)
                {
                    if (!ptr->l && !ptr->r)
                        res->second.push_back(ptr->symb), (ptr = codes);
                    ptr = (*caret & flag ? ptr->r : ptr->l);
                }
                flag = 0x80;
                ++caret;
            }
            else
                for (;;)
                {
                    if (!ptr->l && !ptr->r)
                        res->second.push_back(ptr->symb), (ptr = codes);
                    ptr = (*caret & flag ? ptr->r : ptr->l);

                    if (flag == last_flag)
                    {
                        if (!ptr->l && !ptr->r)
                            res->second.push_back(ptr->symb);
                        ++caret;
                        break;
                    }
                    flag >>= 1;
                }
        if (codes)
            codes->destruct(NULL);
    }
    else
    {
        tree::code table[256] {};
        res->first.insert(res->first.end(), d->first.begin() + 4 + 256, d->first.end());
        code = len = max_len = 0;
        for (m = 0; m < 256; ++m)
            max_len = max((table[m] = { 0, d->first[4 + m], uint8_t(m) }).len, max_len);
        build_Canon(table, max_len);
        group_lens(table, max_len);

        while (caret < end)
            if (caret != last)
            {
                for (; flag; flag >>= 1)
                {
                    code = (code << 1) | (*caret & flag ? 1 : 0);
                    for (m = table[len++].len; m < table[len].len; ++m)
                        if (table[m].bits == code)
                        {
                            res->second.push_back(table[m].symb), (code = len = 0);
                            break;
                        }
                }
                flag = 0x80;
                ++caret;
            }
            else
                for (;;)
                {
                    code = (code << 1) | (*caret & flag ? 1 : 0);
                    for (m = table[len++].len; m < table[len].len; ++m)
                        if (table[m].bits == code)
                        {
                            res->second.push_back(table[m].symb), (code = len = 0);
                            break;
                        }

                    if (flag == last_flag)
                    {
                        ++caret;
                        break;
                    }
                    flag >>= 1;
                }
    }
    return res;
}
