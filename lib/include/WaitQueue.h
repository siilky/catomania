// $Id: WaitQueue.h 435 2010-05-19 21:15:26Z jerry $
#ifndef waitqueue_h
#define waitqueue_h

class WaitQueue
{
public:
    WaitQueue() {}
    WaitQueue(HANDLE waitable)
    {
        add(waitable);            
    }

    virtual ~WaitQueue() {}

    WaitQueue & add(HANDLE waitable);
    bool remove(HANDLE waitable);

    // Wait for all added waitable objects.
    // Returns the handle in signaled state
    // NULL handle is returned if wait time was expired
    // 
    HANDLE wait(bool & isCancelled, DWORD milliseconds = INFINITE, bool waitAll = false);

    // tests added waitable objects and returns immediately
    HANDLE test()
    {
        bool isCancelled;
        return wait(isCancelled, 0);
    }

private:
    std::vector<HANDLE>  waitables_;
};

#endif
