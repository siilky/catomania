
#include "stdafx.h"

#include "common.h"
#include "memtools.h"


static byte * findStartPattern1(DWORD pid);
static byte * findStartPattern2(DWORD pid);

//

size_t MemoryAccess::pageSize_ = 0;

MemoryAccess::MemoryAccess(DWORD pid)
        : pages_(0)
        , page_va_(0)
        , isLocalProcess_(false)
{
    if (pid == GetCurrentProcessId())
    {
        processBase_ = (unsigned)GetModuleHandle(NULL);
        isLocalProcess_ = true;
        hProcess_ = NULL;
        return;
    }

    // TBD
    processBase_ = 0x400000;

    // get the page size
    if (pageSize_ == 0)
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        pageSize_ = si.dwPageSize;
    }

    // get the process
    hProcess_ = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE
                            , FALSE, pid);
    if (hProcess_ == NULL)
    {
        Log("Failed to open process: %i", GetLastError());
        return;
    }

    pages_ = new byte[pageSize_ * pagesCached_];
}

MemoryAccess::~MemoryAccess()
{
    if (!isLocalProcess_)
    {
        delete[] pages_;
        CloseHandle(hProcess_);
    }
}

bool MemoryAccess::isGood() const
{
    return hProcess_ != NULL;
}

byte MemoryAccess::operator[](uintptr_t va_addr)
{
    if (isLocalProcess_)
    {
        return *(byte*)va_addr;
    }

    if (hProcess_ == NULL)
    {
        return 0;
    }

    // check if current page is loaded
    if (page_va_ == 0 
        || va_addr < page_va_
        || va_addr >= page_va_ + (pageSize_ * pagesCached_))
    {
        // cache memory
        uintptr_t read_va = va_addr - (va_addr % pageSize_);

        SIZE_T bytesRead;
        BOOL result = ReadProcessMemory(hProcess_, (void *)read_va, pages_, pagesCached_ * pageSize_, &bytesRead);
        if (result == 0 || bytesRead != pagesCached_ * pageSize_)
        {
            Log("ReadProcessMemory failed (%i)", GetLastError());
            return 0;
        }

        page_va_ = read_va;
    }

    return pages_[va_addr - page_va_];
}

byte * MemoryAccess::findPattern(unsigned hintOffset, int lookupLength, const unsigned short *pattern, int patternLength)
{
    uintptr_t   curntAddr = hintOffset + processBase_;
    int         fragmentLen = 0;
    bool        found = false;

    while (!found && (lookupLength > 0))
    {
        if ((pattern[fragmentLen] & 0xF00) == 0)
        {
            // zero mask, compare data
            if (operator[](curntAddr) == (pattern[fragmentLen] & 0xFF))
            {
                fragmentLen++;				// data ok, continue search
            }
            else
            {
                bool restart = fragmentLen > 0;
                fragmentLen = 0;			// data mismatch, reset position
                if (restart)
                {
                    // проверяем, если несовпадаемый байт начинает новую последовательность
                    continue;
                }
            }
        }
        else
        {
            // nonzero mask, just increment position
            fragmentLen++;
        }

        if (fragmentLen >= patternLength)
        {
            found = true;
            break;
        }

        curntAddr++;
        lookupLength--;
    }

    if (found)
    {
        return (byte*) curntAddr;
    }

    return 0;
}

bool MemoryAccess::read(void *address, void *data, unsigned length)
{
    if (isLocalProcess_)
    {
        memcpy(data, address, length);
        return true;
    }

    if (hProcess_ == NULL)
    {
        return false;
    }

    SIZE_T bytesRead;
    BOOL result = ReadProcessMemory(hProcess_, (void *) address, data, length, &bytesRead);
    if (result == 0 || bytesRead != length)
    {
        Log("ReadProcessMemory failed (%i)", GetLastError());
        return false;
    }
    return true;
}


bool MemoryAccess::write(void *address, const void *data, unsigned length)
{
    if (!allowWrite(address, length))
    {
        return false;
    }

    if (WriteProcessMemory(hProcess_, address, data, length, NULL) == 0)
    {
        Log("WriteProcessMemory failed with %i", GetLastError());
        return false;
    }

    return true;
}

