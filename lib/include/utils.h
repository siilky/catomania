#ifndef utils_h
#define utils_h

#include <tchar.h>
#include "common.h"

std::string wstrToStr(const std::wstring & str);
std::wstring strToWStr(const std::string & str);

inline tstring strToTStr(const std::string & str)
{
#ifdef _UNICODE
    return strToWStr(str);
#else
    return str;
#endif
}

inline std::string tstrToStr(const tstring & str)
{
#ifdef _UNICODE
    return wstrToStr(str);
#else
    return str;
#endif
}

inline std::wstring tstrToWStr(const tstring & str)
{
#ifdef _UNICODE
    return str;
#else
    // TBD
    (void)str;
    assert(0);
#endif
}

inline tstring wstrToTStr(const std::wstring & str)
{
#ifdef _UNICODE
    return str;
#else
    // TBD
    (void)str;
    assert(0);
#endif
}

inline std::string toString(int val)
{
    char buf[32] = "";

    _ltoa_s(val, buf, 10);
    return std::string(buf);
}

inline std::wstring toWstring(int val)
{
    wchar_t buf[32] = L"";

    _ltow_s(val, buf, 10);
    return std::wstring(buf);
}

// inline const tstring wstrToTStr(const std::wstring & str)
// {
// #ifdef _UNICODE
//     return str;
// #else
//     // TBD
//     assert(0);
// #endif
// }

std::ostream & operator<<(std::ostream & out, const wchar_t *wstr);
std::wostream & operator<<(std::wostream & out, const char *str);

inline std::wostream & operator<<(std::wostream & out, const unsigned char *str)
{
    return out << (char*)str;
}

inline std::ostream & operator<<(std::ostream & out, std::wstring & wstr)
{
    return out << wstr.c_str();
}

inline std::wostream & operator<<(std::wostream & out, std::string & str)
{
    return out << str.c_str();
}

size_t formatBytes(unsigned char *line, unsigned int lineSize, barray::const_iterator first, barray::const_iterator last);
void dumpBytes(tostream & output, barray::const_iterator first, barray::const_iterator last);

std::ostream & operator<<(std::ostream & output, const barray & data);
std::wostream & operator<<(std::wostream & output, const barray & data);

// *** Get functions ***

uint64_t getQWORD(barray::const_iterator & position);
uint64_t getQWORD_r(barray::const_iterator & position);
DWORD getDWORD(barray::const_iterator & position);
DWORD getDWORD_r(barray::const_iterator & position);
WORD  getWORD(barray::const_iterator & position);
WORD  getWORD_r(barray::const_iterator & position);
float getFloat(barray::const_iterator & position);
float getFloat_r(barray::const_iterator & position);

std::string getString(const barray & data, barray::const_iterator & position);
std::string getStringWr(const barray & data, barray::const_iterator & position);
std::wstring getWString(const barray & data, barray::const_iterator & position);

// *** Put functions ***

void putQWORD(barray::iterator & position, uint64_t val);
void putQWORD_r(barray::iterator & position, uint64_t val);
void putDWORD(barray::iterator & position, DWORD val);
void putDWORD_r(barray::iterator & position, DWORD val);
void putWORD(barray::iterator & position, WORD val);
void putWORD_r(barray::iterator & position, WORD val);
void putFloat(barray::iterator & position, float val);
void putFloat_r(barray::iterator & position, float val);

void putString(barray & data, barray::iterator & position, const std::string & val);
void putStringWr(barray & data, barray::iterator & position, const std::string & val);
void putWString(barray & data, barray::iterator & position, const std::wstring & val);

// *** Utils 

// Return true if conversion succeeds
bool tstrToBArray(tstring & str, barray & array);

DWORD bitCounter(DWORD N);

void stringTrim(std::wstring & line);

#endif
 
