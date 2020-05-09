#ifndef _netdll_h_
#define _netdll_h_

#include <unordered_map>
#include "types.h"
#include "SystemInfo.h"


class NetDll
{
public:
    NetDll();
    virtual ~NetDll();

    bool addCode(unsigned id, const barray & data, const barray & imports);
    void addString(unsigned id, const barray & data);
    void addFunctionRef(unsigned id, const barray & data);
    void addFunctionRef(unsigned id, void * entry);
    bool executeCodeBlock(unsigned id, unsigned & result);


private:
    struct Block
    {
        Block(const barray & d)
            : data(d)
            , rawData(0)
        { }
        Block(void * p)
            : rawData(p)
        { }

        barray      data;
        unsigned    crc;
        const void  *rawData;

    private:
        Block(const Block &);
        Block & operator=(const Block &);
    };

    void logMessage(const char *format, ...);

    Block * getString(unsigned id);
    Block * getfunctionRef(unsigned id);
    Block * getCode(unsigned id);

    void saveBlock(char prefix, unsigned crc, const barray & data, const barray & additional = barray());
    bool loadLibrary(const barray & name, HMODULE & module);

    std::map<unsigned, Block *> stringBlocks_;
    std::map<unsigned, Block *> functionRefBlocks_;
    std::map<unsigned, Block *> codeBlocks_;

    std::unordered_map<std::string, HMODULE> libraries_;
    
    HMODULE hPsapi_;
    HANDLE  heap_;
    SystemHandleInformation     sysHandleInfo_;
};

#endif
