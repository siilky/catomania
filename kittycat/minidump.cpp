
#include "stdafx.h"

#include "minidump.h"


#define CREATE_MINIDUMP


#if defined(CREATE_MINIDUMP)

#pragma warning(disable : 4091)    //: 'typedef ': ignored on left of '' when no variable is declared 
#include <Dbghelp.h>
#pragma comment(lib, "dbghelp.lib")

long __stdcall crashHandler(EXCEPTION_POINTERS* pep)
{
    QDateTime now = QDateTime::currentDateTime();
    QString filename = QString("cats2_%1%2%3_%4-%5-%6.dmp")
        .arg(now.date().year(), 4, 10, QChar('0')).arg(now.date().month(), 2, 10, QChar('0')).arg(now.date().day(), 2, 10, QChar('0'))
        .arg(now.time().hour(), 2, 10, QChar('0')).arg(now.time().minute(), 2, 10, QChar('0')).arg(now.time().second(), 2, 10, QChar('0'));

    HANDLE hFile = CreateFileW((wchar_t*)filename.utf16()
                               , GENERIC_READ | GENERIC_WRITE
                               , 0
                               , NULL
                               , CREATE_ALWAYS
                               , FILE_ATTRIBUTE_NORMAL
                               , NULL);
    if (hFile == NULL
        || hFile == INVALID_HANDLE_VALUE)
    {
        QString text = QString("An unexpected error has occurred.\n\n"
                               "Failed to create dump with status %1")
            .arg(GetLastError());
        ::MessageBoxW(NULL, (wchar_t*)text.utf16(), L"Cats2", MB_ICONSTOP | MB_OK);
        return EXCEPTION_CONTINUE_SEARCH;
    }

    MINIDUMP_EXCEPTION_INFORMATION mdei;
    mdei.ThreadId = GetCurrentThreadId();
    mdei.ExceptionPointers = pep;
    mdei.ClientPointers = FALSE;

    MINIDUMP_TYPE mdt = (MINIDUMP_TYPE)(
        MiniDumpWithFullMemory
        | MiniDumpWithFullMemoryInfo
        | MiniDumpWithCodeSegs
        | MiniDumpWithDataSegs
        | MiniDumpWithHandleData
        | MiniDumpWithThreadInfo
        | MiniDumpWithUnloadedModules
        );

    BOOL rv = MiniDumpWriteDump(GetCurrentProcess()
                                , GetCurrentProcessId()
                                , hFile
                                , mdt
                                , &mdei
                                , 0
                                , 0);
    CloseHandle(hFile);

    if (rv == TRUE)
    {
        QString text = QString("An unexpected error has occurred.\n\n"
                               "Please report with dump file attached:\n"
                               "%1")
            .arg(filename);
        ::MessageBoxW(NULL, (wchar_t*)text.utf16(), L"Cats2", MB_ICONSTOP | MB_OK);
        return EXCEPTION_EXECUTE_HANDLER;
    }
    else
    {
        QString text = QString("An unexpected error has occurred.\n\n"
                               "Dump failed with status %1")
            .arg(GetLastError());
        ::MessageBoxW(NULL, (wchar_t*)text.utf16(), L"Cats2", MB_ICONSTOP | MB_OK);
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

#else

long __stdcall crashHandler(EXCEPTION_POINTERS* pep)
{
    QString text = QString("An unexpected error has occurred and program was terminated.\n\n"
                           "%1 @ %2 (%3)\n"
                           "%4 : ")
        .arg((ulong)pep->ExceptionRecord->ExceptionCode, 8, 16, QChar('0'))
        .arg((ulong)pep->ExceptionRecord->ExceptionAddress, 8, 16, QChar('0'))
        .arg((ulong)GetModuleHandle(NULL), 8, 16, QChar('0'))
        .arg((ulong)pep->ExceptionRecord->NumberParameters)
        ;
    for (unsigned i = 0; i < pep->ExceptionRecord->NumberParameters; i++)
    {
        text += QString("%1  ").arg(pep->ExceptionRecord->ExceptionInformation[i], 8, 16, QChar('0'));
    }

    text += QString("\n ebp %1  esp %2  eip %3\n eax %4  ebx %5  ecx %6  edx %7\n edi %8  esi %9")
        .arg((ulong)pep->ContextRecord->Ebp, 8, 16, QChar('0'))
        .arg((ulong)pep->ContextRecord->Esp, 8, 16, QChar('0'))
        .arg((ulong)pep->ContextRecord->Eip, 8, 16, QChar('0'))
        .arg((ulong)pep->ContextRecord->Eax, 8, 16, QChar('0'))
        .arg((ulong)pep->ContextRecord->Ebx, 8, 16, QChar('0'))
        .arg((ulong)pep->ContextRecord->Ecx, 8, 16, QChar('0'))
        .arg((ulong)pep->ContextRecord->Edx, 8, 16, QChar('0'))
        .arg((ulong)pep->ContextRecord->Edi, 8, 16, QChar('0'))
        .arg((ulong)pep->ContextRecord->Esi, 8, 16, QChar('0'))
        ;
    ::MessageBoxW(NULL, (LPCWSTR)text.utf16(), L"Cats2", MB_ICONSTOP | MB_OK);

    return EXCEPTION_EXECUTE_HANDLER;
}

#endif


