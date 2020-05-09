#ifndef connectiontcp_h
#define connectiontcp_h

#include "netdata/connection.h"

namespace qlib
{

    class ConnectionTcp : public Connection
    {
    public:
        virtual ~ConnectionTcp();

        // Network mode
        bool open( const std::string   &login
                 , const std::string   &password
                 , const std::wstring  &adddress
                 , bool                 forced
                 , const QNetworkProxy &proxy);

        void close();
    };

}

#endif
