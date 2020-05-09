
#include "stdafx.h"

#include <Tlhelp32.h>

#include "common.h"
#include "compactuint.h"
#include "utils.h"
//#include "textconsole.h"


std::string wstrToStr(const std::wstring & wstr)
{
    std::string dest;

    if ( ! wstr.empty())
    {
        size_t wstrLength = wstr.length();

        dest.resize(wstrLength);

        BOOL    usedDefault;
        if (WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), -1, &dest[0], (int)wstrLength + 1, "?", &usedDefault) == 0)
        {
            dest = "[string cannot be displayed]";
        }
    }

    return dest;
}

std::wstring strToWStr(const std::string & str)
{
    std::wstring dest;

    if ( ! str.empty())
    {
        size_t strLength = str.length();
        dest.resize(strLength);

        if (MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, &dest[0], (int)strLength + 1) == 0)
        {
            dest = _T("[string cannot be displayed]");
        }
    }

    return dest;
}

std::ostream & operator << (std::ostream & out, const wchar_t *wstr)
{
    size_t wstrLength = wcslen(wstr);

    std::vector<char> dest(wstrLength + 1);
    BOOL    usedDefault;
    if (WideCharToMultiByte(CP_ACP, 0, wstr, -1, &dest.front(), (int)wstrLength + 1, "?", &usedDefault) == 0)
    {
        out << "[string cannot be displayed]";
    }
    else
    {
        out << &dest.front();
    }

    return out;
}

std::wostream & operator << (std::wostream & out, const char *str)
{
    size_t strLength = strlen(str);

    std::vector<wchar_t> dest(strLength + 1);
    if (MultiByteToWideChar(CP_ACP, 0, str, -1, &dest.front(), (int)strLength + 1) == 0)
    {
        out << L"[string cannot be displayed]";
    }
    else
    {
        out << &dest.front();
    }

    return out;
}

std::ostream & operator<<(std::ostream & output, const barray & data)
{
    if (data.size() == 0)
    {
        return output;
    }

    std::ios_base::fmtflags flags = output.flags();

    output << std::hex ;

    size_t  bytesPrinted = 0;
    barray::const_iterator it = data.begin();
    bool first = true;
    while (it != data.end())
    {
        if (!first)
        {
            output << ' ';
        }
        first = false;
        output << std::setw(2) << (unsigned)*it;
        ++bytesPrinted;
        ++it;
    }

    output.flags(flags);
    return output;
}

std::wostream & operator<<(std::wostream & output, const barray & data)
{
    if (data.size() == 0)
    {
        return output;
    }

    std::ios_base::fmtflags flags = output.flags();

    output << std::hex << std::setfill(L'0');

    size_t  bytesPrinted = 0;
    barray::const_iterator it = data.begin();
    bool first = true;
    while (it != data.end())
    {
        if (!first)
        {
            output << L' ';
        }
        first = false;
        output << std::setw(2) << (unsigned)*it;
        ++bytesPrinted;
        ++it;
    }

    output.flags(flags);
    return output;
}



#define BYTES_PER_LINE  16

size_t formatBytes(unsigned char *line, unsigned int lineSize, barray::const_iterator first, barray::const_iterator last)
{
    size_t  bytesPrinted = 0, posInLine, bytesInLine = 0;
    size_t  dataSize = last - first;

    line[0] = _T(' ');
    posInLine = 1;

    while (bytesPrinted < dataSize
            && bytesInLine < BYTES_PER_LINE)
    {
        posInLine += sprintf_s((char*)line + posInLine, lineSize - posInLine, " %02X", *(first + bytesPrinted));
        bytesPrinted++;
        bytesInLine++;
    }

    // print the chars
    while (posInLine < (2 + BYTES_PER_LINE * 3))
    {
        line[posInLine++] = ' ';
    }

    copy(first + bytesPrinted - bytesInLine, first + bytesPrinted, stdext::checked_array_iterator<unsigned char *>(line, lineSize, posInLine));

    // strip scary symbols
    for (unsigned char *p = line + posInLine; p < line + posInLine + bytesInLine; p++)
    {
        if (!isgraph(*p) || *p == 0x95)
        {
            *p = '.';
        }
    }

    line[posInLine + bytesInLine] = '\0';

    return bytesPrinted;
}

void dumpBytes(tostream & output, barray::const_iterator first, barray::const_iterator last)
{
    size_t  dataSize = last - first;
    if (dataSize == 0)
    {
        return;
    }

    unsigned char   line[5 + (BYTES_PER_LINE * 4)];         // _SXX(SXX)__.(.)

    size_t  bytesPrinted = 0;
    while (bytesPrinted < dataSize)
    {
        bytesPrinted += formatBytes(line, sizeof_array(line), first + bytesPrinted, last);
        output << line << std::endl;
    }
}

