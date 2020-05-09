
#include "stdafx.h"

#include "PWUnpack.h"


CUnpack::CUnpack()
{
    m_PackedOffset = 0;
    m_Stage = 0;
}

barray CUnpack::decompress(const barray & data )
{
    barray result;
    for(size_t i = 0; i < data.size(); i++)
    {
        processByte(result, data[i]);
    }
    return result;
}

void CUnpack::processByte(barray & data, byte InB)
{
    m_Packed.push_back(InB);

    for (;;)
    {
        if (m_Stage == 0)
        {
            if (HasBits(4))
            {
                if (GetPackedBits(1) == 0)
                {
                    // 0-xxxxxxx low byte
                    offsetCode = 1;
                    m_Stage = 1;
                    continue;
                }
                else
                {
                    if (GetPackedBits(1) == 0)
                    {
                        // 10-xxxxxxx high byte
                        offsetCode = 2;
                        m_Stage = 1;
                        continue;
                    }
                    else
                    {
                        if (GetPackedBits(1) == 0)
                        {
                            // 110-xxxxxxxxxxxxx-* offset
                            offsetCode = 3;
                            m_Stage = 1;
                            continue;
                        }
                        else
                        {
                            if (GetPackedBits(1) == 0)
                            {
                                // 1110-xxxxxxxx-* offset
                                offsetCode = 4;
                                m_Stage = 1;
                                continue;
                            }
                            else
                            {
                                // 1111-xxxxxx-* offset
                                offsetCode = 5;
                                m_Stage = 1;
                                continue;
                            }
                        }
                    }
                }
            }
            else
                break;
        }
        else if (m_Stage == 1)
        {
            if (offsetCode == 1)
            {
                if (HasBits(7))
                {
                    byte OutB = byte(GetPackedBits(7));
                    data.push_back(OutB);
                    addUnpacked(OutB);
                    m_Stage = 0;
                    continue;
                }
                else
                    break;
            }
            else if (offsetCode == 2)
            {
                if (HasBits(7))
                {
                    byte OutB = byte(GetPackedBits(7)) | 0x80;
                    data.push_back(OutB);
                    addUnpacked(OutB);
                    m_Stage = 0;
                    continue;
                }
                else
                    break;
            }
            else if (offsetCode == 3)
            {
                if (HasBits(13))
                {
                    offset = GetPackedBits(13) + 0x140;
                    m_Stage = 2;
                    continue;
                }
                else
                    break;
            }
            else if (offsetCode == 4)
            {
                if (HasBits(8))
                {
                    offset = GetPackedBits(8) + 0x40;
                    m_Stage = 2;
                    continue;
                }
                else
                    break;
            }
            else if (offsetCode == 5)
            {
                if (HasBits(6))
                {
                    offset = GetPackedBits(6);
                    m_Stage = 2;
                    continue;
                }
                else
                    break;
            }
        }
        else if (m_Stage == 2)
        {
            if (offset == 0)
            {
                if (m_PackedOffset)
                {
                    m_PackedOffset = 0;
                    m_Packed.erase(m_Packed.begin());
                }
                else
                {
                    assert(m_Packed.size() == 0);
                }
                m_Stage = 0;
                continue;
            }
            m_Code2 = 0;
            m_Stage = 3;
            continue;
        }
        else if (m_Stage == 3)
        {
            if (HasBits(1))
            {
                if (GetPackedBits(1) == 0)
                {
                    m_Stage = 4;
                    continue;
                }
                else
                {
                    m_Code2++;
                    continue;
                }
            }
            else
                break;
        }
        else if (m_Stage == 4)
        {
            DWORD CopySize = 0;
            if (m_Code2 == 0)
                CopySize = 3;
            else
            {
                DWORD Sz = m_Code2 + 1;
                if (HasBits(Sz))
                    CopySize = GetPackedBits(Sz) + (1 << Sz);
                else
                    break;
            }

            Copy(offset, CopySize, data);
            m_Stage = 0;
            continue;
        }
    }
}

void CUnpack::Copy(DWORD Shift, DWORD Size, barray & unpacked)
{
    for (DWORD i = 0; i < Size; i++)
    {
        int PIndex = m_Unpacked.size() - Shift;
        if (PIndex < 0)
        {
            assert(0);
            Notify("Unpack error");
        }
        else
        {
            byte B = m_Unpacked.at(PIndex);
            m_Unpacked.push_back(B);
            unpacked.push_back(B);
        }
    }
}

DWORD CUnpack::GetPackedBits( DWORD BitCount )
{
    if (BitCount > 16)
        return 0;

    if (!HasBits(BitCount))
    {
        assert(0);
        Notify("Unpack bit stream overflow");
    }

    DWORD AlBitCount = BitCount + m_PackedOffset;
    DWORD AlByteCount = (AlBitCount + 7) / 8;

    DWORD V = 0;
    for (DWORD i = 0; i < AlByteCount; i++)
        V |= DWORD(m_Packed[i]) << (24 - i * 8);
    V <<= m_PackedOffset;
    V >>= 32 - BitCount;

    m_PackedOffset = m_PackedOffset + byte(BitCount);
    DWORD FreeBytes = m_PackedOffset / 8;
    if (FreeBytes)
    {
        //m_Packed.pop_front(FreeBytes);
        m_Packed.erase(m_Packed.begin(), m_Packed.begin() +FreeBytes);
    }
    m_PackedOffset %= 8;

    return V;
}

void CUnpack::addUnpacked(byte b)
{
    m_Unpacked.push_back(b);

    if (m_Unpacked.size() > (8192 + 2048))
    {
        m_Unpacked.erase(m_Unpacked.begin(), m_Unpacked.begin() + 2048);
    }
}
