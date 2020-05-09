#include "stdafx.h"

#include "tea.h"


static void tea_encrypt(uint32_t* v, const uint32_t* k)
{
    /* set up */
    uint32_t v0 = v[0];
    uint32_t v1 = v[1];
    uint32_t sum = 0;

    /* a key schedule constant */
    uint32_t delta = 0x9e3779b9;

    /* cache key */
    uint32_t k0 = k[0];
    uint32_t k1 = k[1];
    uint32_t k2 = k[2];
    uint32_t k3 = k[3];

    /* basic cycle start */
    for (uint32_t i = 0; i < 16; i++)
    {
        sum += delta;
        v0 += ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
        v1 += ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
    }
    /* end cycle */

    v[0] = v0;
    v[1] = v1;
}

void tea_encrypt(barray & data, const uint32_t key[4])
{
    unsigned offset = 0;

    for (; (offset + 8) < data.size(); offset += 8)
    {
        tea_encrypt((uint32_t*)(data.data() + offset), key);
    }
    if (offset < data.size())
    {
        auto remains = data.size() - offset;

        uint32_t v[2] = {0, 0};
        std::copy(data.begin() + offset, data.end(), (char*)v);
        tea_encrypt(v, key);
        std::copy((char*)v, (char*)v + remains, data.begin() + offset);
    }
}

static void tea_decrypt(uint32_t* v, const uint32_t* k)
{
    /* set up */
    uint32_t v0 = v[0];
    uint32_t v1 = v[1];
    uint32_t sum = 0xE3779B90/*0xC6EF3720*/;

    /* a key schedule constant */
    uint32_t delta = 0x9e3779b9;

    /* cache key */
    uint32_t k0 = k[0];
    uint32_t k1 = k[1];
    uint32_t k2 = k[2];
    uint32_t k3 = k[3];

    /* basic cycle start */
    for (uint32_t i = 0; i < 16; i++)
    {
        v1 -= ((v0 << 4) + k2) ^ (v0 + sum) ^ ((v0 >> 5) + k3);
        v0 -= ((v1 << 4) + k0) ^ (v1 + sum) ^ ((v1 >> 5) + k1);
        sum -= delta;
    }
    /* end cycle */

    v[0] = v0;
    v[1] = v1;
}

void tea_decrypt(barray & data, const uint32_t key[4])
{
    unsigned offset = 0;
    
    for (; (offset + 8) < data.size(); offset += 8)
    {
        tea_decrypt((uint32_t*)(data.data() + offset), key);
    }
    if (offset < data.size())
    {
        auto remains = data.size() - offset;

        uint32_t v[2] = { 0, 0 };
        std::copy(data.begin() + offset, data.end(), (char*)v);
        tea_decrypt(v, key);
        std::copy((char*)v, (char*)v + remains, data.begin() + offset);
    }
}
