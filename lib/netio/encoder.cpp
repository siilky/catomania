
#include "stdafx.h"

#include "netio/encoder.h"

void RC4Encoder::init(const barray & key)
{
    for (int i = 0; i < sizeof(matrix); i++)
    {
        matrix[i] = byte(i);
    }

    for (int i = 0, j = 0; i < 256; i++)
    {
        j = (j + key[i % key.size()] + matrix[i]) & 0xFF;
        std::swap(matrix[i], matrix[j]);
    }

    offset = offset2 = 0;
}

void RC4Encoder::encode(barray & data)
{
    for (unsigned int i = 0; i < data.size(); i++)
    {
        offset ++;
        offset2 += matrix[offset];
        std::swap(matrix[offset2], matrix[offset]);

        data[i] ^= matrix[byte(matrix[offset] + matrix[offset2])];
    }
}

#if defined DEBUG
bool RC4Encoder::test()
{
    RC4Encoder enc;

    byte init[] = {0x8f, 0xda, 0xba, 0x73, 0xe5, 0x29, 0x6d, 0xb2, 0x36, 0x19, 0x73, 0xca, 0x97, 0xd4, 0x8a, 0xbf};
    barray bInit(init, init + sizeof(init));

    enc.init(bInit);

    byte test[] = {0x02,0x12,0x10,0xe2,0x33,0xd8,0x84,0xd9,0x89,0xea,0xb4,0x90,0x10,0x69,0x66,0xdb,0x61,0x6c,0xcd,0x00};
    barray bTest(test, test + sizeof(test));

    enc.encode(bTest);

    byte result[] = {0xa2,0x73,0xcb,0xe4,0xd4,0x2a,0xf4,0x4b,0x6e,0x7c,0x98,0x08,0x57,0x4f,0xce,0xf9,0x2e,0x9f,0x71,0x74};

    return memcmp(result, &bTest[0], bTest.size()) == 0;
}
#endif