
#include "stdafx.h"

#include "netio/mppc.h"
#include "log.h"

#pragma warning(push)
#pragma warning(disable: 4244)
#pragma warning(disable: 4100)
#include "netio/gnmppc.h"
#pragma warning(pop)


#define MPPC_PWHACKS    1


BitStreamOut::BitStreamOut()
    : bitOffset_(8)
{
}

void BitStreamOut::put(unsigned value, unsigned bitCount)
{
    assert(bitCount > 0);

    if (data_.empty())
    {
        data_.push_back(0);
    }

    unsigned byteOffset = data_.size() - 1;
    while (bitCount > 0)
    {
        if (bitOffset_ <= 0)
        {
            data_.push_back(0);
            byteOffset++;
            bitOffset_ = 8;
        }

        unsigned bCnt = std::min(std::min<unsigned>(bitCount, 8), bitOffset_);
        int bOff = bitCount - bCnt;
        byte oVal = (value >> bOff) & 0xFF;
        if (bCnt < bitOffset_)
        {
            oVal <<= bitOffset_ - bCnt;
        }
        data_[byteOffset] |= oVal;

        bitCount -= bCnt;
        bitOffset_ -= bCnt;
    }
}

void BitStreamOut::byteSync()
{
    bitOffset_ = 0;
}

BitStreamIn::BitStreamIn(const barray & data)
    : data_(data)
    , byteOffset_(0)
    , bitOffset_(8)
{
}

unsigned BitStreamIn::get(unsigned bitCount)
{
    assert(bitCount > 0);

    unsigned result = 0;

    while (bitCount > 0)
    {
        if (bitOffset_ <= 0)
        {
            byteOffset_++;
            bitOffset_ = 8;
            if (byteOffset_ >= data_.size())
            {
                assert(0);
                break;
            }
        }

        unsigned bCnt = std::min(std::min<unsigned>(bitCount, 8), unsigned(bitOffset_));
        int bOff = bitOffset_ - bCnt;
        byte oVal = (data_[byteOffset_] >> bOff) & ~(0xFF << bCnt);
        if (bCnt < bitCount)
        {
            result |= ((unsigned) oVal) << (bitCount - bCnt);
        }
        else
        {
            result |= oVal;
        }

        bitCount -= bCnt;
        bitOffset_ -= bCnt;
    }

    return result;
}

void BitStreamIn::byteSync()
{
    assert(bitOffset_ != 8);
    if (bitOffset_ != 8)
    {
        byteOffset_++;
        bitOffset_ = 8;
    }
}

//

MPPCEncoder::MPPCEncoder()
{
    historyEndOfft_ = HistorySize;
    std::fill(history_, history_ + 2 * HistorySize, '\0');
    std::fill(hash_, hash_ + HistorySize, L'\0');
}

barray MPPCEncoder::transform(const barray & data)
{
    barray result;

    barray::const_iterator pos = data.begin();
    for (unsigned byteCount = data.size(); byteCount > 0;)
    {
        unsigned toDecode = std::min(byteCount, (unsigned) 8192 * rand() / RAND_MAX/*HistorySize*/);
#if defined MPPC_PWHACKS
        // каждые 8192 байт нужно сбрасывать буфер клиента, отправл€€ ресинк, а то китайци циклический буфер ниасилили.
        toDecode = std::min(toDecode, 2 * HistorySize - historyEndOfft_);
#endif
        barray part = transform(pos, toDecode);
        result.insert(result.end(), part.begin(), part.end());

        pos += toDecode;
        byteCount -= toDecode;
    }

    return result;
}


