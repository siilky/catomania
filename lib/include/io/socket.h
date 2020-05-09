#ifndef socket_h
#define socket_h

#include "thread.h"
#include "io/asyncIoBuffer.h"
#include <winsock.h>

class WSA
{
public:
    WSA()
    {
        WSADATA info;
        if (WSAStartup(MAKEWORD(2,0), &info))
        {
            throw eWinapiError;
        }
    }

    ~WSA()
    {
        WSACleanup();
    }
};

class Socket : public AsyncIoBuffer
{
public:
    typedef enum
    {
        SOCKET_MODE_SERVER,
        SOCKET_MODE_CLIENT,
    } Mode;

    Socket(Mode mode);
    ~Socket()
    {
        // stop thread before virtual functions will disappear
        stop();
        closeIo();
    }

    // Create pipe instance and wait for connections
    // return true if ok, false if expectable error occurs (pipe already exists, etc)
    // User should call listen only once the pipe was created, next listen calls
    // will always fail
    bool listen(const tstring & host, int port, int maxInstances = 1);

    // Connects client pipe instance to the existing server.
    // return true if ok, false if expectable error occurs.
    // Fails if pipe was created in server mode.
    // Sequential connects are allowed (after a server was disconnected)
    bool connect(const std::wstring & host, const std::wstring & port);

//     bool disconnect();

private:
    virtual void stop()
    {
        stopThread();
    }

    virtual bool acceptConnection(HANDLE hFile, bool *fatalError, bool *terminating);
    virtual bool closeServer(HANDLE hFile);
    virtual bool closeClient(HANDLE hFile);
};

#endif // pipe_h