// Written by Zoltan Csizmadia, zoltan_csizmadia@yahoo.com
// For companies(Austin,TX): If you would like to get my resume, send an email.
//
// The source is free, but if you want to use it, mention my name and e-mail address
//
//////////////////////////////////////////////////////////////////////////////////////
#ifndef SYSTEMINFO_H_INCLUDED
#define SYSTEMINFO_H_INCLUDED


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning( disable : 4786 )
#pragma warning( disable : 4200 )


class INtDll
{
public:
    typedef DWORD(WINAPI *PNtQueryObject)(HANDLE, DWORD, VOID*, DWORD, VOID*);
    typedef DWORD(WINAPI *PNtQuerySystemInformation)(DWORD, VOID*, DWORD, ULONG*);
    typedef DWORD(WINAPI *PNtQueryInformationThread)(HANDLE, ULONG, PVOID, DWORD, DWORD*);
    typedef DWORD(WINAPI *PNtQueryInformationFile)(HANDLE, PVOID, PVOID, DWORD, DWORD);
    typedef DWORD(WINAPI *PNtQueryInformationProcess)(HANDLE, DWORD, PVOID, DWORD, PVOID);

    static PNtQuerySystemInformation	NtQuerySystemInformation;
    static PNtQueryObject				NtQueryObject;
    static PNtQueryInformationThread	NtQueryInformationThread;
    static PNtQueryInformationFile		NtQueryInformationFile;
    static PNtQueryInformationProcess	NtQueryInformationProcess;

    static bool							NtDllStatus;
    static DWORD						dwNTMajorVersion;

protected:
    static bool Init();
};

//////////////////////////////////////////////////////////////////////////////////////
//
// SystemProcessInformation
//
//////////////////////////////////////////////////////////////////////////////////////

class SystemHandleInformation : public INtDll
{
public:
    enum HandleType
    {
        TYPE_UNKNOWN = 0,
        TYPE_TYPE = 1,
        TYPE_DIRECTORY,
        TYPE_SYMBOLIC_LINK,
        TYPE_TOKEN,
        TYPE_PROCESS,
        TYPE_THREAD,
        TYPE_UNKNOWN_7,
        TYPE_EVENT,
        TYPE_EVENT_PAIR,
        TYPE_MUTANT,
        TYPE_UNKNOWN_11,
        TYPE_SEMAPHORE,
        TYPE_TIMER,
        TYPE_PROFILE,
        TYPE_WINDOW_STATION,
        TYPE_DESKTOP,
        TYPE_SECTION,
        TYPE_KEY,
        TYPE_PORT,
        TYPE_WAITABLE_PORT,
        TYPE_UNKNOWN_21,
        TYPE_UNKNOWN_22,
        TYPE_UNKNOWN_23,
        TYPE_UNKNOWN_24,
        //TYPE_CONTROLLER,
        //TYPE_DEVICE,
        //TYPE_DRIVER,
        TYPE_IO_COMPLETION,
        TYPE_FILE,
        _TYPES_COUNT
    } SystemHandleType;

    typedef struct _SYSTEM_HANDLE_EX
    {
        PVOID Object;
        ULONG_PTR UniqueProcessId;
        ULONG_PTR HandleValue;
        ULONG GrantedAccess;
        USHORT CreatorBackTraceIndex;
        USHORT ObjectTypeIndex;
        ULONG HandleAttributes;
        ULONG Reserved;
    } SYSTEM_HANDLE_EX, *PSYSTEM_HANDLE_EX;

    typedef struct _SYSTEM_HANDLE_INFORMATION_EX
    {
        ULONG_PTR NumberOfHandles;
        ULONG_PTR Reserved;
        SYSTEM_HANDLE_EX Handles[1];
    } SYSTEM_HANDLE_INFORMATION_EX, *PSYSTEM_HANDLE_INFORMATION_EX;

    SystemHandleInformation(DWORD pID = (DWORD) -1);
    ~SystemHandleInformation();

    bool Refresh();

    //Information functions
    bool GetTypeToken(HANDLE h, std::wstring & str, DWORD processId);
    bool GetType(HANDLE, WORD&, DWORD processId = GetCurrentProcessId());
    bool GetTypeFromTypeToken(const std::wstring & typeToken, WORD& type);

    //Thread related functions
    static bool GetThreadId(HANDLE, DWORD&, DWORD processId = GetCurrentProcessId());

    std::list<SYSTEM_HANDLE_EX> m_HandleInfos;
    DWORD	m_processId;

protected:
    bool IsSupportedHandle(SYSTEM_HANDLE_EX & handle);
    HANDLE OpenProcess(DWORD processId);
    HANDLE DuplicateHandle(HANDLE hProcess, HANDLE hRemote);
};


#endif
