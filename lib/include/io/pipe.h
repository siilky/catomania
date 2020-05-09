#ifndef pipe_h
#define pipe_h

#include "thread.h"
#include "io/asyncIoBuffer.h"


class Pipe : public AsyncIoBuffer
{
public:
    typedef enum
    {
        PIPE_MODE_SERVER,
        PIPE_MODE_CLIENT,
    } Mode;

    Pipe(Mode mode)
        : AsyncIoBuffer(mode == PIPE_MODE_SERVER ? 
                        AsyncIoBuffer::IOBUFFER_MODE_SERVER
                        : AsyncIoBuffer::IOBUFFER_MODE_CLIENT)
    {}
    ~Pipe()
    {
        // stop thread before virtual functions will disappear
        stop();
        closeIo();
    }

    // Create pipe instance and wait for connections
    // return true if ok, false if expectable error occurs (pipe already exists, etc)
    // User should call listen only once the pipe was created, next listen calls
    // will always fail
    bool listen(const tstring &pipeName, int maxInstances = 1);

    // Connects client pipe instance to the existing server.
    // return true if ok, false if expectable error occurs.
    // Fails if pipe was created in server mode.
    // Sequential connects are allowed (after a server was disconnected)
    bool connect(const tstring &pipeName);

private:

    virtual bool acceptConnection(HANDLE hFile, bool *fatalError, bool *terminating);
    virtual bool closeServer(HANDLE hFile);
    virtual bool closeClient(HANDLE hFile);
};

#endif // pipe_h