barray MPPCEncoder::transform(barray::const_iterator position, unsigned size)
{
    /* Note
    Maximum MPPC packet expansion is 12.5%. This is the worst case when
    all octets in the input buffer are >= 0x80 and we cannot find any
    repeated tokens.
    */

    assert(size <= HistorySize);

    byte * histStart = history_ + HistorySize;

    // shift old history down
#if !defined MPPC_PWHACKS
    if (historyEndOfft_ + size >= 2 * HistorySize)
    {
        std::copy(histStart, histStart + HistorySize, history_);
        historyEndOfft_ -= HistorySize;
    }
#else
    assert(historyEndOfft_ + size <= 2 * HistorySize);
    // check is at the end
#endif

    // add packet to the history
    byte *data = history_ + historyEndOfft_;
    std::copy(position, position + size, data);
    historyEndOfft_ += size;
    
    BitStreamOut output;

    while (size > 2)
    {
        // find at least 3-byte fragment

        //byte *src = data;

        unsigned idx = ((40543 * ((((data[0] << 4) ^ data[1]) << 4) ^ data[2])) >> 4) & 0x1fff;
        byte *pHist = histStart + hash_[idx];
        hash_[idx] = (unsigned short)(data - histStart);

        if (pHist > data)	        // It was before MPPC_RESTART_HISTORY
            pHist -= HistorySize;   // try previous history buffer

        unsigned offset = data - pHist;
        byte *pData = data;
        if (offset > HistorySize
             || offset < 1 
             || *pData++ != *pHist++ 
             || *pData++ != *pHist++
             || *pData++ != *pHist++)
        {
            // no match found; encode literal byte
            byte b = *data;
            if (b < 0x80)
            {
                output.put(b, 8);
            }
            else
            {
                output.put(0x100 | (b & 0x7F), 9);
            }

            --size;
            ++data;

            continue;
        }

        // Find length of the matching fragment
        //byte *dataEnd = data + size;
        while ((*pData++ == *pHist++) && (pData <= (history_ + historyEndOfft_)));
        unsigned length = pData - data - 1;

        // at least 3 character match found; code data

        // encode offset
        if (offset < 64)        { output.put(0x3C0 | offset, 10);           }       // 10-bit offset; 0 <= offset < 64
        else if (offset < 320)  { output.put(0xE00 | (offset - 64), 12);    }       // 12-bit offset; 64 <= offset < 320
        else if (offset < 8192) { output.put(0xC000 | (offset - 320), 16);  }       // 16-bit offset; 320 <= offset < 8192
        else
        {
            assert(0);
            return output.bytes();
        }

        // encode length of match
        if (length < 4)         { output.put(0, 1); }                               // length = 3
        else if (length < 8)    { output.put(0x08 | (length & 0x03), 4); }          // 4 <= length < 8
        else if (length < 16)   { output.put(0x30 | (length & 0x07), 6); }          // 8 <= length < 16
        else if (length < 32)   { output.put(0xe0 | (length & 0x0f), 8); }          // 16 <= length < 32
        else if (length < 64)   { output.put(0x3c0 | (length & 0x1f), 10); }        // 32 <= length < 64
        else if (length < 128)  { output.put(0xf80 | (length & 0x3f), 12); }        // 64 <= length < 128
        else if (length < 256)  { output.put(0x3f00 | (length & 0x7f), 14); }       // 128 <= length < 256
        else if (length < 512)  { output.put(0xfe00 | (length & 0xff), 16); }       // 256 <= length < 512
        else if (length < 1024) { output.put(0x3fc00 | (length & 0x1ff), 18); }     // 512 <= length < 1024
        else if (length < 2048) { output.put(0xff800 | (length & 0x3ff), 20); }     // 1024 <= length < 2048
        else if (length < 4096) { output.put(0x3ff000 | (length & 0x7ff), 22); }    // 2048 <= length < 4096
        else if (length < 8192) { output.put(0xffe000 | (length & 0xfff), 24); }    // 4096 <= length < 8192
        else
        {
            assert(0);
            return output.bytes();
        }

        size -= length;
        data += length;
    }
    // Add remaining octets to the output
    while (size > 0)
    {
        byte b = *data;
        if (b < 0x80)
        {
            output.put(b, 8);
        }
        else
        {
            output.put(0x100 | (b & 0x7F), 9);
        }

        --size;
        ++data;
    }

    // sync last byte
    output.put(0x3C0, 10);
    output.byteSync();

#if defined MPPC_PWHACKS
    if (historyEndOfft_ == 2 * HistorySize)
    {
        // reset history between calls
        historyEndOfft_ = HistorySize;
        std::fill(hash_, hash_ + sizeof_array(hash_), (short)0);
    }
#endif

    return output.bytes();
}

