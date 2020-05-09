#ifndef connectiontcp_h
#define connectiontcp_h

#include "connection.h"

class ConnectionTcp : public Connection
{
public:
    // Network mode
    bool open( const std::string  & login
             , const std::string  & password
             , const std::wstring & adddress
             , bool                 forced = false);
};

#endif
