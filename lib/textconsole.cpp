
#include "stdafx.h"

#include "common.h"
#include "log.h"
#include "textconsole.h"


TextConsole::TextConsole()
    : owner_(NULL), enabled_(true)
{
    logFile_.open("pwdata.log", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    if ( ! logFile_.is_open())
    {
        Log("Error opening log file");
    }
    else
    {
        logFile_.pubsetbuf(strbuf_, sizeof_array(strbuf_));
    }

    #pragma warning(suppress:28125)   // The function 'InitializeCriticalSection' must be called from within a try/except block:
    InitializeCriticalSection(&cs_);
    #pragma warning(suppress:28125)   // The function 'InitializeCriticalSection' must be called from within a try/except block:
    InitializeCriticalSection(&lockCs_);

    precision(1);
    fill(_T('0'));
    setf(std::ios_base::fixed);
    setf(std::ios_base::uppercase);
}

TextConsole::~TextConsole()
{
    DeleteCriticalSection(&cs_);
    DeleteCriticalSection(&lockCs_);
}

//------------------------------------------------------------------------------
void TextConsole::fflush()
{
    EnterCriticalSection(&cs_);
    if (logFile_.is_open())
    {
        tstring s = str();
        logFile_.sputn(s.c_str(), (std::streamsize)s.size());
        logFile_.pubsync();
    }
    str(_T(""));
    LeaveCriticalSection(&cs_);
}

tstring TextConsole::getString()
{
    EnterCriticalSection(&cs_);
    tstring text(str());
    fflush();
    LeaveCriticalSection(&cs_);
    return text;
}

CComBSTR TextConsole::getBString()
{
    EnterCriticalSection(&cs_);
    CComBSTR text(str().c_str());
    fflush();
    LeaveCriticalSection(&cs_);
    return text;
}

TextConsole & endl(TextConsole & console)
{
    if (console.enabled_)
    {
        EnterCriticalSection(&console.cs_);
        console << _T("\n");
        LeaveCriticalSection(&console.cs_);

        if (console.owner_ != NULL)
        {
            console.owner_->PostMessage(WM_USER_UPDATE_CONSOLE, 0, 0);
        }
    }
    return console;
}


