#ifndef core_common_h
#define core_common_h

#include <assert.h>

#include "log.h"
#include "types.h"

//------------------------------------------------------------------------------
// this has more disadvantages rather than benefits
// #ifdef _UNICODE
//     #define textout wcout
// #else
//    #define textout cout
// #endif

// #if defined(_CONSOLE)
//     typedef tostream            stdconsole;
// #else
//     typedef class TextConsole   stdconsole;
// #endif


//------------------------------------------------------------------------------
class autoHandle
{
public:
    autoHandle() : h_(INVALID_HANDLE_VALUE)
    {}

    autoHandle(const HANDLE handle) : h_(handle)
    {}

    ~autoHandle()
    {
        Close();
    }

    void assign(const HANDLE h)
    {
        h_ = h;
    }

    // Use copy semantic to copy other (auto)handles

    autoHandle & operator=(autoHandle &r)
    {
        Close();

        if (r.h_ == NULL || r.h_ == INVALID_HANDLE_VALUE)
        {
            h_ = r.h_;
        }
        else
        {
            HANDLE cp = GetCurrentProcess();
            if ( ! DuplicateHandle(cp, r.h_, cp, &h_, 0, FALSE, DUPLICATE_SAME_ACCESS))
            {
                throw eHandleError;
            }
        }
        return *this;
    }

    autoHandle & operator=(const HANDLE h)
    {
        if (h == NULL || h == INVALID_HANDLE_VALUE)
        {
            h_ = h;
        }
        else
        {
            HANDLE cp = GetCurrentProcess();
            if ( ! DuplicateHandle(cp, h, cp, &h_, 0, FALSE, DUPLICATE_SAME_ACCESS))
            {
                throw eHandleError;
            }
        }
        return *this;
    }

    operator const HANDLE() const
    {
        return h_;
    }

    HANDLE * operator &()
    {
        return &h_;
    }

private:
    autoHandle(const autoHandle &);

    void Close()
    {
        if (h_ == NULL || h_ == INVALID_HANDLE_VALUE)
        {
            return;
        }

        CloseHandle(h_);
        h_ = INVALID_HANDLE_VALUE;
    }

    HANDLE  h_;
};

//------------------------------------------------------------------------------
class autoLibrary
{
public:
    autoLibrary() : m_(NULL)
    {};

    autoLibrary(const HMODULE module)
    {
        m_ = module;
    }

    ~autoLibrary()
    {
        Close();
    }

    operator const HMODULE() const
    {
        return m_;
    }

    bool Load(wchar_t *libraryName)
    {
        m_ = LoadLibraryW(libraryName);
        return m_ != NULL;
    }

    void Close()
    {
        if (m_ != NULL)
        {
            FreeLibrary(m_);
            m_ = NULL;
        }
    }

private:
    autoLibrary & operator=(autoLibrary &r);
    autoLibrary & operator=(const HMODULE m);

    HMODULE     m_;
};

//------------------------------------------------------------------------------

#pragma warning(disable : 28125)
#pragma warning(disable : 26135)
class Syncronized
{
public:
    Syncronized()
    {
        InitializeCriticalSection(&cs_);
    }

    virtual ~Syncronized()
    {
        DeleteCriticalSection(&cs_);
    }

    void lock()
    {
        EnterCriticalSection(&cs_);
    }

    void unlock()
    {
        LeaveCriticalSection(&cs_);
    }

private:
    // Disallow copying
    Syncronized(const Syncronized &);
    Syncronized & operator=(const Syncronized &);

    CRITICAL_SECTION    cs_;
};

// ----------------------------------------------------------------------------
class AutoLock
{
public:
    AutoLock(Syncronized & sync)
        : sync_(sync)
    {
        sync_.lock();
    }

    ~AutoLock()
    {
        sync_.unlock();
    }

private:
    AutoLock & operator=(const AutoLock &) { assert(0); }

    Syncronized    &sync_;
};

//------------------------------------------------------------------------------
template<bool autoReset = true>
class Event
{
public:
    Event(bool initialState = false, const TCHAR *name = NULL)
    {
        event_ = CreateEvent(NULL, !autoReset, initialState, name);
        if (event_ == NULL)
        {
            throw eWinapiError;
        }
    }

    ~Event()
    {
        CloseHandle(event_);
    }

    void set()
    {
        if (SetEvent(event_) == 0)
        {
            throw eWinapiError;
        }
    }

    void reset()
    {
        if (ResetEvent(event_) == 0)
        {
            throw eWinapiError;
        }
    }

    operator HANDLE() const
    {
        return event_;
    }

private:
    HANDLE  event_;
};

//------------------------------------------------------------------------------
template<class Parent, class User>
static void CALLBACK timerCallback_int(PVOID lpParameter, BOOLEAN TimerOrWaitFired)
{
    Timer<Parent, User> *timer = (Timer<Parent, User> *)lpParameter;

    if (timer->cb_ != NULL && TimerOrWaitFired)
    {
        timer->cb_(timer->parent_, timer->user_);
    }
}

template<class Parent, class User = void*>
class Timer
{
public:
    typedef void (*timerCallback)(Parent *t, User arg);

    Timer() : timer_(NULL)
    {}
    virtual ~Timer()
    {
        disarm();
    }

    // Periodic
    void set(unsigned msec, timerCallback cb, Parent *context, User *user = NULL)
    {
        arm(msec, true, cb, context, user);
    }

    // single
    void arm(unsigned msec, timerCallback cb, Parent *context, User *user = NULL)
    {
        arm(msec, false, cb, context, user);
    }

    void disarm()
    {
        if (timer_ != NULL)
        {
            if (DeleteTimerQueueTimer(NULL, timer_, INVALID_HANDLE_VALUE) == 0)
            {
                Log("Error stopping timer! (%08X)", GetLastError());
            }

            timer_ = NULL;
        }
    }

    void disarmAsync()
    {
        if (timer_ != NULL)
        {
            if (DeleteTimerQueueTimer(NULL, timer_, NULL) == 0)
            {
                DWORD error = GetLastError();
                if (error != ERROR_IO_PENDING)
                {
                    Log("Error stopping timer (%08X)", error);
                }
            }

            timer_ = NULL;
        }
    }

private:
    void arm(unsigned msec, bool periodic, timerCallback cb, Parent *context, User user = NULL)
    {
        cb_ = cb;
        parent_ = context;
        user_ = user;

        if (timer_ != NULL)
        {
            disarmAsync();
        }

        if ( ! CreateTimerQueueTimer( &timer_
                                    , NULL
                                    , timerCallback_int<Parent, User>
                                    , this
                                    , msec
                                    , periodic ? msec : 0
                                    , WT_EXECUTEDEFAULT))
        {
            throw eWinapiError;
        }
    }

    friend void CALLBACK timerCallback_int<Parent, User>(PVOID lpParameter, BOOLEAN TimerOrWaitFired);

    HANDLE          timer_;
    timerCallback   cb_;
    Parent         *parent_;
    User            user_;
};


#endif // common_h