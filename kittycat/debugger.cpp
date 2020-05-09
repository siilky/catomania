#include <stdafx.h>
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#include <psapi.h>

#include "debugger.h"


class Breakpoint
{
public:
    Breakpoint();
    Breakpoint(Debugger *debugger, quintptr address);

    bool place();
    bool remove();

    bool isRemoved() const
    {
        return isRemoved_;
    }

    quintptr address() const
    {
        return address_;
    }

//     bool operator==(const Breakpoint & r) const
//     {
//         return address_ == r.address_;
//     }

private:
    Debugger    *debugger_;
    quintptr    address_;
    char        instruction_;
    bool        isRemoved_;
};

Breakpoint::Breakpoint()
    : debugger_()
    , isRemoved_()
{
    assert(0);
}

Breakpoint::Breakpoint(Debugger *debugger, quintptr address)
    : debugger_(debugger)
    , address_(address)
    , isRemoved_()
{
    assert(debugger != 0);

    QByteArray data = debugger_->readMemory(address, 1);
    if (data.size() == 1)
    {
        instruction_ = data[0];
    }
    else
    {
        qWarning() << "Failed to add BP at" << address_;
        debugger_ = 0;   // assume BP cannot be placed
    }
}

bool Breakpoint::place()
{
    if (debugger_ == 0)
    {
        return false;
    }

    QByteArray bp;
    bp.append((unsigned char) 0xCC);
    isRemoved_ = !debugger_->writeMemory(address_, bp.data(), bp.size());
    return !isRemoved_;
}

bool Breakpoint::remove()
{
    if (debugger_ == 0)
    {
        return false;
    }

    isRemoved_ = debugger_->writeMemory(address_, &instruction_, 1);
    return isRemoved_;
}

//

struct ThreadInfo
{
    ThreadInfo() : hThread(0), isSuspended(false)
    {}
    ThreadInfo(HANDLE h) : hThread(h), isSuspended(false)
    {}

    HANDLE  hThread;
    bool    isSuspended;    // waiting for debugContinue
};

class DebuggerPrivate
{
public:
    PROCESS_INFORMATION pi;

    volatile bool   runDebugging;

    mutable QMutex  lock;

    QMap<DWORD, ThreadInfo>     threads;
    QMap<quintptr, Breakpoint>  breakpoints;
    QList<unsigned>             continueThreads_;

    //QString     errorString;
    mutable unsigned    lastError;
};


Debugger::Debugger(QObject *parent)
    : QThread(parent)
    , d_ptr(new DebuggerPrivate)
{
    Q_D(Debugger);
    ZeroMemory(&d->pi, sizeof(d->pi));
    d->runDebugging = false;

    moveToThread(this);

    qRegisterMetaType<quintptr>("quintptr");
}

Debugger::~Debugger()
{
    stopDebugging();
    wait();
}

bool Debugger::startProcess(const QString & executable, const QString & currentDir)
{
    Q_D(Debugger);

    if (isRunning())
    {
        d->lastError = ERROR_ALREADY_EXISTS;
        return false;
    }

    STARTUPINFOW si;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);

    std::wstring exec(executable.toStdWString());
    // CreateProcessW may modify args string
    if (CreateProcessW(NULL, (LPWSTR) exec.c_str(), NULL, NULL, false
        , CREATE_SUSPENDED
        , NULL
        , currentDir.toStdWString().c_str()
        , &si
        , &d->pi) == 0)
    {
        d->lastError = GetLastError();
        return false;
    }

    setMemPriority();

    {
        QMutexLocker lock(&d->lock);
        d->threads.clear();
        d->continueThreads_.clear();
    }

    d->runDebugging = true;
    start();

    return true;
}

bool Debugger::addBreakpoint(quintptr addr)
{
    Q_D(Debugger);
    QMutexLocker lock(&d->lock);
    d->lastError = 0;

    if (d->breakpoints.contains(addr))
    {
        qDebug() << "BP" << QString::number(addr, 16) << "is already set";
        return false;
    }

    qDebug() << "Adding BP" << QString::number(addr, 16);
    Breakpoint bp(this, addr);
    d->breakpoints.insert(addr, bp);
    return bp.place();
}

