#ifndef filereader_h_
#define filereader_h_

#include "types.h"

class FileReader
{
public:
    FileReader();
    virtual ~FileReader();

    bool open(const std::wstring & filename);

    operator bool()
    {
        return isGood_;
    }

    size_t size() const
    {
        return size_;
    }

    size_t offset() const
    {
        return offset_;
    }

//     byte operator[](size_t offset);

    void            move(size_t offset);
    byte            readByte();
    WORD            readWord();
    DWORD           readDword();
    float           readFloat();
    std::string     readString(size_t size);
    std::wstring    readWstring(size_t size);
    barray          readBytes(size_t size);

private:
    bool    isGood_;
    size_t  offset_, size_;

    void close();

    HANDLE  file_;
    HANDLE  fileMapping_;
    void   *memory_;
};



#endif