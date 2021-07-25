
#include "stdafx.h"

#include "common.h"
#include "utils.h"
#include "netdata/content.h"
#include "netdata/fragment.h"       // define exceptions


bool CharAttrs::read(SerializerIn & s)
{
    const barray & data = s.stream_;
    barray::const_iterator & offset = s.curPos_;

    size_t bytes_left = data.end() - offset;
    if (bytes_left < 4)
    {
        return false;
    }

    // format parsers
    // 447130 @ 1.4.4 - 0
    // 449030 @ 1.4.4 - 2
    // 46EFC0 @ 1.4.5
    // 44CBC0 @ 1.4.6
    // 458150 @ 1.5.1
    // 00458FD0 @ 1.5.2
    // 004671A0 @ 1.5.3
    // 00467B60 @ 1.5.3.2
    // 0046A4D0 @ 1.5.5
    // 0046A510 @ 1.5.5 ru
    // 004AD480 @ 1.5.5.2 pwi
    // 004B0950 @ 1.5.6
    // 004B2990 @ 1.5.7

    // Actual length calculation is at
    // 631F30 @ 1.4.5
    // 696ED0 @ 1.4.6
    // 6EB7A0 @ 1.5.0
    // 734D00 @ 1.5.1
    // 74A970 @ 1.5.1.1

    // or
    // look for CECPlayer::SetNewExtendStates call

    isEmpty_ = false;

#if PW_SERVER_VERSION < 1470
    copy(offset, offset + 4, primary_);
    offset += 4;
#else
    copy(offset, offset + 8, primary_);
    offset += 8;
#endif

    barray::const_iterator pos = offset;

    if (primary_[1] & 0x04)     // + 22
    {
        offset += 4;      // secondary = pCur + 38;
        offset += 4;
    }

    if (primary_[0] & 0x01)
    {
        offset++;
    }

    if (primary_[0] & 0x02)
    {
        offset++;
    }

    if (primary_[0] & 0x40)
    {
        // CECPlayer::SetNewExtendStates
        offset += 4;
        offset += 4;
        #if PW_SERVER_VERSION >= 1420
            offset += 4;
            offset += 4;
        #endif
        #if PW_SERVER_VERSION >= 1511
            offset += 4;
            offset += 4;
        #endif
        #if PW_SERVER_VERSION >= 1600
            offset += 4;
        #endif
    }

    if (primary_[1] & 0x08)
    {
        inGuild_ = true;
        guildId_ = getDWORD_r(offset);
        //offset += 4; does by get*
        guildRank_ = *(offset++);
    }

    if (primary_[1] & 0x10)
    {
        isCat_ = true;
        offset++;
    }

    if (primary_[2] & 0x01)     // 10000
    {
        byte cl = *(offset++);
        if(cl)
        {
            // array of words
            offset += cl*2;
        }
    }

    if (primary_[0] & 0x08)
    {
        offset++;
    }

    if (primary_[2] & 0x08)     // 0x80000
    {
        #if PW_SERVER_VERSION >= 1442
            offset += 2;
        #else
            offset++;
        #endif
        offset += 4;
    }

    if (primary_[2] & 0x10)     // 0x100000
    {
        offset += 5;
    }

    if (primary_[2] & 0x80)
    {
        offset += 4;
    }

#if PW_SERVER_VERSION >= 1442
    if (primary_[3] & 0x08)  // 0x08000000
    {
        offset += 4;
        // ? #if PW_SERVER_VERSION >= 1450
        offset += 4;
        // #endif
    }
#endif
#if PW_SERVER_VERSION >= 1450
    if (primary_[3] & 0x20)  // 0x20000000
    {
        offset += 4;
    }

    if (primary_[3] & 0x40)  // 0x40000000
    {
        int count = getDWORD_r(offset);
        // some array (dword + byte)
        offset += count * 5;
    }
#endif
#if PW_SERVER_VERSION >= 1460
    if (primary_[3] & 0x80)  // 0x80000000
    {
        offset += 4;
    }
#endif
#if PW_SERVER_VERSION >= 1500
    if(primary_[4] & 0x02)  // +26
    {
        offset += 2;
    }
#endif
#if PW_SERVER_VERSION >= 1510
    if(primary_[4] & 0x04)  // +26
    {
        offset += 1;
    }
    if(primary_[4] & 0x08)
    {
        offset += 1;
    }
#endif
#if PW_SERVER_VERSION >= 1511
    if (primary_[4] & 0x20)
    {
        offset += 1;
    }
#endif
#if PW_SERVER_VERSION >= 1531
    if (primary_[4] & 0x80)
    {
        offset += 8;
    }
#endif
#if PW_SERVER_VERSION >= 1532
    if (primary_[5] & 0x01) // +26  0x100
    {
        offset += 8;
    }
#endif
#if PW_SERVER_VERSION >= 1550
    if (primary_[5] & 0x02) // +26  0x200
    {
        offset += 4;
    }
    if (primary_[5] & 0x04)
    {
        // float
        offset += 4;
    }
#endif
#if PW_SERVER_VERSION >= 1560
    if (primary_[5] & 0x08) // +26  0x800
    {
        offset += 8;
    }
    if (primary_[5] & 0x10)	// 0x1000
    {
        offset += 4;
    }
#endif
#if PW_SERVER_VERSION >= 1570
    if (primary_[5] & 0x20)	// 0x2000
    {
        offset += 4;
    }
#endif
#if PW_SERVER_VERSION >= 1620
	if (primary_[5] & 0x80)	// 0x8000
	{
		offset += 4;
	}
	if (primary_[6] & 0x01)	// 0x10000
	{
		offset ++;
	}
	if (primary_[6] & 0x02)	// 0x20000
	{
		offset += 4;
	}
#endif
#if PW_SERVER_VERSION >= 1640
    if (primary_[6] & 0x04)	// 0x40000
    {
        offset += 8;
    }
#endif
#if PW_SERVER_VERSION >= 1660
    if (primary_[6] & 0x08)	// 0x80000
    {
        offset += 4;
    }
#endif

    additional_.assign(pos, offset);

    isParsed_ = true;
    return isParsed_;
}

void CharAttrs::checkParsed()
{
    if ( ! isParsed_)
    {
        throw eContentError;
    }
}

tostream & operator<<(tostream & stream, CharAttrs attrs)
{
    attrs.checkParsed();

    std::ios_base::fmtflags flags = stream.flags();

    stream << std::hex << std::uppercase << _T('[');
    for (unsigned i = 0; i < sizeof(attrs.primary_); i++)
    {
        stream << std::setw(2) << std::setfill(_T('0')) << attrs.primary_[i] << _T(' ');
    }
    stream << _T('(') << attrs.additional_ << _T(")]");

    if(attrs.inGuild_ == true)
    {
        stream << _T("\r\n     Guild:") << std::setw(8) << std::hex << attrs.guildId_ << _T(" rank:") << attrs.guildRank_;
    }

    stream.flags(flags);
    return stream;
}

tostream & operator<<(tostream & stream, const Coord3D & coords)
{
    std::ios_base::fmtflags flags = stream.flags();

    stream << std::dec << coords.x() << _T(' ') << coords.y() << _T(' ') << coords.z();

    stream.flags(flags);
    return stream;
}
