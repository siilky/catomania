// Written by Zoltan Csizmadia, zoltan_csizmadia@yahoo.com
// For companies(Austin,TX): If you would like to get my resume, send an email.
//
// The source is free, but if you want to use it, mention my name and e-mail address
//
//////////////////////////////////////////////////////////////////////////////////////
//
// SystemInfo.cpp v1.1
//
// History:
// 
// Date      Version     Description
// --------------------------------------------------------------------------------
// 10/16/00	 1.0	     Initial version
// 11/09/00  1.1         NT4 doesn't like if we bother the System process fix :)
//                       SystemInfoUtils::GetDeviceFileName() fix (subst drives added)
//                       NT Major version added to INtDLL class
//
//////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <process.h>
#include "SystemInfo.h"


INtDll::PNtQuerySystemInformation INtDll::NtQuerySystemInformation = NULL;
INtDll::PNtQueryObject INtDll::NtQueryObject = NULL;
INtDll::PNtQueryInformationThread	INtDll::NtQueryInformationThread = NULL;
INtDll::PNtQueryInformationFile	INtDll::NtQueryInformationFile = NULL;
INtDll::PNtQueryInformationProcess INtDll::NtQueryInformationProcess = NULL;
// DWORD INtDll::dwNTMajorVersion = SystemInfoUtils::GetNTMajorVersion();

bool INtDll::NtDllStatus = INtDll::Init();

#pragma warning(disable:6387)
bool INtDll::Init()
{
    // Get the NtDll function pointers
    NtQuerySystemInformation = (PNtQuerySystemInformation)
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQuerySystemInformation");

    NtQueryObject = (PNtQueryObject)
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQueryObject");

    NtQueryInformationThread = (PNtQueryInformationThread)
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQueryInformationThread");

    NtQueryInformationFile = (PNtQueryInformationFile)
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQueryInformationFile");

    NtQueryInformationProcess = (PNtQueryInformationProcess)
        GetProcAddress(GetModuleHandleW(L"ntdll.dll"), "NtQueryInformationProcess");

    return  NtQuerySystemInformation != NULL &&
        NtQueryObject != NULL &&
        NtQueryInformationThread != NULL &&
        NtQueryInformationFile != NULL &&
        NtQueryInformationProcess != NULL;
}

SystemHandleInformation::SystemHandleInformation(DWORD pID)
{
    m_processId = pID;
}

SystemHandleInformation::~SystemHandleInformation()
{
}

bool SystemHandleInformation::IsSupportedHandle(SYSTEM_HANDLE_EX & handle)
{
    //Here you can filter the handles you don't want in the Handle list

// 	// Windows 2000 supports everything :)
// 	if ( dwNTMajorVersion >= 5 )
// 		return true;

    //NT4 System process doesn't like if we bother his internal security :)
    if (handle.UniqueProcessId == 2 && handle.ObjectTypeIndex/*HandleType*/ == 16)
        return false;

    return true;
}

bool SystemHandleInformation::Refresh()
{
    bool  ret = true;

    m_HandleInfos.clear();

    if (!INtDll::NtDllStatus)
        return false;

    SYSTEM_HANDLE_INFORMATION_EX* pSysHandleInformation = 0;
    DWORD size = 0;
    DWORD needed = 0;
    int status;
    while ((status = NtQuerySystemInformation(64 /*SystemExtendedHandleInformation*/,
                                              pSysHandleInformation, size, &needed)) == 0xC0000004 /*STATUS_INFO_LENGTH_MISMATCH*/)
    {
        if (needed == 0)
        {
            ret = false;
            break;
        }

        if (pSysHandleInformation != 0)
        {
            delete[] pSysHandleInformation;
        }

        size = needed + 256;
        pSysHandleInformation = (SYSTEM_HANDLE_INFORMATION_EX*) new byte[size];
    }

    if (status == 0 && pSysHandleInformation != 0)
    {
        // Iterating through the objects
        for (DWORD i = 0; i < pSysHandleInformation->NumberOfHandles; i++)
        {
            SYSTEM_HANDLE_EX & handle = pSysHandleInformation->Handles[i];

            if (!IsSupportedHandle(handle))
                continue;

            // ProcessId filtering check
            if (handle.UniqueProcessId == m_processId
                || m_processId == (DWORD) -1)
            {
                //pSysHandleInformation->Handles[i].HandleType = (WORD)(pSysHandleInformation->Handles[i].HandleType % 256);
                m_HandleInfos.push_back(handle);
            }
        }
    }

    delete[] pSysHandleInformation;

    return ret;
}

