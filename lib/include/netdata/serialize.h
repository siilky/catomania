#ifndef serialize_h
#define serialize_h

#include "compactuint.h"
#include "common.h"
#include "utils.h"

#pragma warning(disable: 6209)

template<int mode>
class Serializer
{
};

template<>
class Serializer<0>    // Calculate length on formed data set
{
public:
    Serializer()
        : size_(0)
    {}

    Serializer & b()                    { size_ ++; return *this; }
    Serializer & w()                    { size_ += 2; return *this; }
    Serializer & l()                    { size_ += 4; return *this; }
    Serializer & q()                    { size_ += 8; return *this; }
    Serializer & skip(size_t bytes)     { size_ += bytes; return *this; }

    Serializer & cb(byte /*b*/)         { size_ ++; return *this; }

    Serializer & b(byte & /*b*/)        { size_ ++; return *this; }
    Serializer & w(WORD & /*w*/)        { size_ += 2; return *this; }
    Serializer & l(DWORD & /*d*/)       { size_ += 4; return *this; }
    Serializer & q(uint64_t & )         { size_ += 8; return *this; }
    Serializer & f(float & /*f*/)       { size_ += 4; return *this; }

    Serializer & wr(WORD & /*w*/)       { size_ += 2; return *this; }
    Serializer & lr(DWORD & /*d*/)      { size_ += 4; return *this; }
    Serializer & qr(uint64_t & )        { size_ += 8; return *this; }
    Serializer & fr(float & /*f*/)      { size_ += 4; return *this; }

    Serializer & s(std::wstring & s)    { size_ += 2 * s.size() + getPackedCUISize(s.size()); return *this; }
    Serializer & c(std::string & s)     { size_ += s.size() + getPackedCUISize(s.size()); return *this; }
    Serializer & cwr(std::string & s)   { size_ += s.size() + 2; return *this; }
    Serializer & cv(std::string & s)    { size_ += s.size(); return *this; }

    Serializer & cui(unsigned & v)      { size_ += getPackedCUISize(v); return *this; }

    // user-defined type
    template<class T>
    Serializer & t(T & t)               { t.format(*this); return *this; }

    // Array of fixed-size basic elements.
    template<typename Elem, typename lengthType, typename outType>
    Serializer & arr( std::vector<Elem> & v
                    , Serializer & (Serializer::*lengthSetter)(lengthType &)
                    , Serializer & (Serializer::*setter)(outType &))
    {
        lengthType length = (lengthType)v.size();
        (this->*lengthSetter)(length);

        return arr(v, length, setter);
    }

    // Array of basic elements with defined count.
    template<typename Elem, typename outType>
    Serializer & arr( std::vector<Elem> & v
                    , size_t count
                    , Serializer & (Serializer::*reader)(outType &))
    {
        for (size_t i = 0; i < count; i++)
        {
            (this->*reader)((outType &)v[i]);
        }

        return *this;
    }

    // Array of POD elements.
    template<typename Elem, typename lengthType>
    Serializer & arr( std::vector<Elem> & v
                    , Serializer & (Serializer::*lengthSetter)(lengthType &))
    {
        lengthType length = (lengthType)v.size();
        (this->*lengthSetter)(length);

        return arr(v, length);
    }

    // Array of POD elements with defined count.
    template<typename Elem>
    Serializer & arr( std::vector<Elem> & v
                    , size_t count)
    {
        for (size_t i = 0; i < count; i++)
        {
            v[i].format(*this);
        }

        return *this;
    }

    // static array
    template<typename In, typename Out>
    Serializer & m(In * /*v*/, size_t items, Serializer & (Serializer::*)(Out &))
    {
        size_ += items * sizeof(Out); 
        return *this;
    }

    unsigned size() const
    {
        return size_;
    }
    unsigned position() const
    {
        return size_;
    }

private:
    unsigned    size_;
};

template<>
class Serializer<1>      // Parse binary data to typed structure
{
public:
    Serializer(const barray & stream)
        : stream_(stream), curPos_(stream.begin()), badbit_(false)
    {}

