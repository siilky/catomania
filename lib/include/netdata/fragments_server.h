#ifndef fragments_server_h
#define fragments_server_h

#include "netdata/fragments.h"
#include "netdata/fragments_giserver.h"
#include "netdata/serialize.h"
//#include "PWUnpack.h"
#include "netio/mppc.h"

#define HANDLE_S(cookies, connection, class, name)  cookies.push_back(connection->bindServerHandler                 \
                                                        (std::function<void (const serverdata::Fragment##name *)>   \
                                                        (std::bind(&##class::on##name, this, std::placeholders::_1) )))

namespace serverdata
{

    extern FragmentFactory fragmentFactory;

    //------------------------------------------------------------------------------
    class FragmentArray : public FragmentSet
    {
    public:
        enum { ID = 0x0000 };

        FragmentArray(FragmentBase *fragment)
            : FragmentSet(fragment)
        {
            id_ = ID;
            isOk_ = true;
            isParsed_ = true;
        }
        FragmentArray(const barray & dataStream, barray::const_iterator & curPos)
            : FragmentSet(dataStream, curPos, &fragmentFactory)
        {}

        virtual barray assemble()
        {
            return FragmentSet::assemble(ID);
        }
    };

    class FragmentArrayPacked : public FragmentSet
    {
    public:
        enum { ID = 0x0010 };

        /*FragmentArrayPacked(FragmentBase *fragment)
            : FragmentSet(fragment)
        {
            id_ = ID;
            isOk_ = true;
            isParsed_ = true;
        }*/
        FragmentArrayPacked(const barray & dataStream, barray::const_iterator & curPos)
            : FragmentSet(dataStream, curPos)
        {
            if (!isOk_)
            {
                return;
            }

            barray packedData;
            packedData.swap(payload_);
            auto pos = packedData.begin();

            auto lUnpacked = getCUI(packedData, pos);
            auto lPacked = getCUI(packedData, pos);
            assert(lPacked <= unsigned(packedData.end() - pos));

            if (lPacked < lUnpacked)
            {
                if (lUnpacked > 0x2000)
                {
                    auto bytesLeft = lPacked;
                    while (bytesLeft >= 2)
                    {
                        auto chunkSize = getWORD_r(pos);
                        if ((chunkSize & 0x8000) != 0)
                        {
                            chunkSize &= 0x7FFF;
                            MPPCDecoder unpack;

                            auto d = unpack.transform(barray(pos, pos + chunkSize));
                            pos += chunkSize;
                            payload_.insert(payload_.end(), d.begin(), d.end());
                        }
                        else
                        {
                            payload_.insert(payload_.end(), pos, pos + chunkSize);
                            pos += chunkSize;
                        }
                        bytesLeft -= 2 + chunkSize;
                    }
                    assert(bytesLeft == 0);
                }
                else
                {
                    MPPCDecoder unpack;
                    payload_ = unpack.transform(barray(pos, pos + lPacked));;
                }
            }
            else
            {
                packedData.erase(packedData.begin(), pos);
                payload_.swap(packedData);
            }

            assert(payload_.size() == lUnpacked);

            barray::const_iterator cur = payload_.begin();
            while (cur < payload_.end())
            {
                FragmentBase *newFragment = fragmentFactory.create(payload_, cur);
                fragmentArray_.push_back(newFragment);
                if (newFragment == NULL || !newFragment->isOk())
                {
                    return;
                }
            }

            assert(cur == payload_.end());
            if (cur == payload_.end())
            {
                isOk_ = true;
            }
        }

        virtual barray assemble()
        {
            return FragmentSet::assemble(ID);
        }
    };


    // ------------------------------------------------------------------------------
    class FragmentGameinfoSet : public FragmentSet
    {
    public:
        enum { ID = 0x0022 };

        FragmentGameinfoSet(FragmentBase *fragment)
            : FragmentSet(fragment)
        {}
        FragmentGameinfoSet(const barray & dataStream, barray::const_iterator & curPos)
            : FragmentSet(dataStream, curPos, &fragmentGiFactory)
        {}

        virtual barray assemble()
        {
            return FragmentSet::assemble(ID);
        }

        virtual void print(tostream &stream) const
        {
            checkOk();
            checkParsed();

            stream << hex << std::uppercase << _T(" GI [") << id_ << _T("] L=") << payload_.size() << _T(" (") << dec << fragmentArray_.size() << (')') << endl;

            for (fragmentArray_t::const_iterator it = fragmentArray_.begin(); it != fragmentArray_.end(); ++it)
            {
                stream << (*it);
            }
        }
    };

    // ------------------------------------------------------------------------------

    class AccInfoImpl
    {
    public:
        enum { ID = 0x0004 };

        DWORD   accId;
        byte    unk;
        DWORD   unk2;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(accId).b(unk).l(unk2);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Acc Info]  accId:") << hex << setw(8) << accId << _T(" unk:") << unk << _T("  unk2:") << unk2 << endl;
        }
    };

    typedef FragmentSpec<AccInfoImpl>   FragmentAccInfo;


    class ErrorInfoImpl
    {
    public:
        enum { ID = 0x0005 };

        #if PW_SERVER_VERSION >= 1500
            DWORD   code;
        #else
            byte    code;
        #endif

        string  baseMessage;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            #if PW_SERVER_VERSION >= 1500
                s.l(code).c(baseMessage);
            #else
                s.b(code).c(baseMessage);
            #endif
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Error Info]  code:") << code << _T(" Base message:") << strToWStr(baseMessage) << endl;
        }
    };

    typedef FragmentSpec<ErrorInfoImpl>   FragmentErrorInfo;


    class StatusAnnounceImpl
    {
    public:
        enum { ID = 0x0007 };

        DWORD   u0;
        WORD    u1;
        DWORD   u2;
        wstring name;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(u0).w(u1).l(u2).s(name);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Status Announce]  Name:") << name
                << _T(" u0:") << hex << setw(8) << u0
                << _T(" u1:") << u1
                << _T(" u2:") << u2<< endl;
        }
    };

    typedef FragmentSpec<StatusAnnounceImpl>   FragmentStatusAnnounce;



    class PlayerLogoutImpl
    {
    public:
        enum { ID = 0x0045 };

        DWORD   type;   // enum PlayerLogoutType
        DWORD   charId;
        DWORD   unk2;
        DWORD   unk3;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(type).l(charId).l(unk2).l(unk3);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Player Logout]  CharId:") << hex << setw(8) << charId << _T(" type:") << type << _T("  unk2:") << unk2 <<_T(" unk3:") << unk3 << endl;
        }
    };

    typedef FragmentSpec<PlayerLogoutImpl>   FragmentPlayerLogout;


    class SelectRoleReImpl
    {
    public:
        enum { ID = 0x0047 };

        enum // code
        {
            Success = 0,
        };


        DWORD   code;
        byte    unk;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(code).b(unk);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [SelectRoleRe]  res:") << hex << setw(8) << code << _T(" unk:") << unk << endl;
        }
    };

    typedef FragmentSpec<SelectRoleReImpl>   FragmentSelectRoleRe;


    class ChatMessageImpl
    {
    public:
        enum { ID = 0x0050 };

        enum
        {
            ChatTypePublic  = 0x00,
            ChatTypeGroup   = 0x02,
            ChatTypeSystem  = 0x09,
        };

        struct Plaintext  // 00 xx; - public; 02 00 - пати
        {
            DWORD           id;         // 00 - система
            std::wstring    message;
        #if PW_SERVER_VERSION >= 1422   // 1422 +-?
            barray          itemData;   // inventory type (l) slot (w)
            DWORD           u1;
        #endif
        };

        //   [50] [Chat Message]  Type:87A
        //   08 7A 00 00 00 3B 08 00 2A 4D C0 00 00 00 09 00 .z...;..*M......
        //   00 00 00 00  
        // на выборах лидирует раммштайн 9 голосов

        struct TwZones      // 08 99
        {
            struct TwZone
            {
                byte            zoneNum;
                DWORD           owningClan;
                DWORD           attackingClan;
                byte            u1;
            };

            DWORD               id;
            DWORD               u1;
            std::vector<TwZone> zones;
            // zones tail
            WORD                u2;
            byte                u3;
        };

        byte        type;
        byte        u0;
        Plaintext   plaintext;
        TwZones     twZones;

    protected:
        template<int mode>
        void format(Serializer<mode> & /*s*/)     { assert(0); }

        template<>
        void format(SerializerIn & s)
        {
            s.b(type).b(u0);
            if (type == ChatTypeSystem || type == ChatTypePublic || type == ChatTypeGroup)
            {
                s.l(plaintext.id).s(plaintext.message);
            #if PW_SERVER_VERSION >= 1422   // 1422 +-?
                s.arr(plaintext.itemData, &SerializerIn::cui, &SerializerIn::b)
                    .l(plaintext.u1);
            #endif
            }
            else if (type == 0x0899)
            {
                s.l(twZones.id).l(twZones.u1);

                byte zoneNum;
                s.b(zoneNum);
                while (zoneNum != 0)
                {
                    TwZones::TwZone z;
                    z.zoneNum = zoneNum;

                    s.l(z.owningClan).l(z.attackingClan).b(z.u1);
                    s.b(zoneNum);

                    twZones.zones.push_back(z);
                }

                s.w(twZones.u2).b(twZones.u3);
            }
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Chat Message]  Type:") << hex << type << " " << hex << u0;

            if (type == ChatTypeSystem || type == ChatTypePublic || type == ChatTypeGroup)
            {
                stream << _T(" id:") << setw(8) << plaintext.id
                    << _T(" : ") << plaintext.message
                #if PW_SERVER_VERSION >= 1422   // 1422 +-?
                    << endl << _T("  ItemData: ") << plaintext.itemData
                #endif
                    << endl;
            }
            else if (type == 0x0899)
            {
                stream << _T(" id:") << hex << setw(8) << twZones.id << _T(" u1:") << twZones.u1
                    << _T(" Zones:") << dec << twZones.zones.size() << endl;

                for (std::vector<TwZones::TwZone>::const_iterator it = twZones.zones.begin();
                    it != twZones.zones.end(); ++it)
                {
                    stream << _T("     Zone:") << dec << it->zoneNum
                        << _T(" owner:") << hex << setw(8) << it->owningClan
                        << _T(" attacker:") << hex << setw(8) << it->attackingClan
                        << _T(" u1:") << hex << setw(2) << int(it->u1) << endl;
                }
            }
            stream << endl;
        }
    };

    typedef FragmentSpec<ChatMessageImpl>   FragmentChatMessage;


    class RoleListReImpl
    {
    public:
        enum { ID = 0x0053 };

        DWORD       unk;
        DWORD       slot;
        DWORD       accId;
        DWORD       connectionId;
        byte        hasChar;

        // char
        DWORD       charId;
        byte        gender;
        byte        race;
        byte        profession;
        DWORD       level;
        DWORD       level2;
        wstring     name;
        CharacterRecord charRecord;

        WORD        unk1;
        DWORD       unk2;
        DWORD       unk3;
        //there's more


    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(unk).l(slot).l(accId).l(connectionId).b(hasChar);
            if (hasChar != 0)
            {
                s.l(charId).b(gender).b(race).b(profession).l(level).l(level2).s(name);
                charRecord.format(s);
                s.w(unk1).l(unk2).l(unk3);
            }
            else
            {
                charId = 0;
            }
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Char Slot]  accId:") << hex << setw(8) << accId 
                    << _T(" Conn:") << connectionId << _T(" Slot:") << slot;
            if (hasChar != 0)
            {
                stream << setw(8) << hex 
                    << _T(" CharID:") << charId
                    << dec
                    << _T("  G:") << (int)gender
                    << _T("  R:") << (int)race
                    << _T("  P:") << (int)profession
                    << _T("  Name:") << name
                    << endl;
            }
        }
    };

    typedef FragmentSpec<RoleListReImpl>   FragmentRoleListRe;


    class CreateRoleReImpl
    {
    public:
        enum { ID = 0x0055 };

        DWORD   result;
        DWORD   charId;
        DWORD   connectionId;   // 0
        DWORD   charId2;        // twice?
        byte    gender;
        byte    race;
        byte    profession;
        DWORD   level;
        DWORD   level2;
        std::wstring    name;
        CharacterRecord charRecord;

        barray          customStatus;
        barray          charMode;
        DWORD           referrerRole;
        DWORD           cashAdd;
        barray          reincarnationData;
        barray          realmData;
        DWORD           refRefCode;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(result).l(charId).l(connectionId).l(charId2).b(gender).b(race).b(profession)
                .l(level).l(level2).s(name);
            charRecord.format(s);
            s.arr(customStatus, &Serializer<mode>::cui, &Serializer<mode>::b)
                .arr(charMode, &Serializer<mode>::cui, &Serializer<mode>::b)
                .l(referrerRole).l(cashAdd)
                .arr(reincarnationData, &Serializer<mode>::cui, &Serializer<mode>::b)
                .arr(realmData, &Serializer<mode>::cui, &Serializer<mode>::b)
                .l(refRefCode);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [CreateRoleRe]  CharId:") << hex << (unsigned) charId
                << endl;
        }
    };

    typedef FragmentSpec<CreateRoleReImpl>   FragmentCreateRoleRe;


    class KeepaliveImpl
    {
    public:
        enum { ID = 0x005A };

        byte    unk;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.b(unk);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Keepalive]  Unk:") << hex << (unsigned)unk << endl;
        }
    };

    typedef FragmentSpec<KeepaliveImpl>   FragmentKeepalive;


    class PlayerBaseInfoReImpl
    {
    public:
        enum { ID = 0x005C };

        DWORD       id;
        wstring     charName;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.skip(13).l(id).s(charName);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Player Base Info]  Id:") << setw(8) << hex << id << _T(" Name: ") << charName << endl;
        }
    };

    typedef FragmentSpec<PlayerBaseInfoReImpl>   FragmentPlayerBaseInfoRe;


    class PrivateChatImpl
    {
    public:
        enum { ID = 0x0060 };

        enum
        {
            ChatTypePrivate     = 0x00,
            ChatTypeSystem      = 0x04,
            ChatTypeDedicated   = 0x07,
        };

        byte        type;
        byte        u0;
        wstring     fromName;
        DWORD       fromId;
        wstring     myName;
        DWORD       myId;
        wstring     text;
#if PW_SERVER_VERSION >= 1422   // 1422 +-?
        barray      itemData;
        DWORD       u1;
#endif

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.b(type).b(u0).s(fromName).l(fromId).s(myName).l(myId).s(text);
#if PW_SERVER_VERSION >= 1422
            s.arr(itemData, &Serializer<mode>::cui, &Serializer<mode>::b)
                .l(u1);
#endif
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Private chat]  Type:") << hex << type
                << _T(" u0:") << u0
                << _T(" From:") << setw(8) << fromId << _T(" '") << fromName
                << _T("'  To:") << myId << _T(" '") << myName
                << _T("' ::") << text << endl
            #if PW_SERVER_VERSION >= 1422
                << _T("  itemData: ") << itemData
                << _T(" u1:") << u1
            #endif
                << endl;
        }
    };

    typedef FragmentSpec<PrivateChatImpl>   FragmentPrivateChat;


    class BannedMessageImpl
    {
    public:
        // response to SelectRole C46 or C02
        enum { ID = 0x007B };

        DWORD       charId;
        DWORD       connectionId;
        byte        unk;
        DWORD       secsRemaining;
        DWORD       banTs;      // this is negative sometimes
        wstring     message;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(charId).l(connectionId).b(unk).l(secsRemaining).l(banTs).s(message);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Banned Message]  Conn:") << setw(8) << hex << connectionId
                << _T(" Char: ") << charId
                << _T(" u: ") << (int)unk
                << _T(" TS: ") << banTs
                << _T(" Remaining: ") << secsRemaining
                << endl
                << _T(" Message: ") << message
                << endl;
        }
    };

    typedef FragmentSpec<BannedMessageImpl>   FragmentBannedMessage;


    class WorldChatImpl
    {
    public:
        enum { ID = 0x0085 };

        enum
        {
            ChatTypeWorld   = 0x01,
            ChatTypeHorn    = 0x0C,
        };

        // горн "мур"
        // ==> [15:49:16.765]
        //  [4F] L=22
        //   0C 00 00 08 62 A0 00 00 00 00 16 3C 04 43 04 40 ....b......<.C.@
        //   04 66 00 66 00 39 00 62 00 33 00 65 00 30 00 30 .f.f.9.b.3.e.0.0
        //   00 00                                           ..
        // <== [15:49:16.816]
        //   [85] [WorldChat]  Type:C
        // 
        //   0C 06 00 08 62 A0 08 49 00 6E 00 66 00 79 00 16 ....b..I.n.f.y..
        //   3C 04 43 04 40 04 66 00 66 00 39 00 62 00 33 00 <.C.@.f.f.9.b.3.
        //   65 00 30 00 30 00 00
        // горн "фыр" зеленым цветом
        // ==> [16:04:30.782]
        //  [4F] L=22
        //   0C 00 00 08 62 A0 00 00 00 00 16 44 04 4B 04 40 ....b......D.K.@
        //   04 34 00 39 00 63 00 37 00 35 00 37 00 30 00 31 .4.9.c.7.5.7.0.1
        //   00 00                                           ..
        // <== [16:04:30.833]
        //   [85] [WorldChat]  Type:C
        // 
        //   0C 06 00 08 62 A0 08 49 00 6E 00 66 00 79 00 16 ....b..I.n.f.y..
        //   44 04 4B 04 40 04 34 00 39 00 63 00 37 00 35 00 D.K.@.4.9.c.7.5.
        //   37 00 30 00 31 00 00                            7.0.1..
        //  ---
        struct            Plaintext  // 01(00) - мир, 0—(06) - горн
        {
            byte            id;
            DWORD           charId;
            std::wstring    charName;
            std::wstring    message;
        #if PW_SERVER_VERSION >= 1422   // 1422 +-?
            barray          itemData;
        #endif
        };

        struct WeiMessage   // 08 00
        {
            DWORD           u1;
            WORD            u2;     // 0x38
            DWORD           itemId;
            wstring         text;   // 0x34
            byte            u3;     // 0
        };

        //  [85] [WorldChat]  Type:8
        //  08 02 00 00 00 18 00 3C C2 EB C2 00 A0 6C 00 00 ....... < .....l..
        //  09 00 00 00 00 00 00 00 01 00 00 00 61 00 70 00 ............a.p.
        //  6F 00 73 00 74 00 6F 00 6C 00 6C 00 00 00 00 00 o.s.t.o.l.l.....
        //  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ................
        //  00 00 00 00 00
        // apostoll вам улыбнулась удача - вы получаете питомца - маленького дракончика!
        //
        struct TwBid        // 08 05
        {
            std::wstring    clanName;
            byte            u1;
            byte            zoneNum;
            DWORD           u2;
            DWORD           u3;
            DWORD           u4;
            DWORD           closingTime;
            byte            u5;
        };

        byte        type;
        byte        u1;
        DWORD       subType;
        Plaintext   plaintext;
        TwBid       twBid;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.b(type);
            if (type == ChatTypeWorld || type == ChatTypeHorn)
            {
                s.b(plaintext.id).l(plaintext.charId).s(plaintext.charName).s(plaintext.message);
            #if PW_SERVER_VERSION >= 1422   // 1422 +-?
                s.arr(plaintext.itemData, &Serializer<mode>::cui, &Serializer<mode>::b);
            #endif
            }
            else if (type == 0x08) // GMAnnounce
            {
                s.b(u1).l(subType);

                if (subType == 0x05)    // TW bid
                {
                    s.s(twBid.clanName).b(twBid.u1).b(twBid.zoneNum)
                        .l(twBid.u2).l(twBid.u3).l(twBid.u4).l(twBid.closingTime).b(twBid.u5);
                }
            }
        }

        void print(tostream & stream) const
        {
            stream << _T(" [WorldChat]  Type:") << hex << type;

            if (type == 0x01)
            {
                stream << _T(" charId:") << setw(8) << plaintext.charId << _T(" | ") 
                    << plaintext.charName << _T(" : ") << plaintext.message;
            }
            else if (type == 0x08 && subType == 0x05)
            {
                stream << _T(" id:") << _T(" Zone:") << dec << twBid.zoneNum
                    << _T(" Clan:") << twBid.clanName
                    << _T(" Closing:") << hex << setw(8) << twBid.closingTime << endl
                    << _T("     u0:") << int(u1)
                    << _T(" u1:") << int(twBid.u1)
                    << _T(" u5:") << int(twBid.u5)
                    << _T(" u2:") << setw(8) << twBid.u2 
                    << _T(" u3:") << setw(8) << twBid.u3 
                    << _T(" u4:") << setw(8) << twBid.u4;
            }
            stream << endl;
        }
    };

    typedef FragmentSpec<WorldChatImpl>   FragmentWorldChat;


    class LoginIpInfoImpl
    {
    public:
        enum { ID = 0x008F };

        DWORD   accId;
        DWORD   connectionId;
        DWORD   lastLoginTs;
        DWORD   lastIp;
        DWORD   currentIp;

        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(accId).l(connectionId).l(lastLoginTs).l(lastIp).l(currentIp);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Login IP Info] ") << setw(8) << hex << accId << _T(" Conn:") << connectionId
                << _T(" TS:") << lastLoginTs << _T(" lastIP:") << lastIp << _T(" curIP:") << currentIp << endl;
        }
    };

    typedef FragmentSpec<LoginIpInfoImpl>   FragmentLoginIpInfo;


    class GetFriendsReImpl
    {
    public:
        enum { ID = 0x00CF };

        struct Friend
        {
            DWORD   id;
            WORD    cl;
            wstring name;

            template<int mode> void format(Serializer<mode> & s)
            {
                s.l(id).wr(cl).s(name);
            }
        };

        DWORD       myId;
        byte        unk1;
        byte        count;
        byte        oCount;
        DWORD       unk2;
        std::vector<struct Friend>  friends;
        std::vector<byte>           onlineIndexes;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(myId).b(unk1).arr(friends, &Serializer<mode>::b).b(oCount);
            s.arr(onlineIndexes, oCount, &Serializer<mode>::b).l(unk2);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Get Friends Re] count:") << dec << friends.size() << endl << _T("   MyID:") 
                   << setw(8) << hex << myId << _T(" unk:") << unk1 << _T(" unk2:") << unk2 << endl;

            for (std::vector<struct Friend>::const_iterator it = friends.begin();
                it != friends.end(); ++it)
            {
                stream << _T("    id:") << setw(8) << hex << it->id << _T(" name:") << it->name
                       << _T(" class:") << dec << it->cl << endl;
            }
            
            stream << _T("  set online: ");
            for (std::vector<byte>::const_iterator it = onlineIndexes.begin();
                it != onlineIndexes.end(); ++it)
            {
                stream << setw(8) << hex << *it << _T(" : ");
            }
            stream << endl;
        }
    };

    typedef FragmentSpec<GetFriendsReImpl>   FragmentGetFriendsRe;


    class SetLockTimeReImpl
    {
    public:
        enum { ID = 0x030F };

        DWORD   status;     // 0 ok, 1 failed
        DWORD   seconds;
        DWORD   connectionId;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(status).l(seconds).l(connectionId);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Set Lock Time Re] ") << dec 
                << _T(" status:") << status
                << _T(" seconds:") << seconds
                << setw(8) << hex << _T(" connId:") << connectionId
                << endl;
        }
    };

    typedef FragmentSpec<SetLockTimeReImpl>   FragmentSetLockTimeRe;


    class BattleGetMapReImpl
    {
    public:
        enum { ID = 0x353 };

        struct Zone
        {
            byte    zoneNumber; 
            byte    zoneLevel; 
            byte    u0; 
            DWORD   owningClan;
            DWORD   attackingClan;
            DWORD   battleTime;
            DWORD   u1;         // чтото ставка, чтото откат
            DWORD   u2;

            template<int mode> void format(Serializer<mode> & s)
            {
                s.b(zoneNumber).b(zoneLevel).b(u0)
                    .l(owningClan).l(attackingClan).l(battleTime).l(u1).l(u2);
            }
        };

        DWORD   unk1;
        DWORD   unk2;
        std::vector<Zone>  zones;
        DWORD   unk3;

        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(unk1).l(unk2).arr(zones, &Serializer<mode>::b).l(unk3);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Zone Map] count:") << dec << zones.size()
                << _T("   Unk1:") << hex << setw(8) << unk1 << _T(" Unk2:") << setw(8) << unk2
                << _T(" Unk3:") << setw(8) << unk3 << endl;

            for (std::vector<Zone>::const_iterator it = zones.begin();
                it != zones.end(); ++it)
            {
                stream << _T("  Zone:") << dec << setw(2) << it->zoneNumber << _T(" Lv:") << it->zoneLevel
                    << _T(" u:") << it->u0
                    << _T(" Own:") << setw(8) << hex << it->owningClan << _T(" Atk:") << setw(8) << it->attackingClan
                    << _T(" Time:") << setw(8) << it->battleTime
                    << _T(" u1:") << setw(8) << it->u1 << _T(" u2:") << setw(8) << it->u2 << endl;
            }
        }
    };

    typedef FragmentSpec<BattleGetMapReImpl>   FragmentBattleGetMapRe;


    class BattleChallengeMapReImpl
    {
    public:
        enum { ID = 0x357 };

        struct Zone
        {
            WORD    zoneNumber; 
            DWORD   id1;
            DWORD   id2;
            DWORD   chargeback;
            DWORD   timestamp;

            template<int mode> void format(Serializer<mode> & s)
            {
                s.w(zoneNumber).l(id1).l(id2).l(chargeback).l(timestamp);
            }
        };

        DWORD   myId;
        DWORD   unk1;
        WORD    unk2;
        DWORD   unk3;
        std::vector<Zone>  zones;
        DWORD   unk4;
        DWORD   unk5;

        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(myId).l(unk1).w(unk2).l(unk3).arr(zones, &Serializer<mode>::b)
                .l(unk4).l(unk5);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Zone Challenge Map]  ")
                << _T("   Unk1:") << hex << setw(8) << unk1 
                << _T(" Unk2:") << setw(8) << (int)unk2
                << _T(" Unk3:") << setw(8) << unk2
                << _T(" Unk4:") << setw(8) << unk4
                << _T(" Unk5:") << setw(8) << unk5
                << _T(" count:") << dec << zones.size()
                << endl;

            for (std::vector<Zone>::const_iterator it = zones.begin();
                it != zones.end(); ++it)
            {
                stream << _T("  Zone:") << dec << setw(2) << int(it->zoneNumber)
                    << _T(" Id1:") << setw(8) << hex << it->id1
                    << _T(" Id2:") << setw(8) << it->id2
                    << _T(" Cb:") << dec << it->chargeback
                    << _T(" TS:") << setw(8) << hex << it->timestamp << endl;
            }
        }
    };

    typedef FragmentSpec<BattleChallengeMapReImpl>   FragmentBattleChallengeMapRe;


    class ComissionShopImpl
    {
    public:
        enum { ID = 0x39D };

        struct Item
        {
            DWORD   id;
            DWORD   slot;
            DWORD   count;
            DWORD   unk1;
            barray  attrs;
            //byte[20] unk2;
            DWORD   price;
            DWORD   unk3;
            DWORD   unk4;

            template<int mode> void format(Serializer<mode> & s)
            {
                s.l(id).l(slot).l(count).l(unk1)
                    .arr(attrs, &Serializer<mode>::cui, &Serializer<mode>::b)
                    .skip(20)
                    .l(price).l(unk3).l(unk4);
            }
        };

        DWORD   id;
        DWORD   unk1;
        DWORD   unk2;
        DWORD   unk3;       // unk1 в листе
        std::vector<Item>  buyItems;
        std::vector<Item>  sellItems;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(unk1).l(unk2).l(id).l(unk3)
                .arr(buyItems, &Serializer<mode>::b)
                .arr(sellItems, &Serializer<mode>::b);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [ComissionShop]  ID:") << setw(8) << hex << id
                << _T(" unk1:") << setw(8) << unk1
                << _T(" unk2:") << setw(8) << unk2
                << _T(" unk3:") << setw(8) << unk3
                << endl;

            auto printItem = [&stream](const Item & i)
            {
                stream << _T(" ID:") << setw(8) << hex << i.id
                    << _T(" slot:") << dec << i.slot
                    << _T(" count:") << i.count
                    << _T(" price:") << i.price
                    << _T(" unk1:") << setw(8) << i.unk1
                    << _T(" unk3:") << setw(8) << i.unk3
                    << _T(" unk4:") << setw(8) << i.unk4
                    << _T(" A:") << i.attrs
                    << endl;
            };

            stream << _T("  Buy") << endl;
            for (const Item & item : buyItems)
            {
                printItem(item);
            }
            stream << _T("  Sell") << endl;
            for (const Item & item : sellItems)
            {
                printItem(item);
            }
        }
    };

    typedef FragmentSpec<ComissionShopImpl>   FragmentComissionShop;


    class ComissionShopListImpl
    {
    public:
        enum { ID = 0x39F };

        struct Shop
        {
            DWORD   id;
            DWORD   unk1;
            DWORD   time;
            DWORD   type;

            template<int mode> void format(Serializer<mode> & s)
            {
                s.l(id).l(unk1).l(time).l(type);
            }
        };

        DWORD   unk1;
        std::vector<Shop>  shops;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(unk1).arr(shops, &Serializer<mode>::cui);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [ComissionShopList]  unk1:") << setw(8) << hex << unk1 << endl;

            auto printShop = [&stream](const Shop & i)
            {
                stream << _T(" ID:") << setw(8) << hex << i.id
                    << _T(" unk1:") << setw(8) << i.unk1
                    << _T(" date:") << setw(8) << i.time
                    << _T(" type:") << setw(8) << i.type
                    << endl;
            };

            for (const Shop & shop : shops)
            {
                printShop(shop);
            }
        }
    };

    typedef FragmentSpec<ComissionShopListImpl>   FragmentComissionShopList;


    class TradeStartReImpl
    {
    public:
        enum { ID = 0xFA2 };

        DWORD status;       // 0 - ok, 1 - error
        DWORD tradeId1;     // 0 if error
        DWORD tradeId2;     // то же, что и в FA3
        DWORD playerId;     // обнул€етс€ в случае ошибки,
        DWORD myId;

        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(status).l(tradeId1).l(playerId).l(myId).l(tradeId2);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Trade Srart RE]  S:") << hex << status
                << _T(" P1:") << setw(8) << playerId
                << _T(" my:") << setw(8) << myId
                << _T(" id1:") << setw(8) << tradeId1
                << _T(" id2:") << setw(8) << tradeId2 
                << endl;
        }
    };

    typedef FragmentSpec<TradeStartReImpl>   FragmentTradeStartRe;


    class TradeRequestImpl
    {
    public:
        enum { ID = 0xFA3 };

        DWORD tradeId;      // 0x80000000 | id идентификатор запроса. “аймаут запроса никак не индицируетс€ тому кому направлен трейд
        DWORD myId;
        DWORD tradeId2;     // TID2 обычно не мен€етс€ при запросах от разных людей
        DWORD playerId;     // requester

        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(tradeId).l(myId).l(tradeId2).l(playerId);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Trade request]  From:") << hex
                << setw(8) << playerId 
                << _T(" id:") << setw(8) << tradeId
                << _T(" id2:") << setw(8) << tradeId2
                << endl;
        }
    };

    typedef FragmentSpec<TradeRequestImpl>   FragmentTradeRequest;


    class TradeAddGoodsReImpl
    {
    public:
        enum { ID = 0xFA5 };

        DWORD status;           // 0 - ok, 4A - не может быть продан
        DWORD tradeId1;
        DWORD sourceId;         // myid если добавл€ю €, partner id если добавл€ет сторона
        DWORD myId;
        DWORD tradeId2;
        DWORD itemId;           // zero if no items
        DWORD inventorySlot;    // если добавл€ет сторона, то слот его
        DWORD itemCount;
        DWORD stack;
        barray attrs;
        DWORD a1;               // additional item attrs
        DWORD a2;
        DWORD u1;
        DWORD u2;
        DWORD u3;
        DWORD money;            // zero if no money

        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(status).l(tradeId1).l(sourceId).l(myId).l(tradeId2).l(itemId)
                .l(inventorySlot).l(itemCount).l(stack)
                .arr(attrs, &Serializer<mode>::b, &Serializer<mode>::b)
                .l(a1).l(a2)
                .l(u1).l(u2).l(u3).l(money);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Trade Add Goods]  S:") << status
                << hex 
                << _T(" id1:") << setw(8) << tradeId1 
                << _T(" id2:") << setw(8) << tradeId2
                << _T(" from:") << setw(8) << sourceId
                << _T(" my:") << setw(8) << myId
                << endl
                << _T("    itemId:") << setw(8) << itemId
                << _T(" count:") << dec << itemCount
                << _T(" InvSlot:") << inventorySlot
                << _T(" Stack:") << stack
                << _T(" Money:") << money << hex
                << endl
                << _T("    u1:") << setw(8) << u1
                << _T(" u2:") << setw(8) << u2
                << _T(" u3:") << setw(8) << u3
                << _T(" a1:") << setw(8) << a1
                << _T(" a2:") << setw(8) << a2
                << _T(" A:") << attrs
                << endl;
        }
    };

    typedef FragmentSpec<TradeAddGoodsReImpl>   FragmentTradeAddGoodsRe;


    class TradeRemoveGoodsReImpl
    {
    public:
        enum { ID = 0xFA7 };

        DWORD status;           // 0 - ok, 4B - не может быть удален (количество больше имеющегос€)
        DWORD tradeId1;
        DWORD sourceId;         // myid если удал€ю €, partner id если удал€ет сторона
        DWORD myId;
        DWORD tradeId3;         // = 0
        DWORD itemId;           // zero if no items
        DWORD inventorySlot;    // если удал€ет сторона, то слот его (?)
        DWORD itemCount;
        DWORD stack;
        barray attrs;
        DWORD a1;               // additional item attrs
        DWORD a2;
        DWORD u1;
        DWORD u2;
        DWORD u3;
        DWORD money;            // zero if no money

        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(status).l(tradeId1).l(sourceId).l(myId).l(tradeId3).l(itemId)
                .l(inventorySlot).l(itemCount).l(stack)
                .arr(attrs, &Serializer<mode>::b, &Serializer<mode>::b)
                .l(a1).l(a2)
                .l(u1).l(u2).l(u3).l(money);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Trade Remove Goods]  S:") << status
                << hex 
                << _T(" id1:") << setw(8) << tradeId1 
                << _T(" id3:") << setw(8) << tradeId3
                << _T(" from:") << setw(8) << sourceId
                << _T(" my:") << setw(8) << myId
                << endl
                << _T("    itemId:") << setw(8) << itemId
                << _T(" count:") << dec << itemCount
                << _T(" InvSlot:") << inventorySlot
                << _T(" Stack:") << stack
                << _T(" Money:") << money << hex
                << endl
                << _T("    u1:") << setw(8) << u1
                << _T(" u2:") << setw(8) << u2
                << _T(" u3:") << setw(8) << u3
                << _T(" a1:") << setw(8) << a1
                << _T(" a2:") << setw(8) << a2
                << _T(" A:") << attrs
                << endl;
        }
    };

    typedef FragmentSpec<TradeRemoveGoodsReImpl>   FragmentTradeRemoveGoodsRe;


    class TradeSubmitReImpl
    {
    public:
        enum { ID = 0xFAB };

        DWORD status;       // 4— - myLock1 <-> 4D - pLock1 54 - unlock by error
        DWORD tradeId1;     // то же, что и в FA2
        DWORD playerId;     // source
        DWORD myId;
        DWORD unk;

        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(status).l(tradeId1).l(playerId).l(myId).l(unk);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Trade submit RE]  ") << hex
                << _T(" S:") << status
                << _T(" id1:") << setw(8) << tradeId1
                << _T(" player:") << setw(8) << playerId
                << _T(" My:") << setw(8) << myId
                << _T(" unk:") << setw(8) << unk
                << endl;
        }
    };

    typedef FragmentSpec<TradeSubmitReImpl>   FragmentTradeSubmitRe;


    class TradeConfirmReImpl
    {
    public:
        enum { ID = 0xFAD };

        DWORD status;       // 51 - myLock2 <-> 50 00 - pLock2 (end) 54 - unlock by error
        DWORD tradeId1;
        DWORD playerId;     // source
        DWORD myId;
        DWORD tradeId2;

        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(status).l(tradeId1).l(playerId).l(myId).l(tradeId2);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Trade confirm RE]  ") << hex
                << _T(" S:") << status
                << _T(" id1:") << setw(8) << tradeId1
                << _T(" id2:") << setw(8) << tradeId2
                << _T(" player:") << setw(8) << playerId
                << _T(" My:") << setw(8) << myId
                << endl;
        }
    };

    typedef FragmentSpec<TradeConfirmReImpl>   FragmentTradeConfirmRe;


    class TradeDiscardReImpl
    {
    public:
        enum { ID = 0xFAF };

        DWORD status;       // 0 - ok (1 - error ?)
        DWORD tradeId1;     // то же, что и в FA2
        DWORD tradeId2;     // то же, что и в FA2
        DWORD player1Id;    // discarded by?
        DWORD myId;

        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(status).l(tradeId1).l(player1Id).l(myId).l(tradeId2);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Trade discard RE]  ") << hex
                << _T(" P1:") << setw(8) << player1Id
                << _T(" My:") << setw(8) << myId
                << _T(" id1:") << setw(8) << tradeId1
                << _T(" id2:") << setw(8) << tradeId2
                << endl;
        }
    };

    typedef FragmentSpec<TradeDiscardReImpl>   FragmentTradeDiscardRe;


    class TradeEndImpl
    {
    public:
        enum { ID = 0xFB0 };

        byte status;        // 01 - ok, 00 - error
        DWORD tradeId1;     // то же, что и в FA2
        DWORD tradeId2;     // то же, что и в FA2
        DWORD myId;

        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(tradeId1).b(status).l(myId).l(tradeId2);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Trade end]  Status:") << (int)status
                << _T(" My:") << hex << setw(8) << myId
                << _T(" id1:") << setw(8) << tradeId1
                << _T(" id2:") << setw(8) << tradeId2
                << endl;
        }
    };

    typedef FragmentSpec<TradeEndImpl>   FragmentTradeEnd;


    class FactionChatImpl
    {
    public:
        enum { ID = 0x12C3 };

        WORD            u1;
        DWORD           senderId;
        std::wstring    text;
    #if PW_SERVER_VERSION >= 1422   // 1422 +-?
        barray          itemData;   // inventory type (l) slot (w)
    #endif
        DWORD           u2;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.w(u1).l(senderId).s(text)
            #if PW_SERVER_VERSION >= 1422   // 1422 +-?
                .arr(itemData, &Serializer<mode>::cui, &Serializer<mode>::b)
            #endif
                .l(u2);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Faction Chat]  charId:") << setw(8) << hex << senderId
                << _T(" T:") << setw(4) << u1
                << _T(" : ") << text
            #if PW_SERVER_VERSION >= 1422   // 1422 +-?
                << endl << _T("  ItemData: ") << itemData
            #endif
                << endl;
        }
    };

    typedef FragmentSpec<FactionChatImpl>   FragmentFactionChat;


    class GetFactionBaseInfoReImpl
    {
    public:
        enum { ID = 0x12CF };

        DWORD       id;
        DWORD       guildId;
        wstring     guildName;
        byte        level;
        WORD        people;
        DWORD       unk1;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(id).l(unk1).l(guildId).s(guildName).b(level).w(people);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Get Faction Base Info Re]  Id:") << setw(8) << hex << guildId << _T(" '") << guildName << _T("'  lvl:") << dec << (unsigned)level << _T(" People:") << (unsigned)people << _T("  myId:") << setw(8) << hex << id << _T(" Unk1:") << unk1 << endl;
        }
    };

    typedef FragmentSpec<GetFactionBaseInfoReImpl>   FragmentGetFactionBaseInfoRe;


    // ACReport is client

    class ACWhoamiImpl
    {
    public:
        enum { ID = 0x138A };

    protected:
        template<int mode> void format(Serializer<mode> & /*s*/)
        {}

        void print(tostream & stream) const
        {
            stream << _T(" [AC Whoami]") << endl;
        }
    };

    typedef FragmentSpec<ACWhoamiImpl>   FragmentACWhoami;


    class ACRemoteCodeImpl
    {
    public:
        enum { ID = 0x138B };

        enum
        {
            DataBlock       = 0,
            ImportFunction  = 1,
            ExecuteCode     = 2,
        };

        struct Block
        {
            WORD    id;
            WORD    type;           // type/entry offset
            WORD    streamSize;     // data.size + 6
            barray  data;

            template<int mode> void format(Serializer<mode> & s)
            {
                unsigned size = data.size() + 6;
                s.cui(size).wr(streamSize).wr(id).wr(type);
                // size should be filled
                s.arr(data, size - 6, &Serializer<mode>::b);
            }
        };

        DWORD               id;         // charId
        std::vector<Block>  blocks;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(id).arr(blocks, &Serializer<mode>::b);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [AC Remote code]") << endl;
        }
    };

    typedef FragmentSpec<ACRemoteCodeImpl>   FragmentACRemoteCode;


    class ACProtoStatImpl
    {
    public:
        enum { ID = 0x13A0 };

    protected:
        template<int mode> void format(Serializer<mode> & /*s*/)
        {}

        void print(tostream & stream) const
        {
            stream << _T(" [AC Proto stat]") << endl;
        }
    };

    typedef FragmentSpec<ACProtoStatImpl>   FragmentACProtoStat;


    class ACStatusAnnounceImpl
    {
    public:
        enum { ID = 0x13A2 };

    protected:
        template<int mode> void format(Serializer<mode> & /*s*/)
        {}

        void print(tostream & stream) const
        {
            stream << _T(" [AC Status Announce]") << endl;
        }
    };

    typedef FragmentSpec<ACStatusAnnounceImpl>   FragmentACStatusAnnounce;


    class ACReportCheaterImpl
    {
    public:
        enum { ID = 0x13A5 };

    protected:
        template<int mode> void format(Serializer<mode> & /*s*/)
        {}

        void print(tostream & stream) const
        {
            stream << _T(" [AC ReportCheater]") << endl;
        }
    };

    typedef FragmentSpec<ACReportCheaterImpl>   FragmentACReportCheater;


    class ACTriggerQuestionImpl
    {
    public:
        enum { ID = 0x13A6 };

    protected:
        template<int mode> void format(Serializer<mode> & /*s*/)
        {}

        void print(tostream & stream) const
        {
            stream << _T(" [AC Trigger Question]") << endl;
        }
    };

    typedef FragmentSpec<ACTriggerQuestionImpl>   FragmentACTriggerQuestion;


    class ACQuestionImpl
    {
    public:
        enum { ID = 0x13A7 };

    protected:
        template<int mode> void format(Serializer<mode> & /*s*/)
        {}

        void print(tostream & stream) const
        {
            stream << _T(" [AC Question]") << endl;
        }
    };

    typedef FragmentSpec<ACQuestionImpl>   FragmentACQuestion;


    class ACAnswerImpl
    {
    public:
        enum { ID = 0x13A8 };

    protected:
        template<int mode> void format(Serializer<mode> & /*s*/)
        {}

        void print(tostream & stream) const
        {
            stream << _T(" [AC Answer]") << endl;
        }
    };

    typedef FragmentSpec<ACAnswerImpl>   FragmentACAnswer;
}


#endif