bool Debugger::removeBreakpoint(quintptr addr)
{
    Q_D(Debugger);
    QMutexLocker lock(&d->lock);
    d->lastError = 0;

    if (!d->breakpoints.contains(addr))
    {
        qDebug() << "Trying to remove missing BP" << QString::number(addr, 16);
        return false;
    }

    qDebug() << "Removing BP" << QString::number(addr, 16);
    Breakpoint bp = d->breakpoints.take(addr);
    return bp.remove();
}

bool Debugger::continueDebugging(unsigned threadId)
{
    Q_D(Debugger);
    QMutexLocker lock(&d->lock);

    d->lastError = 0;

    if (!d->runDebugging)
    {
        qWarning() << "continueDebugging: No debugging is running";
        return false;
    }
    if (!d->threads.contains(threadId))
    {
        qWarning() << "continueDebugging: No thread to continue" << threadId;
        return false;
    }

    ThreadInfo & t = d->threads[threadId];
    if (!t.isSuspended)
    {
        qWarning() << "continueDebugging: Thread does not seem to be suspended" << threadId;
        return false;
    }

    CONTEXT context;
    context.ContextFlags = CONTEXT_SEGMENTS | CONTEXT_INTEGER | CONTEXT_CONTROL;
    if (GetThreadContext(t.hThread, &context) == 0)
    {
        qWarning() << "GetThreadContext failed with" << (d->lastError = GetLastError());
        return false;
    }

    quintptr addr = context.Eip;
    if (d->breakpoints.contains(addr))
    {
        // restore instruction
        d->breakpoints[addr].remove();

        // set single stepping
        context.EFlags |= 0x100; // Set trap flag.

        if (SetThreadContext(t.hThread, &context) == 0)
        {
            qWarning() << "SetThreadContext failed with" << (d->lastError = GetLastError());
            return false;
        }
    }
    else
    {
        qWarning() << "Stepped from unknown breakpoint at " << QString::number(addr, 16);
    }

    d->continueThreads_.append(threadId);

    return true;
}

void Debugger::stopDebugging()
{
    Q_D(Debugger);
    d->runDebugging = false;
}

// QString Debugger::errorString() const
// {
//     Q_D(const Debugger);
//     return d->errorString;
// }

unsigned Debugger::lastError() const
{
    Q_D(const Debugger);
    QMutexLocker lock(&d->lock);
    return d->lastError;
}

unsigned Debugger::processId() const
{
    Q_D(const Debugger);
    return d->runDebugging ? d->pi.dwProcessId : 0;
}

bool Debugger::getContext(unsigned threadId, CONTEXT *ctx)
{
    Q_D(const Debugger);
    QMutexLocker lock(&d->lock);

    auto iThread = d->threads.find(threadId);
    if (iThread == d->threads.end())
    {
        qDebug() << "getContext: No thread running with" << threadId;
        return false;
    }

    ctx->ContextFlags = CONTEXT_FULL;
    bool success = GetThreadContext(iThread->hThread, ctx) != 0;
    d->lastError = success ? 0 : GetLastError();
    return success;
}

bool Debugger::setContext(unsigned threadId, CONTEXT* ctx)
{
    Q_D(const Debugger);
    QMutexLocker lock(&d->lock);

    auto iThread = d->threads.find(threadId);
    if (iThread == d->threads.end())
    {
        qDebug() << "setContext: No thread running with" << threadId;
        return false;
    }

    bool success = SetThreadContext(iThread->hThread, ctx) != 0;
    d->lastError = success ? 0 : GetLastError();
    return success;
}

QByteArray Debugger::readMemory(quintptr addr, unsigned size) const
{
    Q_D(const Debugger);
    d->lastError = 0;

    if (d->pi.dwProcessId == 0)
    {
        qWarning() << "readMemory: No process running";
        return QByteArray();
    }

    QByteArray buffer;
    buffer.resize(size);
    SIZE_T bytesRead = 0;
    if (ReadProcessMemory(d->pi.hProcess, (void*) addr, buffer.data(), size, &bytesRead) == 0)
    {
        d->lastError = GetLastError();
        if (d->lastError == ERROR_PARTIAL_COPY)
        {
            buffer.resize(bytesRead);   // return partial copy
        }
        else
        {
            qWarning() << "ReadProcessMemory failed with" << d->lastError;
            return QByteArray();
        }
    }

    return buffer;
}

