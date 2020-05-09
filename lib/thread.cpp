// $Id: thread.cpp 1040 2014-02-15 16:31:04Z jerry $

#include "stdafx.h"

#include "common.h"
#include "thread.h"

//------------------------------------------------------------------------------
void Thread::run()
{
    if (hThread_ != INVALID_HANDLE_VALUE)
    {
        //assert(0);
        stopThread();
    }

    // reset event to allow thread to run
    stopEvent_.reset();

    hThread_ = (HANDLE)_beginthreadex(NULL, 0, threadFn, this, 0, NULL);
    if (hThread_ == INVALID_HANDLE_VALUE)
    {
        throw eCannotCreateThread;
    }
}

#pragma warning(disable:6258)   // TerminateThread is dangerous

void Thread::stopThread(bool wait)
{
    if (hThread_ == INVALID_HANDLE_VALUE)
    {
        return;
    }

    stopEvent_.set();

    if (wait)
    {
        DWORD res = WaitForSingleObject(hThread_, /*INFINITE*/ 10000);
        if (res == WAIT_TIMEOUT)
        {
            Log("thread won't die!");
            assert(0);
            TerminateThread(hThread_, 0);
        }
    }

    CloseHandle(hThread_);
    hThread_ = INVALID_HANDLE_VALUE;
}

unsigned __stdcall Thread::threadFn(void *arg)
{
    Thread *this_ = (Thread *)arg;
    (*this_)();

    return 0;
}
