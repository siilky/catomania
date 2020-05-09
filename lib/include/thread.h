// $Id: thread.h 1674 2018-06-14 11:43:43Z jerry $
#ifndef thread_h
#define thread_h

#include <assert.h>

#include "common.h"

// This is a wrapper for the thread.
// After it is created - call run method to run the thread.

class Thread
{
public:
    Thread()
        : hThread_(INVALID_HANDLE_VALUE)
    {}

    virtual void run();         // run the thread
    virtual void stop() = 0;    // stop the thread function should be implemented
                                // in child class and call stopThred() function

protected:
    // override this function to code that should be run as thread
    // If you expecting thread to implement stop function - use waiting for
    // stopEvent_ to actually stop the thread
    virtual void operator ()() = 0;

    operator HANDLE() const 
    {
        return hThread_;
    }

    // Destructor and Stop should be overrided by parent class:
    // parent cannot be destroyed while thread is running
    virtual ~Thread()
    {
        if (hThread_ != INVALID_HANDLE_VALUE)
        {
            _RPT0(_CRT_ASSERT, "Thread wasn't stopped in parent object");
        }
        stopThread();
    }

    void stopThread(bool wait = true);    // stop the thread

    Event<>     stopEvent_;

private:
    Thread(Thread & /*r*/);
    Thread & operator=(Thread & /*r*/);

    static unsigned __stdcall threadFn(void *arg);

    volatile HANDLE hThread_;
};

#endif // thread_h