bool Debugger::writeMemory(quintptr addr, const char *data, unsigned size) const
{
    Q_D(const Debugger);
    d->lastError = 0;

    if (d->pi.dwProcessId == 0)
    {
        qWarning() << "No process running";
        return false;
    }

    MEMORY_BASIC_INFORMATION mi;
    if (VirtualQueryEx(d->pi.hProcess, (void*)addr, &mi, sizeof(mi)) == 0)
    {
        d->lastError = GetLastError();
        qWarning() << "VirtualQueryEx failed with" << d->lastError;
        return false;
    }
    if (mi.Protect == PAGE_NOACCESS || mi.Protect == PAGE_GUARD)
    {
        qWarning() << "Unexpected page attribute:" << mi.Protect;
        return false;
    }

    DWORD oldProtect = 0;
    if (mi.Protect == PAGE_READONLY || mi.Protect == PAGE_EXECUTE_READ)
    {
        // bit of simplification - do not check allocation size
        if (VirtualProtectEx(d->pi.hProcess, (void*) addr, size, mi.Protect == PAGE_READONLY ? PAGE_READWRITE : PAGE_EXECUTE_READWRITE, &oldProtect) == 0)
        {
            d->lastError = GetLastError();
            qWarning() << "VirtualProtectEx failed with" << d->lastError;
            return false;
        }
    }

    if (WriteProcessMemory(d->pi.hProcess, (void*)addr, data, size, NULL) == 0)
    {
        d->lastError = GetLastError();
        qWarning() << "WriteProcessMemory failed with" << d->lastError;
        return false;
    }
    if (oldProtect != 0)
    {
        DWORD unused;
        VirtualProtectEx(d->pi.hProcess, (void*) addr, size, oldProtect, &unused);
    }

    return true;
}

void Debugger::flushMemory()
{
    Q_D(Debugger);
    QMutexLocker lock(&d->lock);

    if (d->pi.hProcess != 0)
    {
        EmptyWorkingSet(d->pi.hProcess);
    }
}

//

