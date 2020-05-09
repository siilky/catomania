#include "stdafx.h"

#include "fragments.h"
#include "tea.h"

namespace ARC
{

static const fragmentCollection_t fragments[] =
{
    COLLECTION_ELEMENT(RsaPublicKey),
    COLLECTION_ELEMENT(HandshakeOk),
    COLLECTION_ELEMENT(LoginRe),
    COLLECTION_ELEMENT(GetTokenRe),

    COLLECTION_END
};

bool identifyFragment(const barray & data, barray::const_iterator & curPos, unsigned int *id)
{
    auto pos = curPos;

    auto bytesLeft = data.end() - curPos;
    if (bytesLeft < 12)
    {
        return false;
    }
    bytesLeft -= 12;

    auto hdr = getWORD_r(pos);
    if (hdr != 0x5050)
    {
        return false;
    }
    
    getWORD_r(pos);
    getWORD_r(pos);
    *id = getWORD_r(pos);
    return true;
}

FragmentFactory fragmentFactory(identifyFragment, fragments, fragment_static_ctor<Fragment>);


Fragment::Fragment(const barray & data, barray::const_iterator & curPos)
{
    // do not affect pos if reading wasnt successful
    auto pos = curPos;

    auto bytesLeft = data.end() - curPos;
    if (bytesLeft < 12)
    {
        return;
    }
    bytesLeft -= 12;

    auto hdr = getWORD_r(pos);
    if (hdr != 0x5050)
    {
        return;
    }
    options_ = getWORD_r(pos);
    auto payloadLength = getWORD_r(pos);
    id_ = getWORD_r(pos);
    reserved_ = getDWORD_r(pos);

    if (bytesLeft < payloadLength)
    {
        return;
    }

    payload_.assign(pos, pos + payloadLength);
    curPos = pos + payloadLength;

    isOk_ = true;
}

void Fragment::encode(const std::array<uint32_t, 4> & key)
{
    key_ = key;
    doEncode_ = true;
}

void Fragment::decode(const std::array<uint32_t, 4> & key)
{
    if ((options_ & 0x04) != 0
        && payload_.size() >= 2)
    {
        auto it = payload_.begin();
        auto sz = getWORD(it);

        payload_.erase(payload_.begin(), payload_.begin() + 2);
        if (payload_.size() % 8 != 0)
        {
            assert(0);  // but not critical as decrypt supports it
        }
        tea_decrypt(payload_, key.data());
        payload_.resize(sz);
    }
}

barray Fragment::assemble(unsigned int id)
{
    id_ = id;

    if (doEncode_ && (options_ & 0x04) == 0)
    {
        assert(payload_.size() < std::numeric_limits<unsigned short>::max());
        unsigned short sz = (unsigned short )payload_.size();

        payload_.resize(8 * ((sz + 7) >> 3));
        tea_encrypt(payload_, key_.data());
        options_ |= 0x04;

        payload_.insert(payload_.begin(), 2, 0);
        auto it = payload_.begin();
        putWORD(it, sz);
    }

    barray result(12);
    auto pos = result.begin();
    
    putWORD_r(pos, 0x5050);
    putWORD_r(pos, options_);
    assert(payload_.size() < std::numeric_limits<unsigned short>::max());
    putWORD_r(pos, (unsigned short)payload_.size());
    assert(id < std::numeric_limits<unsigned short>::max());
    putWORD_r(pos, (unsigned short)id);
    putDWORD_r(pos, reserved_);

    result.insert(result.end(), payload_.begin(), payload_.end());

    isOk_ = true;
    isParsed_ = true;

    return result;
}

void Fragment::print(tostream & stream) const
{
    checkOk();
    checkParsed();

    stream << hex << std::uppercase << _T(" [") << id_ 
        << _T("] [") << options_
        << _T("] L=") << payload_.size() << std::endl;
    dumpBytes(stream, payload_.begin(), payload_.end());
}

FragmentBase * FragmentFactory::create(const barray & dataStream, barray::const_iterator & curPos)
{
    auto fragment = ::FragmentFactory::create(dataStream, curPos);

    if (doDecode_)
    {
        auto f = static_cast<ARC::Fragment*>(fragment);
        f->decode(teaKey_);
    }
    return fragment;
}

}