    Serializer & b()                    { ++curPos_; return *this; }
    Serializer & w()                    { curPos_ += 2; return *this; }
    Serializer & l()                    { curPos_ += 4; return *this; }
    Serializer & q()                    { curPos_ += 8; return *this; }
    Serializer & skip(size_t bytes)     { curPos_ += bytes; return *this; }

    Serializer & cb(byte /*b*/)         { /*b = * */curPos_++; return *this; }

    Serializer & b(byte & b)            { b = *curPos_++; return *this; }
    Serializer & w(WORD & w)            { w = getWORD(curPos_); return *this; }
    Serializer & l(DWORD & d)           { d = getDWORD(curPos_); return *this; }
    Serializer & q(uint64_t & q)        { q = getQWORD(curPos_); return *this; }
    Serializer & f(float & f)           { f = getFloat(curPos_); return *this; }

    Serializer & wr(WORD & w)           { w = getWORD_r(curPos_); return *this; }
    Serializer & lr(DWORD & d)          { d = getDWORD_r(curPos_); return *this; }
    Serializer & qr(uint64_t & q)       { q = getQWORD_r(curPos_); return *this; }
    Serializer & fr(float & f)          { f = getFloat_r(curPos_); return *this; }

    Serializer & s(std::wstring & s)    { s = getWString(stream_, curPos_); return *this; }
    Serializer & c(std::string & s)     { s = getString(stream_, curPos_); return *this; }
    Serializer & cwr(std::string & s)   { s = getStringWr(stream_, curPos_); return *this; }

    Serializer & cui(unsigned & v)      { v = getCUI(stream_, curPos_); return *this; }

    // user-defined type
    template<class T>
    Serializer & t(T & t)               { t.format(*this); return *this; }

    // Array of basic elements.
    template<typename Elem, typename lengthType, typename outType>
    Serializer & arr( std::vector<Elem> & v
                    , Serializer & (Serializer::*lengthReader)(lengthType &)
                    , Serializer & (Serializer::*reader)(outType &))
    {
        lengthType length;
        (this->*lengthReader)(length);

        return arr(v, length, reader);
    }

    // Array of basic elements with defined count.
    template<typename Elem, typename outType>
    Serializer & arr( std::vector<Elem> & v
                    , size_t count
                    , Serializer & (Serializer::*reader)(outType &))
    {
        v.resize(count);
        for (size_t i = 0; i < count; i++)
        {
            (this->*reader)((outType &)v[i]);
        }

        return *this;
    }

    // Array of POD elements.
    template<typename Elem, typename lengthType>
    Serializer & arr( std::vector<Elem> & v
                    , Serializer & (Serializer::*lengthReader)(lengthType &))
    {
        lengthType length;
        (this->*lengthReader)(length);

        return arr(v, length);
    }

    // Array of POD elements with defined count.
    template<typename Elem>
    Serializer & arr( std::vector<Elem> & v
                    , size_t count)
    {
        v.resize(count);

        for (size_t i = 0; i < count; i++)
        {
            v[i].format(*this);
        }

        return *this;
    }

    // static array
    template<typename In, typename Out>
    Serializer & m(In *v, size_t items, Serializer & (Serializer::*reader)(Out &))
    {
        for (size_t i = 0; i < items; i++)
        {
            (this->*reader)((Out &)v[i]);
        }

        return *this;
    }

    bool isOk() const
    {
        return badbit_;
    }
    void fail()
    {
        badbit_ = true;
    }

    unsigned size() const
    {
        return stream_.size();
    }
    unsigned position() const
    {
        return curPos_ - stream_.begin();
    }

private:
    friend class CharAttrs;

    const barray          & stream_;
    barray::const_iterator  curPos_;

    bool                badbit_;

private:
    Serializer<1> & operator=(const Serializer<1> &);
};

