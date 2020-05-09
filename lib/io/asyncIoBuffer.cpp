// $Id: asyncIoBuffer.cpp 1558 2017-04-08 08:48:00Z jerry $

#include "stdafx.h"

#include "common.h"
#include "thread.h"
#include "log.h"
#include "WaitQueue.h"
#include "io/asyncIoBuffer.h"


//------------------------------------------------------------------------------
AsyncIoBuffer::AsyncIoBuffer(Mode mode)
    : hFile_(INVALID_HANDLE_VALUE)
    , state_(IO_NOT_CONNECTED)
    , mode_(mode)
    , readCb_(NULL)
{
    if (mode != IOBUFFER_MODE_CLIENT && mode != IOBUFFER_MODE_SERVER)
    {
        Log("Invalid mode!");
        throw eArgumentError;
    }

    #pragma warning(suppress:28125)   // The function 'InitializeCriticalSection' must be called from within a try/except block:
    InitializeCriticalSection(&readQueue_CS_);
    #pragma warning(suppress:28125)   // The function 'InitializeCriticalSection' must be called from within a try/except block:
    InitializeCriticalSection(&writeQueue_CS_);
}

AsyncIoBuffer::~AsyncIoBuffer()
{
    // stop the thread and wait for it
    stop();
    closeIo();

    DeleteCriticalSection(&readQueue_CS_);
    DeleteCriticalSection(&writeQueue_CS_);
}

