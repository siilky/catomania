#ifndef asyncIoBuffer_h
#define asyncIoBuffer_h

#include "thread.h"
#include "common.h"
#include "error.h"

#include <queue>

// Asynchronous IO buffer base class


class AsyncIoBuffer : protected Thread
{
public:
    typedef enum
    {
        IOBUFFER_MODE_SERVER,
        IOBUFFER_MODE_CLIENT,
    } Mode;

    // return true if data should be put in read queue, false otherwise
    // data may be modified
    typedef bool (*AsyncIoBufferReadCb_t)(void *ctx, barray & data);


    AsyncIoBuffer(Mode mode);
    virtual ~AsyncIoBuffer();

    // Return true if client is connected.
    bool isConnected() const
    {
        return state_ == IO_CONNECTED;
    }

    // two mechanisms are available to query read operation - poll and wait

    // Returns true if data is available for reading.
    bool pollRead();

    // handle to waitable object
    // this will signal that data is available for reading
    HANDLE waitableRead()
    {
        return dataReadEvent_;
    }

    // this will signal that pipe state changes (i.e. disconnect or error)
    HANDLE waitableStatus()
    {
        return statusEvent_;
    }

    // callback mechanism for incoming data
    // - callback is called on every read operation

    void setReadCallback(AsyncIoBufferReadCb_t readCb, void *ctx)
    {
        readCb_ = readCb;
        readCtx_ = ctx;
    }

    // Read received IO data into array of bytes.
    // Return true while there is data to read or IO is in good state
    // and false if there's no data and IO is in invalid state (disconnected or error)
    bool read(barray & data);

    // Same like read but just removes data from read buffer
    bool read(size_t bytes);

    // Same as read but does not removes data them from read buffer.
    bool peek(barray & data);

    // Write array of bytes into IO.
    // returns true if ok, false if IO is in invalid state (disconnected or error)
    bool write(const barray & data);

    const ErrorState & getError() const
    {
        return error_;
    }

protected:
    ErrorState  error_;

    bool closeIo();

    virtual void stop()
    {
        stopThread();
    }

    // Parent 'open' function will open/connect file handle and pass it to me
    void openClient(HANDLE hFile);
    void openServer(HANDLE hFile);

    // Methods to override by parent

    // acceptConnection will be called inside a loop when previous client disconnects
    virtual bool acceptConnection(HANDLE hFile, bool *fatalError, bool *terminating) = 0;
    // closeConnection will be called when server endpoint needs to be disconnected
    virtual bool closeServer(HANDLE hFile) = 0;
    // closeClient will be called when client endpoint needs to be disconnected
    virtual bool closeClient(HANDLE hFile) = 0;

    HANDLE              hFile_;

    Event<>             connectEvent_;
    Event<>             statusEvent_;           // disconnect/error status notify
    Event<false>        dataReadEvent_;
    Event<false>        dataWriteEvent_;        // Used to 'poke' r/w loop to next write iteration

private:
    typedef enum
    {
        IO_NOT_CONNECTED,
        IO_CONNECTED,
    } State;

    Mode                mode_;
    volatile State      state_;

    CRITICAL_SECTION    readQueue_CS_;
    barray              readQueue_;

    CRITICAL_SECTION    writeQueue_CS_;
    std::queue<barray>  writeQueue_;

    AsyncIoBufferReadCb_t   readCb_;
    void                   *readCtx_;

    // thread overridden function
    void    operator()();

    bool acceptConnection(bool *fatalError, bool *terminating);

    void fflushBuffers();
    void processReadData(barray & bytes);
};

#endif