void Debugger::run()
{
    debuggingStarted();

    Q_D(Debugger);

    if (DebugActiveProcess(d->pi.dwProcessId) == 0)
    {
        qWarning() << "DebugActiveProcess failed with" << GetLastError();
        debuggingStopped();
        return;
    }

    d->threads.insert(d->pi.dwThreadId, ThreadInfo(d->pi.hThread));
    ResumeThread(d->pi.hThread);

    bool isFirstBp = true;

    while (d->runDebugging)
    {
        // process wakeups
        {
            QMutexLocker lock(&d->lock);
            while (!d->continueThreads_.isEmpty())
            {
                if (ContinueDebugEvent(d->pi.dwProcessId, d->continueThreads_.takeFirst(), DBG_CONTINUE) == 0)
                {
                    qWarning() << "ContinueDebugEvent failed with" << GetLastError();
                }
            }
        }

        DEBUG_EVENT event = { 0 };

        if (WaitForDebugEvent(&event, 200) == 0)
        {
            DWORD result = GetLastError();
            if (result != ERROR_SEM_TIMEOUT)
            {
                qWarning() << "WaitForDebugEvent failed with" << result;
                break;
            }

            continue;   // no event
        }

        assert(event.dwProcessId == d->pi.dwProcessId);

        bool noContinue = false;
        DWORD status = DBG_CONTINUE;

        switch (event.dwDebugEventCode)
        {
            case EXCEPTION_DEBUG_EVENT:
                switch (event.u.Exception.ExceptionRecord.ExceptionCode)
                {
                    case EXCEPTION_BREAKPOINT:
                    {
                        if (event.u.Exception.dwFirstChance == 0)
                        {
                            // second chance
                            status = DBG_EXCEPTION_NOT_HANDLED;
                            break;
                        }

                        // First chance: Display the current instruction and register values. 

                        quintptr exceptionAddress = (quintptr) event.u.Exception.ExceptionRecord.ExceptionAddress;
                        qDebug() << "Triggered breakpoint at" << QString::number(exceptionAddress, 16);

                        {
                            QMutexLocker lock(&d->lock);
                            
                            if (isFirstBp)
                            {
                                isFirstBp = false;  // issue first BP at oep
                                ThreadInfo & thread = d->threads[event.dwThreadId];
                                thread.isSuspended = true;
                                noContinue = true;

                                breakpoint(exceptionAddress, event.dwThreadId);
                            }
                            else if (d->breakpoints.contains(exceptionAddress))
                            {
                                // when we hit our bp adjust eip so it points to BP address instead of BP+1
                                ThreadInfo & thread = d->threads[event.dwThreadId];

                                CONTEXT context;
                                context.ContextFlags = CONTEXT_CONTROL;
                                if (GetThreadContext(thread.hThread, &context) != 0)
                                {
                                    context.Eip -= 1;
                                    if (SetThreadContext(thread.hThread, &context) == 0)
                                    {
                                        qWarning() << "SetThreadContext failed with" << (d->lastError = GetLastError());
                                        status = DBG_EXCEPTION_NOT_HANDLED;
                                        break;
                                    }
                                }
                                else
                                {
                                    qWarning() << "GetThreadContext failed with" << (d->lastError = GetLastError());
                                    status = DBG_EXCEPTION_NOT_HANDLED;
                                    break;
                                }

                                thread.isSuspended = true;
                                noContinue = true;

                                breakpoint(exceptionAddress, event.dwThreadId);
                            }
                            else
                            {
                                // not our breakpoint
                                qDebug() << "Skipped breakpoint at" << QString::number(exceptionAddress, 16);
                                status = DBG_EXCEPTION_NOT_HANDLED;
                                break;
                            }
                        }
                        break;
                    }

                    case EXCEPTION_SINGLE_STEP:
                    {
                        if (event.u.Exception.dwFirstChance == 0)
                        {
                            // second chance
                            status = DBG_EXCEPTION_NOT_HANDLED;
                            break;
                        }

                        quintptr address = (quintptr) event.u.Exception.ExceptionRecord.ExceptionAddress;

                        QMutexLocker lock(&d->lock);

                        // restore all removed BPs
                        bool anyRestored = false;

                        QMutableMapIterator<quintptr, Breakpoint> it(d->breakpoints);
                        while (it.hasNext())
                        {
                            Breakpoint & bp = it.next().value();
                            if (bp.isRemoved())
                            {
                                qDebug() << "Restored breakpoint at" << QString::number(bp.address(), 16);
                                bp.place();
                                anyRestored = true;
                            }
                        }

                        if (!anyRestored)
                        {
                            qWarning() << "Could not find any breakpoint to restore at" << QString::number(address, 16);
                            status = DBG_CONTINUE;
                        }

                        break;
                    }

                    case EXCEPTION_ACCESS_VIOLATION:
                        qWarning() << "AV at" << QString::number((unsigned)event.u.Exception.ExceptionRecord.ExceptionAddress, 16);
                        #if defined(QT_DEBUG)
                            if (event.u.Exception.dwFirstChance == 0)
                            {
                                // second chance
                                // detach to handle AV externally
                                stopDebugger(false);
                                ContinueDebugEvent(event.dwProcessId, event.dwThreadId, DBG_CONTINUE);
                                return;
                            }
                        #endif
                        status = DBG_EXCEPTION_NOT_HANDLED;
                        break;

                    case DBG_CONTROL_C:
                        // First chance: Pass this on to the system. 
                        // Last chance: Display an appropriate error. 
                    default:
                        status = DBG_EXCEPTION_NOT_HANDLED;
                        break;
                }

                break;

            case CREATE_THREAD_DEBUG_EVENT:
            {
                //qDebug() << "New thread" << event.dwThreadId;

                QMutexLocker lock(&d->lock);
                assert(!d->threads.contains(event.dwThreadId));
                d->threads.insert(event.dwThreadId, ThreadInfo(event.u.CreateThread.hThread));

                break;
            }

            case EXIT_THREAD_DEBUG_EVENT:
            {
                //qDebug() << "Exit thread" << event.dwThreadId;

                QMutexLocker lock(&d->lock);
                d->threads.remove(event.dwThreadId);

                break;
            }

            case CREATE_PROCESS_DEBUG_EVENT:
                CloseHandle(event.u.CreateProcessInfo.hFile);
                break;

            case EXIT_PROCESS_DEBUG_EVENT:
                qDebug() << "Exit process" << event.dwProcessId;
                d->runDebugging = false;
                break;

            case LOAD_DLL_DEBUG_EVENT:
                // if (event.u.LoadDll.lpImageName != NULL)
                // {
                //     QByteArray bytes = readMemory((quintptr)event.u.LoadDll.lpImageName, QT_POINTER_SIZE);
                //     if (bytes.size() == QT_POINTER_SIZE)
                //     {
                //         quintptr ptr = * (quintptr*) bytes.data();
                //         if (ptr != 0)
                //         {
                //             QByteArray str = readMemory(ptr, 512);
                //             if (str.size() == 512)
                //             {
                //                 str[511] = 0;
                //                 qDebug() << "Loaded" << (event.u.LoadDll.fUnicode != 0 ?
                //                                          QString::fromWCharArray((const wchar_t*) str.data())
                //                                          : QString::fromLatin1((const char*) str.data()));
                //             }
                //         }
                //     }
                // }
                CloseHandle(event.u.LoadDll.hFile);
                break;

            case UNLOAD_DLL_DEBUG_EVENT:
                break;

            case OUTPUT_DEBUG_STRING_EVENT:
            {
#if defined(QT_DEBUG)
                QByteArray bytes = readMemory((quintptr)event.u.DebugString.lpDebugStringData, event.u.DebugString.nDebugStringLength);
                QString string = event.u.DebugString.fUnicode != 0 ?
                    QString::fromWCharArray((const wchar_t*) bytes.data(), bytes.size())
                    : QString::fromLocal8Bit(bytes);
                qDebug() << "#" << string;
#endif
                break;
            }

            case RIP_EVENT:
                qDebug() << "Process RIP";
                d->runDebugging = false;
                break;
        }

        if (d->runDebugging && !noContinue)
        {
            if (ContinueDebugEvent(event.dwProcessId, event.dwThreadId, status) == 0)
            {
                qWarning() << "ContinueDebugEvent failed with" << GetLastError();
            }
        }
    }

    stopDebugger();
}

