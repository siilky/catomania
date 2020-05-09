#ifndef connectionx_h
#define connectionx_h

#include "connection.h"
#include "netio\netchannelx.h"

class ConnectionX : public Connection
{
public:

    // X mode

    virtual ~ConnectionX()
    {
        close();
    }

    bool open(DWORD pid)
    {
        error_.clear();
        close();

        NetChannelX *server = new NetChannelX(CHANNEL_TYPE_SERVER);
        server_ = server;

        NetChannelX *client = new NetChannelX(CHANNEL_TYPE_CLIENT);
        client_ = client;

        if (!server->connect(pid))
        {
            error_ = server_->getError();
            close();
            return false;
        }
        if (!client->connect(pid))
        {
            error_ = client_->getError();
            close();
            return false;
        }

        return true;
    }

    void close()
    {
        delete server_;
        server_ = 0;
        delete client_;
        client_ = 0;
    }
};

#endif
