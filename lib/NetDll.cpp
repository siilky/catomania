#include "stdafx.h"

#include <thread>
#include <mutex>
#include <ctime>
#include <io.h>
#include <fcntl.h>

#include "types.h"
#include "log.h"
#include "NetDll.h"
#include "netdata\serialize.h"


extern "C" typedef unsigned(__stdcall *pFnNoArgs)();

static bool callFunction(pFnNoArgs pFn, unsigned & result, int & errorCode);
static unsigned crc32_1byte(const unsigned char* data, size_t length, uint32_t previousCrc32 = 0);

static int __stdcall alloc_mem(void **a1, int a2)
{
    *a1 = malloc(a2);
    return 0;
}

static int __stdcall free_mem(void *a1)
{
    free(a1);
    return 0;
}

//

NetDll::NetDll()
    : sysHandleInfo_(GetCurrentProcessId())
{
    addFunctionRef(0xFFF0, alloc_mem);
    addFunctionRef(0xFFEF, free_mem);

    hPsapi_ = LoadLibraryW(L"psapi.dll");
    assert(hPsapi_ != 0);
    if (hPsapi_ == 0)
    {
        logMessage("failed lo load psapi.dll");
    }
    if (!sysHandleInfo_.NtDllStatus)
    {
        logMessage("failed lo initialize ntdll helper");
    }

    heap_ = HeapCreate(HEAP_CREATE_ENABLE_EXECUTE, 64 * 1024, 0);
    if (heap_ == NULL)
    {
        logMessage("failed lo initialize heap");
    }
}

NetDll::~NetDll()
{
    for (auto it : stringBlocks_)
    {
        delete it.second;
    }
    for (auto it : functionRefBlocks_)
    {
        delete it.second;
    }
    for (auto it : codeBlocks_)
    {
        HeapFree(heap_, 0, (LPVOID)it.second->rawData);
        delete it.second;
    }
    for (auto it : libraries_)
    {
        FreeLibrary(it.second);
    }

    FreeLibrary(hPsapi_);
    HeapDestroy(heap_);
}

bool NetDll::addCode(unsigned id, const barray & data, const barray & imports)
{
    if (heap_ == NULL)
    {
        return false;
    }

    unsigned crc = crc32_1byte(data.data(), data.size());

    auto iBlock = codeBlocks_.find(id);
    if (iBlock != codeBlocks_.end())
    {
        if (iBlock->second->crc == crc)
        {
            // same data
            return true;
        }

        logMessage("replacing code block %i (%08X)", id, crc);

        HeapFree(heap_, 0, (LPVOID) iBlock->second->rawData);
        delete iBlock->second;
        codeBlocks_.erase(iBlock);
    }


    if (data.empty())
    {
        logMessage("code block %i is empty", id);
        return false;
    }

    byte * mem = (byte*) HeapAlloc(heap_, 0, data.size());
    if (mem == NULL)
    {
        logMessage("failed to allocate memory for code block %i (%08X)", id, crc);
        return false;
    }

    std::tie(iBlock, std::ignore) = codeBlocks_.insert(std::make_pair(id, new Block(mem)));
    std::copy(data.begin(), data.end(), (char*)iBlock->second->rawData);
    iBlock->second->crc = crc;
    saveBlock('c', crc, data, imports);

    // resolve references

    if (!imports.empty())
    {
        SerializerIn s(imports);

        WORD count;
        s.wr(count);
        for (int i = 0; i < count; ++i)
        {
            Block *b = 0;
            const void *ptr = 0;

            WORD refId, refOffset;
            s.wr(refOffset).wr(refId);
            refOffset = refOffset - 6 - imports.size();

            if ((b = getString(refId)) != 0)
            {
                ptr = &b->rawData;
            }
            else if ((b = getfunctionRef(refId)) != 0)
            {
                if (b->rawData == 0)
                {
                    assert(b->data.size() == 4);

                    WORD libraryId = ((WORD*) (b->data.data()))[0];
                    WORD functionId = ((WORD*) (b->data.data()))[1];
                    Block * bLibraryString, *bFunctionString;
                    HMODULE module;
                    if ((bLibraryString = getString(libraryId)) != 0
                        && (bFunctionString = getString(functionId)) != 0
                        && loadLibrary(bLibraryString->data, module))
                    {
                        void *pFn = GetProcAddress(module, (LPCSTR) bFunctionString->data.data());
                        if (pFn != 0)
                        {
                            b->rawData = pFn;
                            ptr = &b->rawData;
                        }
                    }

                    if (ptr == 0)
                    {
                        logMessage("(%08X) failed to resolve function ref %i", crc, refId);
                        //assert(0);
                        return false;
                    }
                }
                else
                {
                    ptr = b->data.empty() ? b->rawData : &b->rawData;
                }
            }
            else
            {
                logMessage("(%08X) missing reference %i", crc, refId);
                //assert(0);
                //return false;
            }

            //assert(ptr != 0);
            if (ptr != 0)
            {
                if (mem[refOffset - 1] == 0xE8)
                {
                    *((DWORD*) (mem + refOffset)) = *(byte**) ptr - (mem + refOffset + 4);
                }
                else
                {
                    *((DWORD*) (mem + refOffset)) = (DWORD) ptr;
                }
            }
        }
    }

    return true;
}

