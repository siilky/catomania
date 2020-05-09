#pragma once

#include "netdata/fragment.h"
#include "netdata/fragments.h"
#include "netdata/serialize.h"
#include "types.h"

namespace ARC
{

class FragmentFactory : public ::FragmentFactory
{
public:
    FragmentFactory(identifyFragment_fn identifier, const fragmentCollection_t *collection, fragment_ctor_t default_ctor = NULL)
        : ::FragmentFactory(identifier, collection, default_ctor)
    {}

    // return NULL if there's not enough data to recognize fragment structure
    FragmentBase *create(const barray & dataStream, barray::const_iterator & curPos);

    void setTeaDecoder(const std::array<uint32_t, 4> & key)
    {
        teaKey_ = key;
        doDecode_ = true;
    }

private:
    bool                    doDecode_ = false;
    std::array<uint32_t, 4> teaKey_;
};

extern FragmentFactory fragmentFactory;

class Fragment : public FragmentBase
{
public:
    Fragment()
    {}
    Fragment(unsigned id)
        : FragmentBase(id)
    {}
    Fragment(const barray & data, barray::const_iterator & curPos);

    virtual bool parse()
    {
        isParsed_ = true;
        return isParsed_;
    }

    void setOptions(unsigned short options)
    {
        options_ = options;
    }

    unsigned short options() const
    {
        return options_;
    }

    void encode(const std::array<uint32_t, 4> & key);
    void decode(const std::array<uint32_t, 4> & key);

protected:
    barray assemble(unsigned int id);

    // hide default user assemble function (visible in child)
    virtual barray assemble()
    {
        return barray();
    }

    virtual void print(tostream & stream) const;

private:
    unsigned short options_ = 0;
    unsigned reserved_ = 0;

    std::array<uint32_t, 4> key_;
    bool                    doEncode_ = false;
};


template<class Impl>
class FragmentSpec : public FragmentFormatSpec<Fragment, Impl>
{
public:
    FragmentSpec()
    {}
    FragmentSpec(const barray & data, barray::const_iterator & curPos)
        : FragmentFormatSpec(data, curPos)
    {}
};


class FragmentRsaPublicKeyImpl
{
public:
    enum { ID = 0x0001 };

    std::string key;

protected:
    template<int mode> void format(Serializer<mode> & s)
    {
        s.cwr(key);
    }
};
typedef FragmentSpec<FragmentRsaPublicKeyImpl>   FragmentRsaPublicKey;

class FragmentHandshakeOkImpl
{
public:
    enum { ID = 0x0002 };

protected:
    template<int mode> void format(Serializer<mode> & /*s*/)
    {}
};
typedef FragmentSpec<FragmentHandshakeOkImpl>   FragmentHandshakeOk;

class FragmentLoginReImpl
{
public:
    enum { ID = 0x4001 };

    byte status;
    byte code;

    std::string token;
    std::string uid1;
    std::string uid2;
    std::string password;
    std::string email;
    std::string extra;
    std::string uid3;
    DWORD   v1;
    DWORD   v2l;
    DWORD   v2h;
    DWORD   v3;

protected:
    template<int mode> void format(Serializer<mode> & s)
    {
        s.b(status).b(code);
        if (status != 0)
        {
            s.cwr(token).cwr(uid1).cwr(uid2).cwr(password).cwr(email).cwr(extra)
            .lr(v1).lr(v2h).lr(v2l)
            .cwr(uid3).lr(v3);
        }
    }
};
typedef FragmentSpec<FragmentLoginReImpl>   FragmentLoginRe;

class FragmentGetTokenReImpl
{
public:
    enum { ID = 0x4002 };

    std::string token;
    std::string extra1;
    DWORD    v1l = 0;
    DWORD    v1h = 0;

protected:
    template<int mode> void format(Serializer<mode> & s)
    {
        s.cwr(token);
        if (s.size() > s.position())
        {
            s.cwr(extra1);
        }
        if (s.size() > s.position())
        {
            s.lr(v1h).lr(v1l);
        }
    }
};
typedef FragmentSpec<FragmentGetTokenReImpl>   FragmentGetTokenRe;


// ==== Client ====


class FragmentRsaPublicKeyReImpl
{
public:
    enum { ID = 0x0050 };

    barray key;

protected:
    template<int mode> void format(Serializer<mode> & s)
    {
        s.arr(key, 0x40, &Serializer<mode>::b);
    }
};
typedef FragmentSpec<FragmentRsaPublicKeyReImpl>   FragmentRsaPublicKeyRe;


class FragmentLoginImpl
{
public:
    enum { ID = 0x3001 };

    std::string login;
    std::string password;
    std::string lang;
    std::string useragent;
    std::string hwid;
    std::string pcname;
    WORD        option;

protected:
    template<int mode> void format(Serializer<mode> & s)
    {
        s.cwr(login).cwr(password).cwr(lang).cwr(useragent).wr(option).cwr(hwid).cwr(pcname);
    }
};
typedef FragmentSpec<FragmentLoginImpl>   FragmentLogin;


class FragmentGetTokenImpl
{
public:
    enum { ID = 0x3002 };

    DWORD    v1;
    std::string game;   // pwi
    DWORD    v2l;       // counter
    DWORD    v2h;

protected:
    template<int mode> void format(Serializer<mode> & s)
    {
        s.lr(v1).cwr(game).lr(v2h).lr(v2l);
    }
};
typedef FragmentSpec<FragmentGetTokenImpl>   FragmentGetToken;


class FragmentKeepaliveImpl
{
public:
    enum { ID = 0x3004 };

protected:
    template<int mode> void format(Serializer<mode> & /*s*/)
    {}
};
typedef FragmentSpec<FragmentKeepaliveImpl>   FragmentKeepalive;


class FragmentSendPinImpl
{
public:
    enum { ID = 0x3007 };

    std::string hwid;
    std::string pcName;
    std::string pin;

protected:
    template<int mode> void format(Serializer<mode> & s)
    {
        s.cwr(hwid).cwr(pcName).cwr(pin);
    }
};
typedef FragmentSpec<FragmentSendPinImpl>   FragmentSendPin;


}
