#ifndef MemTools_h
#define MemTools_h

#include "types.h"


class MemoryAccess
{
public:
    MemoryAccess(DWORD pid);
    ~MemoryAccess();

    bool isGood() const;

    byte operator[](uintptr_t va_addr);

    // combined pattern and mask version

    byte * findPattern( unsigned    hintOffset              // считается с 0, означает смещение от базы
                      , int         lookupLength
                      , const unsigned short *pattern
                      , int         patternLength);

    template <size_t size>
    byte * findPattern( unsigned    hintOffset              // считается с 0, означает смещение от базы
                      , int         lookupLength
                      , const unsigned short (&pattern)[size])
    {
        return findPattern(hintOffset, lookupLength, pattern, size);
    }

    bool read(void *address, void *data, unsigned length);

    template <size_t size>
    bool read(void *address, byte(&data)[size])
    {
        return read(address, data, size);
    }

    bool write(void *address, const void *data, unsigned length);

    template <size_t size>
    bool write(void *address, const byte(&data)[size])
    {
        return write(address, data, size);
    }


    unsigned processBase() const
    {
        return processBase_;
    }

    HANDLE processHandle() const
    {
        return hProcess_;
    }

private:
    MemoryAccess(const MemoryAccess & r);
    MemoryAccess & operator=(const MemoryAccess & r);

    bool allowWrite(void *addr, int length);

    const static int        pagesCached_ = 2;
    static size_t           pageSize_;

    unsigned    processBase_;

    bool        isLocalProcess_;
    HANDLE      hProcess_;
    byte       *pages_;             // cached pages of process memory
    uintptr_t   page_va_;           // va addr of first cached page
};


//------------------------------------------------------------------------------
/**
 * Find a sequence of data bytes in memory
 *
 * @param  pid              id of process to find of
 * @param  va_hint          start address to search from
 * @param  lookupLength     length of data to lookup to 
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
                  , int         patternLength);

template <size_t size>
byte * findPattern( DWORD       pid
                  , uintptr_t   va_hint
                  , int         lookupLength
                  , const byte (&pattern)[size]
                  , const byte (&mask)[size])
{
    return findPattern(pid, va_hint, lookupLength, pattern, mask, size);
}

template <size_t size>
byte * findPattern( uintptr_t   va_hint
                  , int         lookupLength
                  , const byte (&pattern)[size]
                  , const byte (&mask)[size])
{
    return findPattern(GetCurrentProcessId(), va_hint, lookupLength, pattern, mask, size);
}


// Allows writing to the memory region
void allowWrite(DWORD pid, byte *addr, int length);
void allowWrite(byte *addr, int length);

//

bool isExeCompatible(DWORD pid = GetCurrentProcessId());
bool isAlreadyLoaded(DWORD pid = GetCurrentProcessId());
bool setAlreadyLoaded(bool value = true, DWORD pid = GetCurrentProcessId());

//

struct ProcessInfo
{
    DWORD           pid;
    std::wstring    name;       // process name
    std::wstring    exeName;    // full path
};

bool getClientPids(std::vector<ProcessInfo> & pids);


#endif