void NetDll::addString(unsigned id, const barray & data)
{
    unsigned crc = crc32_1byte(data.data(), data.size());

    Block *b = getString(id);
    if (b == 0)
    {
        Block *newB = new Block(data);
        newB->rawData = newB->data.data();
        newB->crc = crc;
        stringBlocks_.insert(std::make_pair(id, newB));
        saveBlock('s', crc, data);
    }
    else if (b->crc != crc && b->data != data)
    {
        logMessage("rewriting string block %i (%08X)", id, crc);

        b->data = data;
        b->crc = crc;
        b->rawData = b->data.data();
        saveBlock('s', crc, data);
    }
}

void NetDll::addFunctionRef(unsigned id, const barray & data)
{
    assert(data.size() == 4);

    Block *b = getfunctionRef(id);
    if (b == 0)
    {
        functionRefBlocks_.insert(std::make_pair(id, new Block(data)));
    }
    else if (b->data != data)
    {
        logMessage("replacing function reference block %i", id);

        b->data = data;
        b->rawData = 0;
    }
}

void NetDll::addFunctionRef(unsigned id, void * entry)
{
    Block *b = getfunctionRef(id);
    if (b == 0)
    {
        functionRefBlocks_.insert(std::make_pair(id, new Block(entry)));
    }
    else
    {
        b->rawData = entry;
    }
}

bool NetDll::executeCodeBlock(unsigned id, unsigned & result)
{
    Block * b = getCode(id);
    if (b == 0)
    {
        logMessage("failed to execute missing code %i", id);
        return false;
    }

    pFnNoArgs pFn = (pFnNoArgs) b->rawData;

    assert(pFn != 0);
    if (pFn == 0)
    {
        logMessage("code %i (%08X) has no entry", id, b->crc);
        return false;
    }

    sysHandleInfo_.Refresh();

    std::set<HANDLE> handlesBefore;
    for (auto handle : sysHandleInfo_.m_HandleInfos)
    {
        handlesBefore.insert((HANDLE) handle.HandleValue);
    }

    int errorCode = 0;
    bool success = callFunction(pFn, result, errorCode);
    if (!success)
    {
        logMessage("code %i (%08X) failed with exception: %08X", id, b->crc, errorCode);
    }

    sysHandleInfo_.Refresh();

    std::set<HANDLE> handlesExtra;
    for (auto handle : sysHandleInfo_.m_HandleInfos)
    {
        if (handlesBefore.find((HANDLE) handle.HandleValue) == handlesBefore.end())
        {
            handlesExtra.insert((HANDLE) handle.HandleValue);
        }
    }
    if (!handlesExtra.empty())
    {
        int closed = 0;

        for (auto handle : handlesExtra)
        {
            WORD type;
            if (sysHandleInfo_.GetType(handle, type)
                && type == SystemHandleInformation::TYPE_PROCESS)
            {
                CloseHandle(handle);
                closed++;
            }
        }

        logMessage("block %i (%08X) added %i handles, %i closed", id, b->crc, handlesExtra.size(), closed);
    }

    return success;
}

//

void NetDll::logMessage(const char *format, ...)
{
    static const char *filename = "acd.msg";

    struct _stat stat;
    if (_stat(filename, &stat) != 0 
        || (stat.st_mode & _S_IFREG) == 0
        || (stat.st_mode & _S_IWRITE) == 0)
    {
        return;
    }

    static std::mutex mutex;
    std::lock_guard<std::mutex> lock(mutex);
    {
        int file = 0;
        _sopen_s(&file, filename, _O_BINARY | _O_APPEND | _O_WRONLY, _SH_DENYWR, _S_IREAD | _S_IWRITE);
        if (file < 0)
        {
            return;
        }

        char msg[1024] = { 0, };

        const std::time_t tt = time(0);
        std::tm localTime;
        localtime_s(&localTime, &tt);
        int off = sprintf_s(msg, "[%02i:%02i:%02i %02i-%02i] %p | ", localTime.tm_hour, localTime.tm_min, localTime.tm_sec
                            , localTime.tm_mday, localTime.tm_mon
                            , this);
        if (off < 0)
        {
            strcat_s(msg, "[?] ");
            off = 4;
        }

        va_list ap;
        va_start(ap, format);
        int off2 = vsprintf_s(msg + off, sizeof_array(msg) - off, format, ap);
        va_end(ap);

        Log("%hs", msg + off);

        _write(file, msg, off2 > 0 ? (off2 + off) : off);
        _write(file, "\n", sizeof("\n") - sizeof(""));
        _close(file);
    }
}

NetDll::Block * NetDll::getString(unsigned id)
{
    auto iBlock = stringBlocks_.find(id);
    return iBlock != stringBlocks_.end() ? iBlock->second : 0;
}