//------------------------------------------------------------------------------
// Get

uint64_t getQWORD(barray::const_iterator & position)
{
    uint64_t result = 0;
    result |= (uint64_t(*(position + 0)) << 56);
    result |= (uint64_t(*(position + 1)) << 48);
    result |= (uint64_t(*(position + 2)) << 40);
    result |= (uint64_t(*(position + 3)) << 32);
    result |= (uint64_t(*(position + 4)) << 24);
    result |= (uint64_t(*(position + 5)) << 16);
    result |= (uint64_t(*(position + 6)) << 8);
    result |=  uint64_t(*(position + 7));
    position += 8;
    return result;
}

uint64_t getQWORD_r(barray::const_iterator & position)
{
    DWORD d1 = getDWORD_r(position);
    DWORD d2 = getDWORD_r(position);
    return (uint64_t(d2) << 32) | d1;
//     uint64_t result = 0;
//     result |= (*(position + 7) << 56);
//     result |= (*(position + 6) << 48);
//     result |= (*(position + 5) << 40);
//     result |= (*(position + 4) << 32);
//     result |= (*(position + 3) << 24);
//     result |= (*(position + 2) << 16);
//     result |= (*(position + 1) << 8);
//     result |=  *(position + 0);
//     position += 8;
//     return result;
}

DWORD getDWORD(barray::const_iterator & position)
{
    DWORD result = ((*position) << 24) | (*(position + 1) << 16) | (*(position + 2) << 8) | *(position + 3);
    position += 4;
    return result;
}

DWORD getDWORD_r(barray::const_iterator & position)
{
    DWORD result = (*(position + 3) << 24) | (*(position + 2) << 16) | (*(position + 1) << 8) | *position;
    position += 4;
    return result;
}

WORD getWORD(barray::const_iterator & position)
{
    WORD result = (*position << 8) | (*(position + 1));
    position += 2;
    return result;
}

WORD getWORD_r(barray::const_iterator & position)
{
    WORD result = (*(position + 1) << 8) | *position;
    position += 2;
    return result;
}

float getFloat(barray::const_iterator & position)
{
    DWORD val = getDWORD(position);
    float *p = reinterpret_cast<float *>(&val);
    return *p;
}

float getFloat_r(barray::const_iterator & position)
{
    DWORD val = getDWORD_r(position);
    float *p = reinterpret_cast<float *>(&val);
    return *p;
}

std::string getString(const barray & data, barray::const_iterator & position)
{
    unsigned int nameLength = getCUI(data, position);
    if (nameLength == 0)
    {
        return std::string();
    }
    else
    {
        // trim trailing zero-bytes
        while (nameLength > 0 && position[nameLength - 1] == 0)
        {
            nameLength --;
        }
        std::string result(position, position + nameLength);
        position += nameLength;
        return result;
    }
}

std::string getStringWr(const barray & /*data*/, barray::const_iterator & position)
{
    unsigned int nameLength = getWORD_r(position);
    if (nameLength == 0)
    {
        return std::string();
    }
    else
    {
        // trim trailing zero-bytes
        while (nameLength > 0 && position[nameLength - 1] == 0)
        {
            nameLength--;
        }
        std::string result(position, position + nameLength);
        position += nameLength;
        return result;
    }
}

std::wstring getWString(const barray & data, barray::const_iterator & position)
{
    unsigned int nameLength = getCUI(data, position);
    if (nameLength == 0)
    {
        return std::wstring();
    }
    else
    {
        if (nameLength % 2 != 0)
        {
            assert(0);
            nameLength &= ~1;
        }

        // trim trailing zero-bytes
        unsigned actualLength = nameLength;
        while (actualLength > 0 && *(wchar_t*) &*(position + actualLength - 2) == L'\0')
        {
            actualLength -= 2;
        }
        std::wstring result((wchar_t*) &(*position), (wchar_t*) (&(*(position + actualLength - 1)) + 1)/*yes this looks like -1+1 but it's ok*/);
        position += nameLength;
        return result;
    }
}

void putQWORD(barray::iterator & position, uint64_t val)
{
    *position++ = byte((val >> 56) & 0xFF);
    *position++ = byte((val >> 48) & 0xFF);
    *position++ = byte((val >> 40) & 0xFF);
    *position++ = byte((val >> 32) & 0xFF);
    *position++ = byte((val >> 24) & 0xFF);
    *position++ = byte((val >> 16) & 0xFF);
    *position++ = byte((val >> 8) & 0xFF);
    *position++ = byte((val >> 0) & 0xFF);
}

