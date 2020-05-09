#ifndef clientchannel_h
#define clientchannel_h

#include "netio/netchannel.h"
#include "io/pipe.h"


typedef enum
{
    CHANNEL_TYPE_SERVER,
    CHANNEL_TYPE_CLIENT,
} NetChannelXType;

class NetChannelX : public NetChannel
                  , private Pipe
{
public:
    NetChannelX(NetChannelXType type);
    virtual ~NetChannelX()
    {};

    bool connect(DWORD pid);

    // see NetChannel header file for virtual interface documentation

    virtual bool isConnected() const
    {
        return Pipe::isConnected();
    }

    virtual bool read(barray &data)
    {
        return Pipe::read(data);
    }

    virtual bool read(size_t bytes)
    {
        return Pipe::read(bytes);
    }

    virtual bool peek(barray &data)
    {
        return Pipe::peek(data);
    }

    virtual bool write(const barray &data)
    {
        return Pipe::write(data);
    }

    virtual const ErrorState & getError() const
    {
        return Pipe::getError();
    }

private:
    NetChannelXType     type_;
};

#endif