bool MemoryAccess::allowWrite(void *addr, int length)
{
    if (hProcess_ == 0)
    {
        return false;
    }

    MEMORY_BASIC_INFORMATION mi;
    if (VirtualQueryEx(hProcess_, addr, &mi, sizeof(mi)) == 9)
    {
        Log("VirtualQueryEx failed with %i", GetLastError());
        return false;
    }
    if ((mi.Protect & PAGE_EXECUTE_READWRITE) != 0
        && addr >= mi.AllocationBase
        && ((uintptr_t) addr + length) <= (((uintptr_t) mi.AllocationBase) + mi.RegionSize))
    {
        return true;
    }

    DWORD oldProtect;
    if (!VirtualProtectEx(hProcess_, (void *)addr, length, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        Log("VirtualProtectEx failed with %i", GetLastError());
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
/**
 * Find a sequence of data bytes in memory
 *
 * @param  pid              id of process to find of
 * @param  va_hint          start address to search from
 * @param  lookupLength     length of data to lookup 
 * @param  pattern          pattern of data bytes
 * @param  mask             mask of pattern bytes - any non-zero value indicates byte to match
 * @param  patternLength    length of pattern data
 *
 * @return byte *           address of last byte of first matched pattern
 *                          NULL if pattern was not found
 */
byte * findPattern( DWORD       pid
                  , uintptr_t   va_hint
                  , int         lookupLength
                  , const byte *pattern
                  , const byte *mask
                  , int         patternLength)
{
    MemoryAccess memory(pid);

    uintptr_t   exeBaseAddr_ = memory.processBase();
    uintptr_t   curntAddr    = va_hint + exeBaseAddr_;
    int         fragmentLen  = 0;
    bool        found        = false;

    while ( !found && (lookupLength > 0) )
    {
        if (mask[fragmentLen] != 0)
        {
            // nonzero mask, compare data
            if (memory[curntAddr] == pattern[fragmentLen])
            {
                fragmentLen ++;				// data ok, continue search
            }
            else
            {
                fragmentLen = 0;			// data mismatch, reset position
            }
        }
        else
        {
            // zero mask, just increment position
            fragmentLen ++;
        }

        if (fragmentLen >= patternLength)
        {
            found = true;
            break;
        }

        curntAddr ++;
        lookupLength --;
    }

    if ( !found )
    {
        curntAddr = NULL;
    }

    return (byte*)curntAddr;
}

void allowWrite(DWORD pid, byte *addr, int length)
{
    DWORD oldProtect;

    HANDLE hProcess_ = OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_OPERATION | PROCESS_VM_READ | PROCESS_VM_WRITE
                                  , FALSE, pid);
    if (hProcess_ == NULL)
    {
        throw eCannotOpenProcess;
    }

    if ( ! VirtualProtectEx(hProcess_, (addr), length, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        throw eCannotSetProtect;
    }

    CloseHandle(hProcess_);
}

void allowWrite(byte *addr, int length)
{
    DWORD oldProtect;

    if ( ! VirtualProtect(addr, length, PAGE_EXECUTE_READWRITE, &oldProtect))
    {
        throw eCannotSetProtect;
    }
}

//------------------------------------------------------------------------------
bool isExeCompatible(DWORD pid)
{
    // Return true if exe is compatible

    byte *addr = findStartPattern1(pid);
    if (addr == NULL)
    {
        addr = findStartPattern2(pid);
        if (addr == NULL)
        {
            Log("isExeCompatible: Failed to find start pattern");
            return false;
        }
    }

    return true;
}

bool isAlreadyLoaded(DWORD pid)
{
    // returns alreadyLoaded if dll is already attached to the process

    byte *addr = findStartPattern1(pid);
    if (addr == NULL)
    {
        addr = findStartPattern2(pid);
        if (addr == NULL)
        {
            Log("isLoaded: Failed to find start pattern");
            return false;
        }
    }
    if (*(addr + 1) == 0xCC)
    {
        return true;
    }

    return false;
}

bool setAlreadyLoaded(bool value, DWORD pid)
{
    byte *addr = findStartPattern1(pid);
    if (addr == NULL)
    {
        addr = findStartPattern2(pid);
        if (addr == NULL)
        {
            Log("setLoaded: Failed to find start pattern");
            return false;
        }
    }

    // put a marker to make sure we'll not attach it twice
    addr++;
    allowWrite(pid, addr, 1);

    *addr = value ? 0xCC : 0x00;

    return true;
}

static byte * findStartPattern1(DWORD pid)
{
    const byte rData[] = { 0x8B, 0x44, 0x24, 0x04,  0x56, 0x50, 0x8B, 0xF1
                         , 0xE8, 0x33, 0x22, 0x00,  0x00, 0x32, 0xC0, 0xC7
                         , 0x06, 0x3C, 0xD8, 0x85,  0x00, 0x88, 0x46, 0x08
                         , 0x88, 0x46, 0x18, 0x88,  0x46, 0x1C, 0x88, 0x46
                         , 0x2C, 0x8B, 0xC6, 0x5E,  0xC2, 0x04, 0x00/*, 0x90*/};
    const byte rMask[] = { 0x25, 0x34, 0x58, 0x00,  0x02, 0x00, 0x00, 0x00
                         , 0x24, 0x00, 0x00, 0x00,  0x00, 0x94, 0x36, 0x42
                         , 0x00, 0x00, 0x00, 0x00,  0x00, 0x61, 0x94, 0x12
                         , 0x72, 0x65, 0x05, 0x10,  0x88, 0x84, 0x61, 0x39
                         , 0x71, 0x24, 0x57, 0x5E,  0x82, 0x54, 0x92/*, 0x49*/};

    return findPattern(pid, 0x1000, 0x200, rData, rMask, sizeof(rData));
}

static byte * findStartPattern2(DWORD pid)
{
    // 00401000 8B 44 24 0C                             mov     eax, [esp+arg_8]
    // 00401004 48                                      dec     eax
    // 00401005 78 20                                   js      short locret_401027
    // 00401007 53                                      push    ebx
    // 00401008 8B 5C 24 14                             mov     ebx, [esp+4+arg_C]
    // 0040100C 55                                      push    ebp
    // 0040100D 8B 6C 24 10                             mov     ebp, [esp+8+arg_4]
    // 00401011 56                                      push    esi
    // 00401012 8B 74 24 10                             mov     esi, [esp+0Ch+arg_0]
    // 00401016 57                                      push    edi
    // 00401017 8D 78 01                                lea     edi, [eax+1]
    // 0040101A 8B CE                                   mov     ecx, esi
    // 0040101C FF D3                                   call    ebx
    // 0040101E 03 F5                                   add     esi, ebp
    // 00401020 4F                                      dec     edi
    // 00401021 75 F7                                   jnz     short loc_40101A
    // 00401023 5F                                      pop     edi
    // 00401024 5E                                      pop     esi
    // 00401025 5D                                      pop     ebp
    // 00401026 5B                                      pop     ebx
    // 00401027 C2 10 00                                retn    10h

    const byte rData[] = {
        0x8B, 0x44, 0x24, 0x0C, 0x48, 0x78, 0x20, 0x53,
        0x8B, 0x5C, 0x24, 0x14, 0x55, 0x8B, 0x6C, 0x24,
        0x10, 0x56, 0x8B, 0x74, 0x24, 0x10, 0x57, 0x8D,
        0x78, 0x01, 0x8B, 0xCE, 0xFF, 0xD3, 0x03, 0xF5,
        0x4F, 0x75, 0xF7, 0x5F, 0x5E, 0x5D, 0x5B, 0xC2,
        0x10, 0x00
    };
    const byte rMask[] = {
        0x8B, 0x44, 0x24, 0x0C, 0x48, 0x78, 0x20, 0x53,
        0x8B, 0x5C, 0x24, 0x14, 0x55, 0x8B, 0x6C, 0x24,
        0x10, 0x56, 0x8B, 0x74, 0x24, 0x10, 0x57, 0x8D,
        0x78, 0x01, 0x8B, 0xCE, 0xFF, 0xD3, 0x03, 0xF5,
        0x4F, 0x75, 0xF7, 0x5F, 0x5E, 0x5D, 0x5B, 0xC2,
        0x10, 0x00
    };

    return findPattern(pid, 0x1000, 0x200, rData, rMask, sizeof(rData));
}

//

#include <Psapi.h>
#pragma comment(lib, "Psapi.lib")

bool getClientPids(std::vector<ProcessInfo> & pids)
{
    pids.clear();

    HANDLE processes = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (processes == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    PROCESSENTRY32W  process;
    process.dwSize = sizeof(process);

    if (Process32FirstW(processes, &process) == TRUE)
    {
        do
        {
            if (process.szExeFile != 0
                && _wcslwr_s(process.szExeFile) == 0
                && wcsstr(process.szExeFile, L"elementclient"))
            {
                ProcessInfo p;
                p.pid = process.th32ProcessID;
                p.name = std::wstring(process.szExeFile);

                TCHAR fileName[MAX_PATH] = L"";
                // GetProcessImageFileName(hProcess, fileName, sizeof_array(fileName));
                // GetModuleFileNameEx(hProcess, (HMODULE)0x400000, fileName, sizeof_array(fileName));

                HMODULE hMod;
                DWORD cbNeeded;
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process.th32ProcessID);
                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded) != 0)
                {
                    GetModuleFileNameExW(hProcess, hMod, fileName, sizeof_array(fileName));
                }
                CloseHandle(hProcess);

                p.exeName = std::wstring(fileName);
                pids.push_back(p);
            }
        } while (Process32Next(processes, &process));
    }
    else
    {
        return false;
    }

    CloseHandle(processes);
    return true;
}
