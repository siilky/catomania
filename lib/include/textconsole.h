#ifndef textconsole_h
#define textconsole_h

#include <atlbase.h>
#include "atlwin.h"

#include "types.h"

#define WM_USER_UPDATE_CONSOLE  (WM_USER+50)

class TextConsole : public tostringstream
{
public:
    TextConsole();
    ~TextConsole();

    void Attach(CWindow *owner) throw()
    {
        owner_ = owner;
    }

    void Detach() throw()
    {
        owner_ = NULL;
    }

    void setEnable(bool isEnabled)
    {
        enabled_ = isEnabled;
    }

    tstring     getString();
    CComBSTR    getBString();
    void        fflush();

    void lock()
    {
        EnterCriticalSection(&lockCs_);
    }

    void unlock()
    {
        LeaveCriticalSection(&lockCs_);
    }

    // make custom manipulators to work
    TextConsole & operator << (TextConsole & (*pfn)(TextConsole &))
    {
        EnterCriticalSection(&cs_);
        TextConsole & rez = pfn(*this);
        LeaveCriticalSection(&cs_);
        return rez;
    }

    TextConsole & operator << (ios_base & (*pfn)(ios_base & ))
    {
        EnterCriticalSection(&cs_);
        pfn(*this);
        LeaveCriticalSection(&cs_);
        return *this;
    }

//     TextConsole & operator << (tios & (*pfn)(tios & ))
//     {
//         pfn(*this);
//         return *this;
//     }

    friend TextConsole & endl(TextConsole & console);

    template< class T >
    friend TextConsole & operator << (TextConsole & str, T /*&*/ val);

private:
    std::basic_filebuf<TCHAR>   logFile_;
    TCHAR                       strbuf_[4096];

    volatile bool               enabled_;

    CRITICAL_SECTION            cs_, lockCs_;
    CWindow                    *owner_;
};

TextConsole & endl(TextConsole & console);

template< class T >
TextConsole & operator<<(TextConsole & str, T /*&*/ val)
{
    if (str.enabled_)
    {
        EnterCriticalSection(&str.cs_);
        static_cast<tostream&>(str) << val;
        LeaveCriticalSection(&str.cs_);
    }
    return str;
}

#endif // textconsole_h