NetDll::Block * NetDll::getfunctionRef(unsigned id)
{
    auto iBlock = functionRefBlocks_.find(id);
    return iBlock != functionRefBlocks_.end() ? iBlock->second : 0;
}

NetDll::Block * NetDll::getCode(unsigned id)
{
    auto iBlock = codeBlocks_.find(id);
    return iBlock != codeBlocks_.end() ? iBlock->second : 0;
}

void NetDll::saveBlock(char prefix, unsigned crc, const barray & data, const barray & additional)
{
    struct _stat stat;
    if (_stat("acd.blocks", &stat) != 0 || (stat.st_mode & _S_IFDIR) == 0)
    {
        return;
    }

    char filename[64] = { 0 };
    sprintf_s(filename, "acd.blocks/%c%08X", prefix, crc);

    if (_stat(filename, &stat) == 0 && (stat.st_mode & _S_IFREG) != 0)
    {
        return;
    }
    int file = 0;
    _sopen_s(&file, filename, _O_BINARY | _O_CREAT | _O_WRONLY, _SH_DENYWR, _S_IREAD | _S_IWRITE);
    if (file < 0)
    {
        logMessage("failed to write block %c%08X", prefix, crc);
        return;
    }

    _write(file, data.data(), data.size());
    _close(file);

    if (!additional.empty())
    {
        sprintf_s(filename, "acd.blocks/%c%08Xa", prefix, crc);
        _sopen_s(&file, filename, _O_BINARY | _O_CREAT | _O_WRONLY, _SH_DENYWR, _S_IREAD | _S_IWRITE);
        if (file < 0)
        {
            logMessage("failed to write additional block %c%08Xa", prefix, crc);
            return;
        }

        _write(file, additional.data(), additional.size());
        _close(file);
    }
}

bool NetDll::loadLibrary(const barray & name, HMODULE & module)
{
    if (name.empty())
    {
        return false;
    }

    std::string n;
    n.resize(name.size());
    std::copy(name.begin(), name.end(), n.begin());

    auto iLib = libraries_.find(n);
    if (iLib != libraries_.end())
    {
        module = iLib->second;
        return true;
    }

    HMODULE library = LoadLibraryA(n.data());
    if (library == NULL)
    {
        logMessage("failed lo load library %s : %i", n.data(), GetLastError());
        return false;
    }

    libraries_.insert(std::make_pair(n, library));
    module = library;
    return true;
}

static bool callFunction(pFnNoArgs pFn, unsigned & result, int & errorCode)
{
    __try
    {
        result = pFn();
        return true;
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        errorCode = GetExceptionCode();
        return false;
    }
}

// 

static const uint32_t Crc32Lookup[256] =
{
    //// same algorithm as crc32_bitwise
    //for (int i = 0; i <= 0xFF; i++)
    //{
    //  uint32_t crc = i;
    //  for (int j = 0; j < 8; j++)
    //    crc = (crc >> 1) ^ ((crc & 1) * Polynomial);
    //  Crc32Lookup[0][i] = crc;
    //}
    //// ... and the following slicing-by-8 algorithm (from Intel):
    //// http://www.intel.com/technology/comms/perfnet/download/CRC_generators.pdf
    //// http://sourceforge.net/projects/slicing-by-8/
    //for (int slice = 1; slice < MaxSlice; slice++)
    //  Crc32Lookup[slice][i] = (Crc32Lookup[slice - 1][i] >> 8) ^ Crc32Lookup[0][Crc32Lookup[slice - 1][i] & 0xFF];
        0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,0xE963A535,0x9E6495A3,
        0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,
        0x1DB71064,0x6AB020F2,0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,
        0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,0xFA0F3D63,0x8D080DF5,
        0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,
        0x35B5A8FA,0x42B2986C,0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
        0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,0xCFBA9599,0xB8BDA50F,
        0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,
        0x76DC4190,0x01DB7106,0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,
        0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,0x91646C97,0xE6635C01,
        0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,
        0x65B0D9C6,0x12B7E950,0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
        0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,0xA4D1C46D,0xD3D6F4FB,
        0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,
        0x5005713C,0x270241AA,0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,
        0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,0xB7BD5C3B,0xC0BA6CAD,
        0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,
        0xE3630B12,0x94643B84,0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
        0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,0x196C3671,0x6E6B06E7,
        0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,
        0xD6D6A3E8,0xA1D1937E,0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
        0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,0x316E8EEF,0x4669BE79,
        0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,
        0xC5BA3BBE,0xB2BD0B28,0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
        0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,0x72076785,0x05005713,
        0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,
        0x86D3D2D4,0xF1D4E242,0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,
        0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,0x616BFFD3,0x166CCF45,
        0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,
        0xAED16A4A,0xD9D65ADC,0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
        0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,0x54DE5729,0x23D967BF,
        0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D,
};

static unsigned crc32_1byte(const unsigned char* data, size_t length, uint32_t previousCrc32)
{
    uint32_t crc = ~previousCrc32;

    while (length--)
        crc = (crc >> 8) ^ Crc32Lookup[(crc & 0xFF) ^ *data++];
    return ~crc;
}