//------------------------------------------------------------------------------
void AsyncIoBuffer::operator()()
{
    bool    stopThread = false;
    OVERLAPPED  read_olpd, write_olpd;
    bool    readPending = false, writePending = false;
    DWORD   lastError = 0;
    bool    isError = false;
    barray  data_read(1400);          // some default size
    int     data_read_offset = 0;

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);

    WaitQueue connectWq;
    connectWq.add(stopEvent_);
    connectWq.add(connectEvent_);

    Event<false>  readEvent;
    Event<false>  writeEvent;

    WaitQueue processWq;
    processWq.add(stopEvent_);
    processWq.add(dataWriteEvent_);

    // this loop reiterates every time the pipe disconnects (client)
    // or the pipe error occurs (both modes)
    while(!stopThread)
    {
        Log("Waiting...");

        if (mode_ == IOBUFFER_MODE_SERVER)
        {
            bool    fatalError = false;
            if ( ! acceptConnection(&fatalError, &stopThread))
            {
                // not connected

                if (fatalError)
                {
                    Log("Fatal error while connecting server end");
                    break;
                }

                continue;
            }
        }
        else if (mode_ == IOBUFFER_MODE_CLIENT)
        {
            // wait until we're allowed to continue

            bool isCancelled = false;
            HANDLE evt = connectWq.wait(isCancelled);

            if (isCancelled)
            {
                Log("Some of the io events died while waiting");
                stopThread = true;
                break;
            }

            if (evt == connectEvent_)
            {
                // cool
            }
            else if (evt == stopEvent_)
            {
                stopThread = true;
                break;
            }
            else
            {
                Log("This should not ever happen!");
                throw eStateError;
            }
        }
        else
        {
            Log("Invalid IO mode (%d)!", mode_);
            throw eStateError;
        }

        Log("Connected.");

        memset(&read_olpd, 0, sizeof(read_olpd));
        readEvent.reset();
        read_olpd.hEvent = readEvent;

        memset(&write_olpd, 0, sizeof(write_olpd));
        writeEvent.reset();
        write_olpd.hEvent = writeEvent;

        data_read_offset = 0;
        readPending = false;
        writePending = false;
        isError = false;

        // read and write IO loop

        while ( ! isError && ! stopThread)
        {
            // see if we have anything to write to

            if ( ! writePending)
            {
                EnterCriticalSection(&writeQueue_CS_);

                dataWriteEvent_.reset();

                while ( !writeQueue_.empty() && !writePending && !isError)
                {
                    // as we write data as datagrams - there is no reason
                    // to control result of operation - we cannot write
                    // data partially and any error will force IO to
                    // reconnect and data buffers to fflush
                    barray &data = writeQueue_.front();
                    DWORD bytesWritten;
                    if (WriteFile(hFile_, &(data[0]), (DWORD)data.size(), &bytesWritten, &write_olpd) == 0)
                    {
                        lastError = GetLastError();
                        if (lastError == ERROR_IO_PENDING)
                        {
                            writePending = true;
                        }
                        else
                        {
                            Log("WriteFile failed with %u", lastError);
                            error_.setFromGLE();
                            isError = true;
                        }
                    }

                    if (writePending)
                    {
                        processWq.add(writeEvent);
                    }
                    else
                    {
                        writeQueue_.pop();
                    }
                }
                LeaveCriticalSection(&writeQueue_CS_);

                if (isError)
                {
                    break;
                }
            } // write_pending

            // see if we have anything to read from

            if ( ! readPending)
            {
                while ( !readPending && !isError)
                {
                    DWORD   bytesRead;
                    if (ReadFile( hFile_
                                , &(data_read[0]) + data_read_offset
                                , (DWORD)data_read.size() - data_read_offset
                                , &bytesRead
                                , &read_olpd) == 0)
                    {
                        // retireve error code
                        lastError = GetLastError();
                        if (lastError == ERROR_IO_PENDING
                            || lastError == ERROR_MORE_DATA)
                        {
                            readPending = true;
                        }
                        else
                        {
                            Log("ReadFile failed with %u", lastError);
                            error_.setFromGLE(lastError);
                            data_read_offset = 0;
                            isError = true;
                            break;
                        }
                    }
                    else
                    {
                        // sync read OK
                        if (bytesRead == 0)
                        {
                            error_.clear();

                            // End of file
                            Log("ReadFile reached end of file");
                            data_read_offset = 0;
                            isError = true;
                            break;
                        }
                        else
                        {
                            // put the read readData into queue
                            barray readData(data_read.begin(), data_read.begin() + bytesRead + data_read_offset);
                            processReadData(readData);
                            data_read_offset = 0;
                        }
                    }
                } // read_pending

                if (readPending)
                {
                    processWq.add(readEvent);
                }
            }

            if (isError)
            {
                break;
            }

            // If everything is ok then get for the result of read/write

            bool isCanceled;
            HANDLE  evt = processWq.wait(isCanceled);

            if (isCanceled)
            {
                Log("Some of the io events died while waiting");
                stopThread = true;
                break;
            }

            if (evt == stopEvent_)
            {
                stopThread = true;
            }
            else if (evt == readEvent)
            {
                readEvent.reset();
                processWq.remove(readEvent);

                if ( ! readPending)
                {
                    Log("Got EVT_READ but read was not scheduled");
                }
                else
                {
                    readPending = false;

                    DWORD   transferred;
                    if (GetOverlappedResult(hFile_, &read_olpd, &transferred, FALSE) == 0)
                    {
                        lastError = GetLastError();
                        if (lastError == ERROR_MORE_DATA)
                        {
                            data_read.resize(data_read.capacity() * 2);
                            Log("Increasing buffer size to %u", data_read.capacity());

                            data_read_offset += transferred;
                        }
                        else if (lastError != ERROR_IO_INCOMPLETE)
                        {
                            Log("Read was failed (async) with %u", lastError);
                            error_.setFromGLE(lastError);
                            isError = true;
                            break;
                        }
                    }
                    else
                    {
                        if (transferred == 0)
                        {
                            Log("Read was completed (async) with zero size, disconnecting");
                            error_.set(10054, 0, L"Connection closed");
                            isError = true;
                        }
                        else
                        {
                            // put the read data into queue
                            barray readData(data_read.begin(), data_read.begin() + transferred + data_read_offset);
                            processReadData(readData);
                        }

                        data_read_offset = 0;
                    }
                }
            }
            else if (evt == writeEvent)
            {
                // reset the event to not go here again if we have nothing to write
                writeEvent.reset();
                processWq.remove(writeEvent);

                // write operation completed
                if ( ! writePending)
                {
                    Log("Got EVT_WRITE but write was not scheduled");
                }
                else
                {
                    writePending = false;
                    writeQueue_.pop();

                    DWORD unused;
                    if (GetOverlappedResult(hFile_, &write_olpd, &unused, FALSE) == 0)
                    {
                        lastError = GetLastError();
                        Log("Write was failed (async) with %u", lastError);
                        error_.setFromGLE(lastError);
                        isError = true;
                        break;
                    }
                    else
                    {
                        // nothing to compare written amount of data to
                    }
                }
            }
            else if (evt == dataWriteEvent_)
            {
                // data was written into writeQueue: wakeup and go work
                // ok I'm up. Have to reset the event to prevent waitqueue stickinig into it
                dataWriteEvent_.reset();
            }
            else
            {
                Log("This should not ever happen!");
                throw eStateError;
            }
        } // while ( ! isError)

        Log("End loop");
        closeIo();
    }

    statusEvent_.set();

    Log("End of IObuffer thread");
}

//------------------------------------------------------------------------------

bool AsyncIoBuffer::pollRead()
{
    bool empty;

    EnterCriticalSection(&readQueue_CS_);
    empty = readQueue_.empty();
    LeaveCriticalSection(&readQueue_CS_);
    return !empty;
}

bool AsyncIoBuffer::read(barray & data)
{
    bool result = true;

    EnterCriticalSection(&readQueue_CS_);

    if ( ! readQueue_.empty())
    {
        data = readQueue_;
        readQueue_.clear();
    }
    else
    {
        data.clear();
        result = state_ == IO_CONNECTED;
    }

    dataReadEvent_.reset();
    LeaveCriticalSection(&readQueue_CS_);

    return result;
}

