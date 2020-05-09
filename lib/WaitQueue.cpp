// $Id: WaitQueue.cpp 803 2012-05-10 19:57:20Z jerry $
//
#include "stdafx.h"

#include "WaitQueue.h"
#include "common.h"


WaitQueue & WaitQueue::add( HANDLE waitable )
{
    waitables_.push_back(waitable);
    return *this;
}

bool WaitQueue::remove( HANDLE waitable )
{
    std::vector<HANDLE>::iterator it = find(waitables_.begin(), waitables_.end(), waitable);
    if (it == waitables_.end())
    {
        return false;
    }

    waitables_.erase(it);
    return true;
}

HANDLE WaitQueue::wait(bool & isCancelled, DWORD milliseconds /*= INFINITE*/, bool waitAll /*= false*/)
{
    if (waitables_.size() == 0)
    {
        isCancelled = true;
        return NULL;
    }

    isCancelled = false;
    HANDLE result = NULL;

    DWORD waitRez = WaitForMultipleObjects(waitables_.size(), &waitables_[0], waitAll, milliseconds);
    if (waitRez == WAIT_FAILED)
    {
        throw eWinapiError;
    }

    if (waitRez == WAIT_TIMEOUT)
    {}
    else if (waitRez >= WAIT_OBJECT_0 && waitRez < WAIT_OBJECT_0 + waitables_.size())
    {
        result = waitables_[waitRez % MAXIMUM_WAIT_OBJECTS];
    }
    else if (waitRez >= WAIT_ABANDONED_0 && waitRez < WAIT_ABANDONED_0 + waitables_.size())
    {
        isCancelled = true;
    }
    else
    {
        assert(0);
    }

    return result;
}