void putQWORD_r(barray::iterator & position, uint64_t val)
{
    *position++ = byte((val >> 0) & 0xFF);
    *position++ = byte((val >> 8) & 0xFF);
    *position++ = byte((val >> 16) & 0xFF);
    *position++ = byte((val >> 24) & 0xFF);
    *position++ = byte((val >> 32) & 0xFF);
    *position++ = byte((val >> 40) & 0xFF);
    *position++ = byte((val >> 48) & 0xFF);
    *position++ = byte((val >> 56) & 0xFF);
}

//------------------------------------------------------------------------------
// Put

void putDWORD(barray::iterator & position, const DWORD val)
{
    *position++ = byte((val >> 24) & 0xFF);
    *position++ = byte((val >> 16) & 0xFF);
    *position++ = byte((val >>  8) & 0xFF);
    *position++ = byte((val >>  0) & 0xFF);
}

void putDWORD_r(barray::iterator & position, const DWORD val)
{
    *position++ = byte((val >>  0) & 0xFF);
    *position++ = byte((val >>  8) & 0xFF);
    *position++ = byte((val >> 16) & 0xFF);
    *position++ = byte((val >> 24) & 0xFF);
}

void putWORD(barray::iterator & position, const WORD val)
{
    *position++ = byte((val >> 8) & 0xFF);
    *position++ = byte((val >> 0) & 0xFF);
}

void putWORD_r(barray::iterator & position, const WORD val)
{
    *position++ = byte((val >> 0) & 0xFF);
    *position++ = byte((val >> 8) & 0xFF);
}

void putFloat(barray::iterator & position, const float val)
{
    const DWORD *p = reinterpret_cast<const DWORD *>(&val);
    putDWORD(position, *p);
}

void putFloat_r(barray::iterator & position, const float val)
{
    const DWORD *p = reinterpret_cast<const DWORD *>(&val);
    putDWORD_r(position, *p);
}

void putString(barray & data, barray::iterator & position, const std::string & val)
{
    size_t sizeInBytes = val.size();
    putCUI(position, sizeInBytes);
    assert(position + sizeInBytes <= data.end());
    std::copy(val.begin(), val.end(), position);
    position += sizeInBytes;
}

void putStringWr(barray & data, barray::iterator & position, const std::string & val)
{
    size_t sizeInBytes = val.size();
    putWORD_r(position, sizeInBytes);
    assert(position + sizeInBytes <= data.end());
    std::copy(val.begin(), val.end(), position);
    position += sizeInBytes;
}

void putWString(barray & data, barray::iterator & position, const std::wstring & val)
{
    size_t sizeInBytes = 2 * val.size();
    putCUI(position, sizeInBytes);
    assert(position + sizeInBytes <= data.end());
    std::copy(val.begin(), val.end(), (wchar_t *)&(*position));
    position += sizeInBytes;
}

//------------------------------------------------------------------------------
// Tools

bool tstrToBArray(tstring & str, barray & array)
{
    bool            result = true;
    const TCHAR    *ptr = str.c_str();
    const TCHAR    *end;
    TCHAR          *endptr = NULL;

    array.clear();

    if (str.length() == 0)
    {
        return true;
    }

    end = str.c_str() + str.length();

    do 
    {
        unsigned long value = _tcstoul(ptr, &endptr, 16);
        if (endptr != ptr && value <= 0xFF)
        {
            array.push_back((byte)value);
            ptr = endptr;
        }
        else
        {
            result = false;
            array.clear();
            break;
        }
    } while (ptr < end);

    return result;
}

DWORD bitCounter(DWORD N)
{ 
    N = (N & 0x55555555) + ((N >> 1) & 0x55555555); 
    N = (N & 0x33333333) + ((N >> 2) & 0x33333333); 
    N = (N & 0x0F0F0F0F) + ((N >> 4) & 0x0F0F0F0F); 
    N = (N & 0x00FF00FF) + ((N >> 8) & 0x00FF00FF); 
    N = (N & 0x0000FFFF) + ((N >> 16) & 0x0000FFFF); 
    return N; 
}     


void stringTrim(std::wstring & line)
{
    std::wstring whitespaces(L"\xFEFF\xFFFE \t\f\v\n\r");

    size_t first = line.find_first_not_of(whitespaces);
    if (first != std::string::npos)
    {
        if (first != 0)
        {
            line.erase(0, first);
        }

        size_t last = line.find_last_not_of(whitespaces);
        if (last != std::string::npos)
        {
            if ((last + 1) != line.size())
            {
                line.erase(last + 1);
            }
        }
        else
        {
            line.clear();
        }
    }
    else
    {
        line.clear();
    }
}
