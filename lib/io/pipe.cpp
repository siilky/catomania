// $Id: pipe.cpp 1040 2014-02-15 16:31:04Z jerry $

#include "stdafx.h"

#include "common.h"
#include "thread.h"
#include "log.h"
#include "io/pipe.h"
#include "WaitQueue.h"


//------------------------------------------------------------------------------

bool Pipe::listen(const tstring &pipeName, int maxInstances)
{
    tstring name(_T("\\\\.\\pipe\\"));
    name += pipeName;
    HANDLE hPipe = CreateNamedPipe( name.c_str()
                                  , PIPE_ACCESS_DUPLEX
                                        | FILE_FLAG_OVERLAPPED
                                        | FILE_FLAG_FIRST_PIPE_INSTANCE 
                                  , PIPE_TYPE_MESSAGE
                                        | PIPE_READMODE_MESSAGE
                                  , maxInstances
                                  , 32768
                                  , 32768
                                  , NMPWAIT_USE_DEFAULT_WAIT
                                  , NULL);
    if (hPipe != INVALID_HANDLE_VALUE)
    {
        Log("Pipe created (%08X)", hPipe);
        openServer(hPipe);
        return true;
    }
    else
    {
        DWORD lastError = GetLastError();
        Log("Pipe failed to create with %u", lastError);
        error_.setFromGLE(lastError);
        return false;
    }
}

bool Pipe::connect(const tstring &pipeName)
{
    tstring name(_T("\\\\.\\pipe\\"));
    name += pipeName;

    HANDLE hPipe = CreateFile( name.c_str()
                              , GENERIC_READ | GENERIC_WRITE
                              , 0
                              , NULL
                              , OPEN_EXISTING
                              , FILE_FLAG_OVERLAPPED
                              , NULL);
    if (hPipe != INVALID_HANDLE_VALUE)
    {
        DWORD dwMode = PIPE_READMODE_MESSAGE; 
        if ( ! SetNamedPipeHandleState( hPipe, &dwMode, NULL, NULL))
        {
            Log("SetNamedPipeHandleState failed");
            CloseHandle(hPipe);
            throw eWinapiError;
        }

        openClient(hPipe);
        return true;
    }
    else
    {
        DWORD lastError = GetLastError();
        Log("Pipe failed to create with %u", lastError);
        error_.setFromGLE(lastError);

        return false;
    }
}


//------------------------------------------------------------------------------

bool Pipe::acceptConnection(HANDLE hFile, bool *fatalError, bool *terminating)
{
    bool        connected = false;

    *fatalError = false;
    *terminating = false;

    Event<false>  connectEvent;

    OVERLAPPED  olpd;
    memset(&olpd, 0, sizeof(olpd));
    olpd.hEvent = connectEvent;

    WaitQueue connectWq;
    connectWq.add(stopEvent_);
    connectWq.add(connectEvent);

    // connect the pipe loop

    while (!connected && !*fatalError && !(*terminating))
    {
        bool pending = false;

        if (ConnectNamedPipe(hFile, &olpd) == 0)
        {
            DWORD error = GetLastError();
            switch(error)
            {
                case ERROR_PIPE_CONNECTED:
                    connected = true;
            	    break;

                case ERROR_IO_PENDING:
                    pending = true;
            	    break;

                case ERROR_NO_DATA:
                    Log("Stale end of pipe");
                    Sleep(100);
                    break;

                default:
                    Log("ConnectNamedPipe failed with %d.", error);
                    error_.setFromGLE(error);
                    *fatalError = true;
                    break;
            }
        }

        // if connection is pending - wait for its completion
        if (pending)
        {
            bool isCancelled;
            HANDLE evt = connectWq.wait(isCancelled);

            if (isCancelled)
            {
                *terminating = true;
                break;
            }

            if (evt == connectEvent)
            {
                DWORD unused;
                if (GetOverlappedResult(hFile, &olpd, &unused, FALSE) == 0)
                {
                    Log("Connect was failed (async) with %u", GetLastError());
                }
                else
                {
                    // connected, eah
                    connected = true;
                }
            }
            else if (evt == stopEvent_)
            {
                *terminating = true;
                break;
            }
            else
            {
                Log("This should not ever happen!");
                throw eStateError;
            }
        } // if pending

        if (!connected && !*fatalError && !(*terminating))
        {
            Log("retrying..");
            Sleep(1000);
        }
    }

    return connected;
}

bool Pipe::closeClient(HANDLE hFile)
{
    if (hFile != INVALID_HANDLE_VALUE && ! CloseHandle(hFile))
    {
        Log("CloseHandle failed with %d.", GetLastError());
        return false;
    }
    return true;
}

bool Pipe::closeServer(HANDLE hFile)
{
    if (hFile != INVALID_HANDLE_VALUE && ! DisconnectNamedPipe(hFile) ) 
    {
        Log("DisconnectNamedPipe failed with %d.", GetLastError());
        return false;
    }
    return true;
}