#ifndef mppc_h_
#define mppc_h_

#include "types.h"

#include <deque>

class BitStreamOut
{
public:
    BitStreamOut();

    void put(unsigned value, unsigned bitCount);

    const barray & bytes() const
    {
        return data_;
    }

    void byteSync();

private:
    barray      data_;
    unsigned    bitOffset_;         // текущая позиция вставки бита
};

class BitStreamIn
{
public:
    BitStreamIn(const barray & data);

    virtual unsigned get(unsigned bitCount);

    unsigned size() const           // remaining size in bits
    {
        return (data_.size() - byteOffset_) * 8 + bitOffset_ - 8;
    }

    virtual void byteSync();

private:
    BitStreamIn & operator=(const BitStreamIn &);

    const barray &  data_;
    unsigned        byteOffset_;
    int             bitOffset_;     // текущая позиция выемки бита
};

//


class MPPCEncoder
{
public:
    MPPCEncoder();

    barray transform(const barray & data);
    barray transform(barray::const_iterator position, unsigned size);

private:
    static const unsigned HistorySize = 8192;

    byte            history_[2 * HistorySize];
    unsigned        historyEndOfft_;
    unsigned short  hash_[HistorySize];
};


class MPPCDecoder
{
public:
    MPPCDecoder();

    barray transform(const barray & data);
    barray transform(const barray & data, int oSize);

private:
    static const unsigned HistorySize = 8192;

    byte            history_[3 * HistorySize];
    unsigned        historyEndOfft_;
};


#if defined(_DEBUG)
void testMPPC();
#endif

#endif