HANDLE SystemHandleInformation::OpenProcess(DWORD processId)
{
    // Open the process for handle duplication
    return ::OpenProcess(PROCESS_DUP_HANDLE, true, processId);
}

HANDLE SystemHandleInformation::DuplicateHandle(HANDLE hProcess, HANDLE hRemote)
{
    HANDLE hDup = NULL;

    // Duplicate the remote handle for our process
    ::DuplicateHandle(hProcess, hRemote, GetCurrentProcess(), &hDup, 0, false, DUPLICATE_SAME_ACCESS);

    return hDup;
}

//Information functions
bool SystemHandleInformation::GetTypeToken(HANDLE h, std::wstring & str, DWORD processId)
{
    if (!NtDllStatus)
        return false;

    ULONG size = 0x2000;
    bool ret = false;

    HANDLE handle;

    HANDLE hRemoteProcess = NULL;
    bool remote = processId != GetCurrentProcessId();
    if (remote)
    {
        // Open the remote process
        hRemoteProcess = OpenProcess(processId);

        if (hRemoteProcess == NULL)
            return false;

        // Duplicate the handle
        handle = DuplicateHandle(hRemoteProcess, h);
    }
    else
    {
        handle = h;
    }

    INtDll::NtQueryObject(handle, 2, NULL, 0, &size);
    UCHAR * lpBuffer = new UCHAR[size];

    if (INtDll::NtQueryObject(handle, 2, lpBuffer, size, NULL) == 0)
    {
        str = std::wstring((LPCWSTR) (lpBuffer + 0x60));
        ret = true;
    }

    if (remote)
    {
        if (hRemoteProcess != NULL)
            CloseHandle(hRemoteProcess);

        if (handle != NULL)
            CloseHandle(handle);
    }

    if (lpBuffer != NULL)
        delete[] lpBuffer;

    return ret;
}

bool SystemHandleInformation::GetType(HANDLE h, WORD & type, DWORD processId)
{
    type = TYPE_UNKNOWN;

    std::wstring strType;
    if (!GetTypeToken(h, strType, processId))
        return false;

    return GetTypeFromTypeToken(strType, type);
}

bool SystemHandleInformation::GetTypeFromTypeToken(const std::wstring & typeToken, WORD& type)
{
    const WORD count = 27;
    static const struct
    {
        HandleType      type;
        const wchar_t * name;
    }
    constStrTypes[] = {
        TYPE_UNKNOWN,       L"",
        TYPE_TYPE,          L"",
        TYPE_DIRECTORY,     L"Directory",
        TYPE_SYMBOLIC_LINK, L"SymbolicLink",
        TYPE_TOKEN,         L"Token",
        TYPE_PROCESS,       L"Process",
        TYPE_THREAD,        L"Thread",
        TYPE_UNKNOWN_7,     L"Unknown7",
        TYPE_EVENT,         L"Event",
        TYPE_EVENT_PAIR,    L"EventPair",
        TYPE_MUTANT,        L"Mutant",
        TYPE_UNKNOWN_11,    L"Unknown11",
        TYPE_SEMAPHORE,     L"Semaphore",
        TYPE_TIMER,         L"Timer",
        TYPE_PROFILE,       L"Profile",
        TYPE_WINDOW_STATION,L"WindowStation",
        TYPE_DESKTOP,       L"Desktop",
        TYPE_SECTION,       L"Section",
        TYPE_KEY,           L"Key",
        TYPE_PORT,          L"Port",
        TYPE_WAITABLE_PORT, L"WaitablePort",
        TYPE_UNKNOWN_21,    L"Unknown21",
        TYPE_UNKNOWN_22,    L"Unknown22",
        TYPE_UNKNOWN_23,    L"Unknown23",
        TYPE_UNKNOWN_24,    L"Unknown24",
        //TYPE_CONTROLLER,
        //TYPE_DEVICE,
        //TYPE_DRIVER,
        TYPE_IO_COMPLETION, L"IoCompletion",
        TYPE_FILE,          L"File",
    };

    type = TYPE_UNKNOWN;

    for (WORD i = 1; i < _TYPES_COUNT; i++)
        if (constStrTypes[i].name == typeToken)
        {
            type = (WORD)constStrTypes[i].type;
            return true;
        }

    return false;
}
