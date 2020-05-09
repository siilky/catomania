// $Id: compactuint.cpp 935 2013-05-12 14:07:16Z jerry $
//
#include "stdafx.h"

#include "utils.h"
#include "compactuint.h"
#include "netdata/fragment.h"


bool getCUI_safe(const barray & data, barray::const_iterator & position, __notnull unsigned int *value)
{
    assert(value != NULL);

    unsigned int result;

    size_t bytesLeft = data.end() - position;
    if (bytesLeft == 0)
    {
        return false;
    }

    byte firstByte = *position;
    if (firstByte < 0x80)
    {
        ++position;
        result = firstByte & 0x7F;
    }
    else if (firstByte < 0xC0)
    {
        if (bytesLeft < 2) return false;

        result = getWORD(position) & 0x3FFF;
    }
    else if (firstByte < 0xE0)
    {
        if (bytesLeft < 4) return false;

        result = getDWORD(position) & 0x1FFFFFFF;
    }
    else
    {
        if (bytesLeft < 5) return false;

        result = getDWORD(++position);
    }

    *value = result;
    return true;
}

unsigned int getCUI(const barray &data, barray::const_iterator & position)
{
    if (position == data.end()) throw ePackedIntUnderflow;

    size_t  bytesLeft = data.end() - position;

    byte firstByte = *position;
    if (firstByte < 0x80)
    {
        ++position;
        return firstByte & 0x7F;
    }
    else if (firstByte < 0xC0)
    {
        if (bytesLeft < 2) throw ePackedIntUnderflow;

        return getWORD(position) & 0x3FFF;
    }
    else if (firstByte < 0xE0)
    {
        if (bytesLeft < 4) throw ePackedIntUnderflow;

        return getDWORD(position) & 0x1FFFFFFF;
    }
    else
    {
        if (bytesLeft < 5) throw ePackedIntUnderflow;

        return getDWORD(++position);
    }
}

bool peekCUI_safe(const barray &data, barray::const_iterator position, __notnull unsigned int *value)
{
    return getCUI_safe(data, position, value);
}

unsigned int peekCUI(const barray &data, barray::const_iterator position)
{
    return getCUI(data, position);
}

//---

unsigned putCUI(barray::iterator & position, unsigned int val)
{
    if (val < 0x80)
    {
        *position++ = byte(val);
        return 1;
    }
    else if (val < 0x4000)
    {
        *position++ = byte((val >> 8) & 0xFF) | 0x80;
        *position++ = byte((val >> 0) & 0xFF);
        return 2;
    }
    else if (val < 0x20000000)
    {
        *position++ = byte((val >> 24) & 0xFF) | 0xC0;
        *position++ = byte((val >> 16) & 0xFF);
        *position++ = byte((val >>  8) & 0xFF);
        *position++ = byte((val >>  0) & 0xFF);
        return 4;
    }
    else
    {
        *position++ = 0xE0;
        putDWORD(position, val);
        return 5;
    }
}

unsigned getPackedCUISize( unsigned int val )
{
    if (val < 0x80)
    {
        return 1;
    }
    else if (val < 0x4000)
    {
        return 2;
    }
    else if (val < 0x20000000)
    {
        return 4;
    }
    else
    {
        return 5;
    }
}