#include "stdafx.h"

#include "qchanneltcp.h"
#include "connectiontcp.h"

namespace qlib
{

ConnectionTcp::~ConnectionTcp()
{
    close();
}

bool ConnectionTcp::open( const std::string  & login
                        , const std::string  & password
                        , const std::wstring & adddress
                        , bool                 forced
                        , const QNetworkProxy & proxy)
{
    error_.clear();
    close();

    QChannelTcp *server = new QChannelTcp();
    server_ = server;

    server->setProxy(proxy);
    if ( ! server->connect(login, password, adddress, forced))
    {
        error_ = server->getError();
        close();
        return false;
    }

    return true;
}

void ConnectionTcp::close()
{
    delete server_;
    server_ = 0;
}

}