template<>
class Serializer<2>    // Merge typed data to binary array
{
public:
    Serializer(barray & stream)
        : stream_(stream), curPos_(stream.begin()), badbit_(false)
    {}

    Serializer & b()                    { *curPos_++ = 0; return *this; }
    Serializer & w()                    { putWORD(curPos_, 0); return *this; }
    Serializer & l()                    { putDWORD(curPos_, 0); return *this; }
    Serializer & skip(size_t bytes)     { for (size_t i = 0; i < bytes; i++) { *curPos_++ = 0; } return *this; }

    Serializer & cb(byte b)             { *curPos_++ = b; return *this; }

    Serializer & b(byte & b)            { *curPos_++ = b; return *this; }
    Serializer & w(WORD & w)            { putWORD(curPos_, w); return *this; }
    Serializer & l(DWORD & d)           { putDWORD(curPos_, d); return *this; }
    Serializer & q(uint64_t & q)        { putQWORD(curPos_, q); return *this; }
    Serializer & f(float & f)           { putFloat(curPos_, f); return *this; }

    Serializer & wr(WORD & w)           { putWORD_r(curPos_, w); return *this; }
    Serializer & lr(DWORD & d)          { putDWORD_r(curPos_, d); return *this; }
    Serializer & qr(uint64_t & q)       { putQWORD_r(curPos_, q); return *this; }
    Serializer & fr(float & f)          { putFloat_r(curPos_, f); return *this; }

    Serializer & s(std::wstring & s)    { putWString(stream_, curPos_, s); return *this; }
    Serializer & c(std::string & s)     { putString(stream_, curPos_, s); return *this; }
    Serializer & cwr(std::string & s)   { putStringWr(stream_, curPos_, s); return *this; }

    Serializer & cui(unsigned & v)      { putCUI(curPos_, v); return *this; }

    // user-defined type
    template<class T>
    Serializer & t(T & t)               { t.format(*this); return *this; }

    // Array of basic elements.
    template<typename Elem, typename lengthType, typename outType>
    Serializer & arr( std::vector<Elem> & v
                    , Serializer & (Serializer::*lengthWriter)(lengthType &)
                    , Serializer & (Serializer::*writer)(outType &))
    {
        lengthType length = (lengthType)v.size();
        (this->*lengthWriter)(length);

        return arr(v, length, writer);
    }

    // Array of basic elements with defined count.
    template<typename Elem, typename outType>
    Serializer & arr( std::vector<Elem> & v
                    , size_t count
                    , Serializer & (Serializer::*writer)(outType &))
    {
        for (size_t i = 0; i < count; i++)
        {
            (this->*writer)((outType &)v[i]);
        }

        return *this;
    }

    // Array of POD elements.
    template<typename Elem, typename lengthType>
    Serializer & arr( std::vector<Elem> & v
                    , Serializer & (Serializer::*lengthWriter)(lengthType &))
    {
        lengthType length = (lengthType)v.size();
        (this->*lengthWriter)(length);

        return arr(v, length);
    }

    // Array of POD elements with defined count.
    template<typename Elem>
    Serializer & arr( std::vector<Elem> & v
                    , size_t count)
    {
        for (size_t i = 0; i < count; i++)
        {
            v[i].format(*this);
        }

        return *this;
    }

    // static array
    template<typename In, typename Out>
    Serializer & m(In *v, size_t items, Serializer & (Serializer::*writer)(Out &))
    {
        for (size_t i = 0; i < items; i++)
        {
            (this->*writer)((Out &)v[i]);
        }

        return *this;
    }

    void fail()
    {
        badbit_ = true;
    }

    unsigned size() const
    {
        return stream_.size();
    }
    unsigned position() const
    {
        return curPos_ - stream_.begin();
    }

private:
    barray            & stream_;
    barray::iterator    curPos_;

    bool                badbit_;

private:
    Serializer<2> & operator=(const Serializer<2> &);
};

typedef Serializer<0>   SerializerLength;
typedef Serializer<1>   SerializerIn;
typedef Serializer<2>   SerializerOut;


#endif
