
#include "stdafx.h"

#include "game/data/filereader.h"


bool FileReader::open(const std::wstring & filename)
{
    if (file_ != INVALID_HANDLE_VALUE)
    {
        close();
    }

    isGood_ = false;

    if (filename.empty())
    {
        return isGood_;
    }

    file_ = CreateFileW(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
    if (file_ != INVALID_HANDLE_VALUE)
    {
        DWORD high;
        DWORD low = GetFileSize(file_, &high);

        if (low != INVALID_FILE_SIZE)
        {
            size_t size = (long long(high) << 32) | long long(low);

            fileMapping_ = CreateFileMapping(file_, 0, PAGE_READONLY, high, low, 0);
            if (fileMapping_ != 0)
            {
                auto memory = MapViewOfFile(fileMapping_, FILE_MAP_READ, 0, 0, size);
                if (memory != 0)
                {
                    MemReader::open(memory, size);
                }
            }
        }
    }

    return isGood_;
}

// byte FileReader::operator[](size_t offset)
// {
//     if (offset < size_)
//     {
//         return *((byte*)(memory_) + offset);
//     }
//     else
// }

void FileReader::close()
{
    if (memory_ != 0)
    {
        UnmapViewOfFile(memory_);
    }

    if (fileMapping_ != 0)
    {
        CloseHandle(fileMapping_);
        fileMapping_ = 0;
    }

    if (file_ != INVALID_HANDLE_VALUE)
    {
        CloseHandle(file_);
        file_ = INVALID_HANDLE_VALUE;
    }

    MemReader::close();
}


void MemReader::move(size_t offset)
{
    if (offset_ + offset <= size_)
    {
        offset_ += offset;
    }
    else
    {
        isGood_ = false;
    }
}

uint8_t MemReader::readByte()
{
    uint8_t result = 0;

    if (offset_ < size_)
    {
        result = *((uint8_t *)(memory_) + offset_);
        offset_++;
    }
    else
    {
        isGood_ = false;
    }

    return result;
}

uint16_t MemReader::readWord()
{
    uint16_t result = 0;

    if ((offset_ + 2) <= size_)
    {
        result = *(uint16_t *)((byte*)(memory_) + offset_);
        offset_ += 2;
    }
    else
    {
        isGood_ = false;
    }

    return result;
}

uint32_t MemReader::readDword()
{
    uint32_t result = 0;

    if ((offset_ + 4) <= size_)
    {
        result = *(uint32_t *)((byte*)(memory_) + offset_);
        offset_ += 4;
    }
    else
    {
        isGood_ = false;
    }

    return result;
}

float MemReader::readFloat()
{
    float result = 0;

    if ((offset_ + 4) <= size_)
    {
        result = *(float*)((byte*)(memory_) + offset_);
        offset_ += 4;
    }
    else
    {
        isGood_ = false;
    }

    return result;
}

std::string MemReader::readString(size_t size)
{
    std::string result;

    if ((offset_ + size) <= size_)
    {
        result.resize(size);
        std::copy((byte*)(memory_) + offset_, (byte*)(memory_) + offset_ + size, result.begin());
        size_t trim = result.find('\0');
        if (trim != result.npos)
        {
            result.resize(trim);
        }

        offset_ += size;
    }
    else
    {
        isGood_ = false;
    }

    return result;
}

std::wstring MemReader::readWstring(size_t size)
{
    std::wstring result;

    if ((offset_ + size * 2) <= size_)
    {
        result.resize(size);

        wchar_t *begin = (wchar_t *)((byte*)(memory_) + offset_);
        wchar_t *end = (wchar_t *)((byte*)(memory_) + offset_ + size * 2);
        std::copy(begin, end, result.begin());

        size_t trim = result.find(L'\0');
        if (trim != result.npos)
        {
            result.resize(trim);
        }

        offset_ += size * 2;
    }
    else
    {
        isGood_ = false;
    }

    return result;
}

barray MemReader::readBytes(size_t size)
{
    barray result;

    if ((offset_ + size) <= size_)
    {
        result.resize(size);
        std::copy((byte*)(memory_) + offset_, (byte*)(memory_) + offset_ + size, result.begin());
        offset_ += size;
    }
    else
    {
        isGood_ = false;
    }

    return result;
}

