#ifndef serverlist_h_
#define serverlist_h_

#include <vector>


struct Server
{
    std::wstring    name;
    std::wstring    host;
    std::wstring    port;
};


bool loadServers(std::vector<Server> & servers);

#endif