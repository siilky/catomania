#ifndef filereader_h_
#define filereader_h_

#include "types.h"

class MemReader
{
    MemReader(const MemReader &) = delete;
    MemReader operator=(const MemReader &) = delete;
public:
    MemReader() = default;
    MemReader(MemReader && r)
        : internalData_(std::move(r.internalData_))
        , size_(r.size_)
        , offset_(r.offset_)
        , isGood_(r.isGood_)
    {
        memory_ = internalData_.empty() ? r.memory_ : internalData_.data();
    }

    virtual ~MemReader()
    {
        close();
    }

    void open(barray && data)
    {
        internalData_ = data;
        memory_ = internalData_.data();
        size_ = data.size();
        offset_ = 0;
        isGood_ = true;
    }

    void open(void * memory, size_t size)
    {
        memory_ = memory;
        size_ = size;
        offset_ = 0;
        isGood_ = true;
    }

    void close()
    {
        internalData_.clear();
        isGood_ = false;
        memory_ = 0;
        size_ = 0;
    }

    operator bool() const
    {
        return isGood_;
    }

    bool eof() const
    {
        return offset_ >= size_;
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
    uint8_t         readByte();
    uint16_t        readWord();
    uint32_t        readDword();
    float           readFloat();
    std::string     readString(size_t size);
    std::wstring    readWstring(size_t size);
    barray          readBytes(size_t size);

protected:
    bool    isGood_ = false;
    size_t  offset_ = 0, size_ = 0;

    void   *memory_ = 0;
    barray internalData_;
};


class FileReader : public MemReader
{
public:
    ~FileReader()
    {
        close();
    }

    bool open(const std::wstring & filename);
    void close();

private:
    HANDLE  file_ = INVALID_HANDLE_VALUE;
    HANDLE  fileMapping_ = INVALID_HANDLE_VALUE;
};



#endif