#include "stdafx.h"

#include <WinCon.h>
#include <io.h>
#include <fcntl.h>

#include "common.h"

#include "log.h"
#include "utils.h"

HANDLE hLogFile = INVALID_HANDLE_VALUE;
CRITICAL_SECTION    g_cs;

#ifdef _DEBUG
void debugInit(const wchar_t *filename)
{
    #pragma warning(suppress:28125)   // The function 'InitializeCriticalSection' must be called from within a try/except block:
    InitializeCriticalSection(&g_cs);
    AllocConsole();
    SetConsoleTitle(filename);

    // redirect unbuffered STDERR to the console
    long stdHandle = (long) GetStdHandle(STD_ERROR_HANDLE);
    int hConHandle = _open_osfhandle(stdHandle, _O_TEXT);
    FILE *fp = _fdopen(hConHandle, "w");
    *stderr = *fp;

    std::wstring logFileName(filename);
    logFileName += L".log";
    hLogFile = CreateFileW( logFileName.c_str()
                         , GENERIC_READ | GENERIC_WRITE
                         , FILE_SHARE_READ
                         , NULL
                         , CREATE_ALWAYS
                         , 0
                         , NULL);
    if (hLogFile == INVALID_HANDLE_VALUE)
    {
        debugPrint("Failed to create log file (%i)\n", GetLastError());
    }
}

void debugDestroy()
{
    DeleteCriticalSection(&g_cs);
    CloseHandle(hLogFile);
//    FreeConsole();
}

//------------------------------------------------------------------------------
void debugPrint(const char *format, ...)
{
    EnterCriticalSection(&g_cs);

    wchar_t ts[64];
    wsprintf(ts, L"[%08X]", (unsigned) time(0));

    std::wstring fmt = strToWStr(format);
    wchar_t msg0[1024];
    va_list ap;
    va_start(ap, format);
    vswprintf_s(msg0, sizeof(msg0)/sizeof(msg0[0]), fmt.c_str(), ap);
    va_end(ap);
    wcscat_s(msg0, L"\n");

    DWORD   dummy;
    HANDLE std_out = GetStdHandle(STD_OUTPUT_HANDLE);

    SetConsoleTextAttribute(std_out, FOREGROUND_GREEN);
    WriteConsoleW(std_out, ts, (DWORD)wcslen(ts), &dummy, NULL);
    SetConsoleTextAttribute(std_out, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_RED | FOREGROUND_INTENSITY);
    WriteConsoleW(std_out, msg0, (DWORD)wcslen(msg0), &dummy, NULL);

    if (hLogFile != INVALID_HANDLE_VALUE)
    {
        WriteFile(hLogFile, ts, (DWORD)wcslen(ts) * sizeof(*ts), &dummy, NULL);
        WriteFile(hLogFile, msg0, (DWORD)wcslen(msg0) * sizeof(*msg0), &dummy, NULL);
        FlushFileBuffers(hLogFile);
    }

    LeaveCriticalSection(&g_cs);
} 

#endif