bool AsyncIoBuffer::read(size_t bytes)
{
    // Removes data from read buffer

    bool result = true;

    if (bytes == 0)
    {
        return true;
    }

    EnterCriticalSection(&readQueue_CS_);

    if ( ! readQueue_.empty())
    {
        readQueue_.erase(readQueue_.begin(), readQueue_.begin() + std::min(readQueue_.size(), bytes));
    }

    if (readQueue_.empty())
    {
        dataReadEvent_.reset();
        result = state_ == IO_CONNECTED;
    }

    LeaveCriticalSection(&readQueue_CS_);

    return result;
}


bool AsyncIoBuffer::peek(barray & data)
{
    bool result = true;

    EnterCriticalSection(&readQueue_CS_);

    if ( ! readQueue_.empty())
    {
        data = readQueue_;
    }
    else
    {
        data.clear();
        result = state_ == IO_CONNECTED;
    }

    dataReadEvent_.reset();
    LeaveCriticalSection(&readQueue_CS_);

    return result;
}

bool AsyncIoBuffer::write(const barray & data)
{
    if (state_ != IO_CONNECTED)
    {
        // don't fill up the queue if pipe is not connected
        return false;
    }

    if (data.size() > 0)
    {
        EnterCriticalSection(&writeQueue_CS_);
        writeQueue_.push(data);
        dataWriteEvent_.set();
        LeaveCriticalSection(&writeQueue_CS_);
    }
    else
    {
        Log("Will not write empty readData");
    }

    return true;
}

// Protected

void AsyncIoBuffer::openClient(HANDLE hFile)
{
    if (mode_ != IOBUFFER_MODE_CLIENT)
    {
        throw eStateError;
    }

    if (state_ == IO_CONNECTED)
    {
        closeIo();
    }

    hFile_ = hFile;
    state_ = IO_CONNECTED;
    error_.clear();

    // reset the readData available event so application will not fall to reading empty pipe
    dataReadEvent_.reset();
    dataWriteEvent_.reset();

    // allow thread to continue
    connectEvent_.set();
    run();
}

void AsyncIoBuffer::openServer(HANDLE hFile)
{
    if (mode_ != IOBUFFER_MODE_SERVER)
    {
        throw eStateError;
    }

    if (state_ == IO_CONNECTED)
    {
        closeIo();
    }

    hFile_ = hFile;
    run();
}

// Private

bool AsyncIoBuffer::closeIo()
{
    if (state_ == IO_NOT_CONNECTED)
    {
        return true;
    }

    bool result = false;

    if (mode_ == IOBUFFER_MODE_SERVER)
    {
        result = closeServer(hFile_);
        // don't do this - file is still in valid state 
        // hFile_ = INVALID_HANDLE_VALUE;
    }
    else if (mode_ == IOBUFFER_MODE_CLIENT)
    {
        result = closeClient(hFile_);
        hFile_ = INVALID_HANDLE_VALUE;
    }
    else
    {
        Log("Invalid buffer mode %d!", mode_);
        throw eStateError;
    }

    state_ = IO_NOT_CONNECTED;
    fflushBuffers();

    // set the event to wake up waiting thread to make him know that
    // we're disconnected
    statusEvent_.set();

    return result;
}

bool AsyncIoBuffer::acceptConnection(bool *fatalError, bool *terminating)
{
    // return false if IO was not connected (thread stop event was signaled or
    // fatal error occurs), true if IO was connected

    *fatalError = false;
    *terminating = false;

    bool result = acceptConnection(hFile_, fatalError, terminating);

    if (result)
    {
        fflushBuffers();
        state_ = IO_CONNECTED;
        statusEvent_.set();
    }

    return result;
}

void AsyncIoBuffer::fflushBuffers()
{
    // fflush the buffers

    EnterCriticalSection(&writeQueue_CS_);
    while ( ! writeQueue_.empty()) writeQueue_.pop();
    dataWriteEvent_.reset();
    LeaveCriticalSection(&writeQueue_CS_);

    EnterCriticalSection(&readQueue_CS_);
    dataReadEvent_.reset();
    readQueue_.clear();
    LeaveCriticalSection(&readQueue_CS_);
}

void AsyncIoBuffer::processReadData(barray & bytes)
{
    bool addToQueue = true;

    if (readCb_ != NULL)
    {
        addToQueue = readCb_(readCtx_, bytes);
    }

    if (addToQueue)
    {
        EnterCriticalSection(&readQueue_CS_);
        readQueue_.insert(readQueue_.end(), bytes.begin(), bytes.end());
        dataReadEvent_.set();
        LeaveCriticalSection(&readQueue_CS_);
    }
}