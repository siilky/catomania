#ifndef cunpack_h
#define cunpack_h

#include "common.h"
#include "log.h"

class CUnpack
{
public:
    CUnpack();

    barray decompress(const barray & data);

private:
    void processByte(barray & data, byte InB);

    void Notify(char* Msg)
    {
        Log("%s", Msg);
    }

    bool HasBits(DWORD Count)
    {
        return (m_Packed.size() * 8 - m_PackedOffset) >= Count;
    }

    void Copy(DWORD Shift, DWORD Size, barray& UnpackedChunk);
    DWORD GetPackedBits(DWORD BitCount);
    void addUnpacked(byte b);

    // Data

    DWORD offsetCode;
    DWORD m_Code2;
    DWORD m_Stage;
    DWORD offset;
    byte m_PackedOffset;
    barray m_Packed;
    barray m_Unpacked;
};

#endif