void Debugger::stopDebugger(bool terminateProcess)
{
    Q_D(Debugger);
    QMutexLocker lock(&d->lock);

    if (d->pi.dwProcessId != 0) // protect from multiple calls
    {
        DebugActiveProcessStop(d->pi.dwProcessId);
        if (terminateProcess)
        {
            TerminateProcess(d->pi.hProcess, 0);
        }

        CloseHandle(d->pi.hThread);
        CloseHandle(d->pi.hProcess);
        d->pi.hThread = 0;
        d->pi.hProcess = 0;

        d->pi.dwProcessId = 0;
    }

    d->threads.clear();
    d->breakpoints.clear();
    d->continueThreads_.clear();
    d->runDebugging = false;

    debuggingStopped();
}

void Debugger::setMemPriority()
{
    typedef enum _PROCESSINFOCLASS
            {
                ProcessBasicInformation, // 0, q: PROCESS_BASIC_INFORMATION, PROCESS_EXTENDED_BASIC_INFORMATION
                ProcessQuotaLimits, // qs: QUOTA_LIMITS, QUOTA_LIMITS_EX
                ProcessIoCounters, // q: IO_COUNTERS
                ProcessVmCounters, // q: VM_COUNTERS, VM_COUNTERS_EX
                ProcessTimes, // q: KERNEL_USER_TIMES
                ProcessBasePriority, // s: KPRIORITY
                ProcessRaisePriority, // s: ULONG
                ProcessDebugPort, // q: HANDLE
                ProcessExceptionPort, // s: HANDLE
                ProcessAccessToken, // s: PROCESS_ACCESS_TOKEN
                ProcessLdtInformation, // 10
                ProcessLdtSize,
                ProcessDefaultHardErrorMode, // qs: ULONG
                ProcessIoPortHandlers, // (kernel-mode only)
                ProcessPooledUsageAndLimits, // q: POOLED_USAGE_AND_LIMITS
                ProcessWorkingSetWatch, // q: PROCESS_WS_WATCH_INFORMATION[]; s: void
                ProcessUserModeIOPL,
                ProcessEnableAlignmentFaultFixup, // s: BOOLEAN
                ProcessPriorityClass, // qs: PROCESS_PRIORITY_CLASS
                ProcessWx86Information,
                ProcessHandleCount, // 20, q: ULONG, PROCESS_HANDLE_INFORMATION
                ProcessAffinityMask, // s: KAFFINITY
                ProcessPriorityBoost, // qs: ULONG
                ProcessDeviceMap, // qs: PROCESS_DEVICEMAP_INFORMATION, PROCESS_DEVICEMAP_INFORMATION_EX
                ProcessSessionInformation, // q: PROCESS_SESSION_INFORMATION
                ProcessForegroundInformation, // s: PROCESS_FOREGROUND_BACKGROUND
                ProcessWow64Information, // q: ULONG_PTR
                ProcessImageFileName, // q: UNICODE_STRING
                ProcessLUIDDeviceMapsEnabled, // q: ULONG
                ProcessBreakOnTermination, // qs: ULONG
                ProcessDebugObjectHandle, // 30, q: HANDLE
                ProcessDebugFlags, // qs: ULONG
                ProcessHandleTracing, // q: PROCESS_HANDLE_TRACING_QUERY; s: size 0 disables, otherwise enables
                ProcessIoPriority, // qs: ULONG
                ProcessExecuteFlags, // qs: ULONG
                ProcessResourceManagement,
                ProcessCookie, // q: ULONG
                ProcessImageInformation, // q: SECTION_IMAGE_INFORMATION
                ProcessCycleTime, // q: PROCESS_CYCLE_TIME_INFORMATION // since VISTA
                ProcessPagePriority, // q: ULONG
                ProcessInstrumentationCallback, // 40
                ProcessThreadStackAllocation, // s: PROCESS_STACK_ALLOCATION_INFORMATION, PROCESS_STACK_ALLOCATION_INFORMATION_EX
                ProcessWorkingSetWatchEx, // q: PROCESS_WS_WATCH_INFORMATION_EX[]
                ProcessImageFileNameWin32, // q: UNICODE_STRING
                ProcessImageFileMapping, // q: HANDLE (input)
                ProcessAffinityUpdateMode, // qs: PROCESS_AFFINITY_UPDATE_MODE
                ProcessMemoryAllocationMode, // qs: PROCESS_MEMORY_ALLOCATION_MODE
                ProcessGroupInformation, // q: USHORT[]
                ProcessTokenVirtualizationEnabled, // s: ULONG
                ProcessConsoleHostProcess, // q: ULONG_PTR
                ProcessWindowInformation, // 50, q: PROCESS_WINDOW_INFORMATION
                ProcessHandleInformation, // q: PROCESS_HANDLE_SNAPSHOT_INFORMATION // since WIN8
                ProcessMitigationPolicy, // s: PROCESS_MITIGATION_POLICY_INFORMATION
                ProcessDynamicFunctionTableInformation,
                ProcessHandleCheckingMode,
                ProcessKeepAliveCount, // q: PROCESS_KEEPALIVE_COUNT_INFORMATION
                ProcessRevokeFileHandles, // s: PROCESS_REVOKE_FILE_HANDLES_INFORMATION
                ProcessWorkingSetControl, // s: PROCESS_WORKING_SET_CONTROL
                ProcessHandleTable, // since WINBLUE
                ProcessCheckStackExtentsMode,
                ProcessCommandLineInformation, // 60, q: UNICODE_STRING
                ProcessProtectionInformation, // q: PS_PROTECTION
                MaxProcessInfoClass
            } PROCESSINFOCLASS;
    typedef NTSTATUS (__stdcall *pNtSetInformationProcess)(
        HANDLE              ProcessHandle,
        PROCESSINFOCLASS    ProcessInformationClass,
        PVOID               ProcessInformation,
        ULONG               ProcessInformationLength);

    Q_D(Debugger);

    HMODULE ntdll = LoadLibraryW(L"ntdll.dll");
    if (ntdll != NULL)
    {
        pNtSetInformationProcess NtSetInformationProcess = (pNtSetInformationProcess) GetProcAddress(ntdll, "NtSetInformationProcess");
        if (NtSetInformationProcess != NULL)
        {
            ULONG PagePriority = 3;
            NTSTATUS status = NtSetInformationProcess(d->pi.hProcess, ProcessPagePriority, &PagePriority, sizeof(PagePriority));
            assert(status >= 0);
            qDebug() << "NtSetInformationProcess:" << status;
        }
        else
        {
            qWarning() << "Failed to get NtSetInformationProcess address";
        }

        FreeLibrary(ntdll);
    }
    else
    {
        qWarning() << "Failed to load ntdll:" << GetLastError();
    }
}