MPPCDecoder::MPPCDecoder()
    : history_{}
    , historyEndOfft_(0)
{
}

barray MPPCDecoder::transform(const barray & data)
{
    barray result;

    if (historyEndOfft_ > 2 * HistorySize)  // сбрасываем ненужную часть если она становитс€ больше окна
    {
        std::copy(history_ + HistorySize, history_ + historyEndOfft_, history_);
        historyEndOfft_ -= HistorySize;
    }

    BitStreamIn stream(data);
    while (stream.size() >= 8)
    {
        // last byte should fit 8 or 9, if its less then we're at end of stream

        unsigned value = stream.get(8);
        if (value < 0x80)       // 0 xxx xxxx
        {
            // literal byte < 0x80
            if (historyEndOfft_ < 3 * HistorySize)
            {
                history_[historyEndOfft_++] = byte(value);
                result.push_back(byte(value));
            }
            else
            {
                // buffer overflow; drop packet
                assert(0);
                return barray();
            }

            continue;
        }

        if ((value & 0xC0) == 0x80) //  10 xxx xxxx
        {
            // literal byte >= 0x80
            if (historyEndOfft_ < 3 * HistorySize)
            {
                value = (0x80 | ((value & 0x3F) << 1) | stream.get(1));
                history_[historyEndOfft_++] = byte(value);
                result.push_back(byte(value));
            }
            else
            {
                assert(0);
                return barray();
            }

            continue;
        }

        // Not a literal byte so it must be an (offset,length) pair
        //   Offset values less than 64 are encoded as bits 1111 followed by the
        //  lower 6 bits of the value.
        //   Offset values between 64 and 320 are encoded as bits 1110 followed by
        //  the lower 8 bits of the computation (value - 64).
        //   Offset values between 320 and 8191 are encoded as bits 110 followed
        //  by the lower 13 bits of the computation (value - 320).
        
        unsigned offset, len;

        if ((value & 0xF0) == 0xF0)         // 1111 xx xxxx  : 10-bit offset; 0 <= offset < 64
        {
            offset = ((value & 0x0F) << 2) | stream.get(2);
        }
        else if ((value & 0xF0) == 0xE0)    // 1110 xxxx xxxx  : 12-bit offset; 64 <= offset < 320
        {
            offset = (((value & 0x0F) << 4) | stream.get(4)) + 64;
        }
        else if ((value & 0xE0) == 0xC0)    // 110 x xxxx xxxx xxxx : 16-bit offset; 320 <= offset < 8192
        {
            offset = (((value & 0x1F) << 8) | stream.get(8)) + 320;
        }
        else
        {
            assert(0);
            return barray();
        }

        if (offset == 0)    // customization: byte sync
        {
            stream.byteSync();
            continue;
        }

        // decode length of match
        value = stream.get(1);
        if (value == 0x00)                  // 0  len = 3
        {			
            len = 3;
        }
        else if (stream.get(1) == 0x00)     // 10 xx  4 <= len < 8
        {
            len = 0x04 | stream.get(2);
        }
        else if (stream.get(1) == 0x00)     // 110 xxx  8 <= len < 16
        {
            len = 0x08 | stream.get(3);
        }
        else if (stream.get(1) == 0x00)     // 1110 xxxx  16 <= len < 32
        {
            len = 0x10 | stream.get(4);
        }
        else if (stream.get(1) == 0x00)     // 11110 x xxxx  32 <= len < 64
        {
            len = 0x20 | stream.get(5);
        }
        else if (stream.get(1) == 0x00)     // 111110 xx xxxx  64 <= len < 128
        {
            len = 0x40 | stream.get(6);
        }
        else if (stream.get(1) == 0x00)     // 1111110 xxx xxxx  128 <= len < 256
        {
            len = 0x80 | stream.get(7);
        }
        else if (stream.get(1) == 0x00)     // x1111110 xxxx xxxx  256 <= len < 512
        {
            len = 0x0100 | stream.get(8);
        }
        else if (stream.get(1) == 0x00)     // 111111110 x xxxx xxxx  512 <= len < 1024
        {
            len = 0x0200 | stream.get(9);
        }
        else if (stream.get(1) == 0x00)     // 1111111110 xx xxxx xxxx  1024 <= len < 2048
        {
            len = 0x0400 | stream.get(10);
        }
        else if (stream.get(1) == 0x00)     // 11111111110 xxx xxxx xxxx  2048 <= len < 4096
        {
            len = 0x0800 | stream.get(11);
        }
        else if (stream.get(1) == 0x00)     // 111111111110 xxxx xxxx xxxx  4096 <= len < 8192
        {
            len = 0x1000 | stream.get(12);
        }
        else
        {
            assert(0);
            return barray();
        }

        byte *dst = history_ + historyEndOfft_;
        byte *src = dst - offset;
        historyEndOfft_ += len;
        if (historyEndOfft_ < 3 * HistorySize)
        {
            // copy uncompressed bytes to the history
            // In some cases len may be greater than off. It means that memory
            // areas pointed by s and s-off overlap. To decode that strange case
            // data should be copied exactly by address increasing to make
            // some data repeated.
            //
            while (len--)
            {
                *dst = *src;
                result.push_back(*dst);
                dst++;
                src++;
            }
        }
        else
        {
            assert(0);
            return barray();
        }
    }

    return result;
}

