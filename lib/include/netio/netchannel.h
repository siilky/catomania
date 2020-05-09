#ifndef netchannel_h
#define netchannel_h

#include "error.h"

// NetChannel interface

class NetChannel
{
public:
    virtual ~NetChannel(){}

    // Return true if client is connected.
    virtual bool isConnected() const = 0;

    // Return false if stream was disconnected.
    // return data with 0 size if there's no data to read
    virtual bool read(barray & data) = 0;

    // Removes data from read buffer
    virtual bool read(size_t bytes) = 0;

    // Read pipe data into array of bytes without removing them from read buffer.
    virtual bool peek(barray & data) = 0;

    // Write array of bytes into pipe.
    // returns true if ok, false if expectable error occurs (stream was disconnected)
    virtual bool write(const barray & data) = 0;

    virtual const ErrorState & getError() const = 0;
};

#endif