barray MPPCDecoder::transform(const barray & data, int oSize)
{
    barray result;
    result.resize(oSize);

    if (oSize >= 8192)
    {
        if (GNET::mppc::uncompress2(result.data(), &oSize, data.data(), data.size()) >= 0)
        {
            assert(oSize == (int)result.size());
            result.resize(oSize);
            return result;
        }
        else
        {
        }
    }
    else
    {
        if (GNET::mppc::uncompress(result.data(), &oSize, data.data(), data.size()) >= 0)
        {
            assert(oSize == (int)result.size());
            result.resize(oSize);
            return result;
        }
    }

    //throw std::runtime_error("Failed to uncompress mppc");
    assert(0);
    Log("Failed to uncompress mppc");
    return {};
}


#if defined(_DEBUG)
#pragma warning(disable:6262)

void testMPPC()
{
    std::ifstream is("rfc2118.txt", std::ios_base::in | std::ios::ate);
    if (!is.good())
    {
        assert(0);
        return;
    }

    unsigned filesize = unsigned(is.tellg());
    is.seekg(0);

    barray dataIn(filesize);
    is.read((char*)&dataIn[0], filesize);

    // add some randomness
    for (int i = 0; i < 50; i++)
    {
        unsigned pos = (unsigned) (dataIn.size() - 100) * rand() / RAND_MAX;
        unsigned length = (unsigned) 50 * rand() / RAND_MAX;
        std::generate(dataIn.begin() + pos, dataIn.begin() + pos + length, []() { return (char)std::rand(); });
    }

    // encode
    MPPCEncoder encoder;
    MPPCDecoder decoder;
    for (int i = 0; i < 1000; i++)
    {
        std::vector<barray> chunks;

        barray::const_iterator pos = dataIn.begin();
        for (size_t byteCount = dataIn.size(); byteCount > 0;)
        {
            unsigned toDecode = std::min(byteCount, (unsigned) 8192 * rand() / RAND_MAX/*HistorySize*/);
            barray chunk = encoder.transform(pos, toDecode);
            chunks.push_back(chunk);

            pos += toDecode;
            byteCount -= toDecode;
        }

        // decode

        barray decoded;

        for (std::vector<barray>::const_iterator it = chunks.begin(); it != chunks.end(); ++it)
        {
            barray chunk = decoder.transform(*it);
            decoded.insert(decoded.end(), chunk.begin(), chunk.end());
        }

        auto m = std::mismatch(decoded.begin(), decoded.end(), dataIn.begin());
        unsigned off1 = m.first - decoded.begin();
        unsigned off2 = m.second - dataIn.begin();
        assert(decoded == dataIn);
        (void) off1;
        (void) off2;
    }
}
#endif
