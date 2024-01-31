//
// Client (to server) packets declaration
//
#ifndef fragments_client_h
#define fragments_client_h

#include "netdata/fragments.h"
#include "netdata/fragments_giclient.h"
#include "netdata/serialize.h"

#define HANDLE_C(cookies, connection, class, name) cookies.push_back(connection->bindClientHandler                  \
                                                    (std::function<void (const clientdata::Fragment##name *)>   \
                                                    (std::bind(&##class::on##name, this, _1) )))


namespace clientdata
{
    extern FragmentFactory fragmentFactory;

    //------------------------------------------------------------------------------
    class FragmentArray : public FragmentSet
    {
    public:
        enum { ID = 0x0000 };

        FragmentArray(FragmentBase *fragment)
            : FragmentSet(fragment)
        {}
        FragmentArray(const barray & dataStream, barray::const_iterator & curPos)
            : FragmentSet(dataStream, curPos, &fragmentFactory)
        {}

        virtual barray assemble()override
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
        {
            id_ = ID;
            isOk_ = true;
            isParsed_ = true;
        }
        FragmentGameinfoSet(const barray & dataStream, barray::const_iterator & curPos)
            : FragmentSet(dataStream, curPos, &fragmentGiFactory)
        {}

        virtual barray assemble() override
        {
            return FragmentSet::assemble(ID);
        }

        virtual void encode(unsigned int x)
        {
            #if PW_SERVER_VERSION >= 1700
                for (auto f : fragmentArray_)
                {
                    f->setId(f->getId() ^ (uint16_t)x);
                }
            #else
                (void)x;
            #endif
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

    // Custom fragment used for sending

    class FragmentCustom : public Fragment
    {
    public:
        DWORD ID;

        FragmentCustom()
        {}
        FragmentCustom(barray & data, barray::const_iterator & curPos)
            : Fragment(data, curPos)
        {}

        void setId(unsigned int id)
        {
            ID = id;
        }

        void setPayload(barray & data)
        {
            payload_ = data;
        }

        virtual bool parse()
        {
            assert(0);
            return false;
        }

        virtual void print(tostream & /*stream*/) const
        {
            //assert(0);
        }

        virtual barray assemble()
        {
            return Fragment::assemble(ID);
        }
    };

    // ------------------------------------------------------------------------------

    class ChallengeResponseImpl
    {
    public:
        enum { ID = 0x0003 };

        std::string login;
        barray response;
        byte use_token;
#if defined(PW_SERVER_COMEBACK)
        std::string hwid;
        std::vector<std::string> hdd_ids;
#endif

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.c(login).arr(response, &Serializer<mode>::cui, &Serializer<mode>::b);

    #if PW_SERVER_VERSION >= 1440
            s.b(use_token);
    #endif

#if defined(PW_SERVER_COMEBACK)
            s.c(hwid).arr(hdd_ids, &Serializer<mode>::cui, &Serializer<mode>::c);
#else
    #if PW_SERVER_VERSION >= 1451
                pack.cb(4);
        #if defined(PW_SERVER_TOKEN_AUTH)
                s.cb(0x03);
                s.cb(0x00);
                s.cb(0x00);
                s.cb(0x00);
        #else //if defined(PW_SERVER_TOKEN_2_AUTH)
                s.cb(0xFF);
                s.cb(0xFF);
                s.cb(0xFF);
                s.cb(0xFF);
        #endif
    #endif
#endif
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Challenge Re]");
        }
    };

    typedef FragmentSpec<ChallengeResponseImpl>   FragmentChallengeResponse;


    class SelectRoleImpl
    {
    public:
        enum { ID = 0x0046 };

        DWORD   charId;
        byte    unk;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(charId)
            #if PW_SERVER_VERSION >= 1500
                .b(unk)
            #endif
            ;
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Select Role]  CharId:") << setw(8) << hex << charId << endl;
        }
    };

    typedef FragmentSpec<SelectRoleImpl>   FragmentSelectRole;


    class EnterWorldImpl
    {
    public:
        enum { ID = 0x0048 };

        DWORD   charId;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
             s.l(charId).l().l().l().l().l();
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Enter World]  charId:") << setw(8) << hex << charId << endl;
        }
    };

    typedef FragmentSpec<EnterWorldImpl>   FragmentEnterWorld;


    class ChatMessageImpl
    {
    public:
        enum { ID = 0x004F };

        enum
        {
            ChatTypePublic  = 0x00,
            ChatTypeGroup   = 0x02,
            ChatTypeTrade   = 0x07,
        };

        byte        type;
        byte        u0;
        DWORD       myId;
        DWORD       u1;         // put 0
        std::wstring    message;
#if PW_SERVER_VERSION >= 1420   // 1422 ?
        barray          itemData;   // inventory type (l) slot (w)
#endif

    protected:
        template<int mode>
        void format(Serializer<mode> & s)
        {
            s.b(type).b(u0).l(myId).l(u1).s(message);
#if PW_SERVER_VERSION >= 1420   // 1422 ?
            s.arr(itemData, &Serializer<mode>::cui, &Serializer<mode>::b);
#endif
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Chat Message]  Type:") << hex << type << " " << hex << u0
                << _T(" : ") << message
#if PW_SERVER_VERSION >= 1420   // 1422 ?
                << endl << _T("  ItemData: ") << itemData
#endif
                << endl;
        }
    };

    typedef FragmentSpec<ChatMessageImpl>   FragmentChatMessage;


    class RoleListImpl
    {
    public:
        enum { ID = 0x0052 };

        DWORD   accId;
        DWORD   unk;
        DWORD   slot;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(accId).l(unk).l(slot);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Role List]  accId:") << hex << setw(8) << accId
                << _T(" unk:") << unk 
                << _T(" slot:") << slot << endl;
        }
    };

    typedef FragmentSpec<RoleListImpl>   FragmentRoleList;


    class CreatRoleImpl
    {
    public:
        enum { ID = 0x0054 };

        struct EquipItem
        {
            DWORD       id;
            DWORD       pos;
            DWORD       count;
            DWORD       maxCount;
            barray      extraData;
            DWORD       proctype;
            DWORD       expireDate;
            DWORD       guid1;
            DWORD       guid2;
            DWORD       mask;

            template<int mode> void format(Serializer<mode> & s)
            {
                s.l(id).l(pos).l(count).l(maxCount)
                    .arr(extraData, &Serializer<mode>::cui, &Serializer<mode>::b)
                    .l(proctype).l(expireDate).l(guid1).l(guid2).l(mask);
            }
        };

        DWORD           accId;
        DWORD           unk1;
        DWORD           unk2;
        byte            gender;     
        WORD            prof;
        DWORD           level;
        DWORD           unk3;
        std::wstring    name;
        CharacterRecord charRecord;

        barray          customStatus;
        barray          charMode;
        DWORD           referrerRole;
        DWORD           cashAdd;
        barray          reincarnationData;
        barray          realmData;
        barray          referID;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(accId).l(unk1).l(unk2).b(gender).w(prof).l(level).l(unk3).s(name);
            charRecord.format(s);
            s.arr(customStatus, &Serializer<mode>::cui, &Serializer<mode>::b)
                .arr(charMode, &Serializer<mode>::cui, &Serializer<mode>::b)
                .l(referrerRole).l(cashAdd)
                .arr(reincarnationData, &Serializer<mode>::cui, &Serializer<mode>::b)
                .arr(realmData, &Serializer<mode>::cui, &Serializer<mode>::b)
                .arr(referID, &Serializer<mode>::cui, &Serializer<mode>::b);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Create Role]  accId:") << hex << setw(8) << accId
                << endl;
        }
    };

    typedef FragmentSpec<CreatRoleImpl>   FragmentCreatRole;


    class PlayerBaseInfoImpl
    {
    public:
        enum { ID = 0x005B };

        DWORD               myId;
        DWORD               unk;
        std::vector<DWORD>  characters;
        byte                unk2;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(myId).l(unk).arr(characters, &Serializer<mode>::b, &Serializer<mode>::l);
        #if PW_SERVER_VERSION >= 1640
            s.b(unk2);
        #endif
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Player Base Info]  MyID:") << setw(8) << hex << myId 
                << _T(" Unk:") << setw(8) << unk 
            #if PW_SERVER_VERSION >= 1640
                << _T(" u2:") << (unsigned)unk2
            #endif
                << _T(" Count:") << dec << characters.size() << endl
                << _T("  Characters");

            std::vector<DWORD>::const_iterator it = characters.begin();
            while(it != characters.end())
            {
                stream << _T(" : ") << setw(8) << hex << *it;
                it++;
            }
            stream << endl;
        }
    };

    typedef FragmentSpec<PlayerBaseInfoImpl>   FragmentPlayerBaseInfo;


    class PrivateChatImpl
    {
    public:
        enum { ID = 0x0060 };

        enum
        {
            ChatTypePrivate = 0x00,
            ChatTypeSystem = 0x04,
            ChatTypeDedicated = 0x07,
        };

        WORD            type;
        std::wstring    myName;
        DWORD           myId;
        std::wstring    toName;
        DWORD           toId;
        std::wstring    text;
    #if PW_SERVER_VERSION >= 1420   // 1422 ?
        barray          itemData;   // inventory type (l) slot (w)
        DWORD           u1;
    #endif
        
    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.w(type).s(myName).l(myId).s(toName).l(toId).s(text);
            #if PW_SERVER_VERSION >= 1420
                s.arr(itemData, &Serializer<mode>::cui, &Serializer<mode>::b)
                .l(u1);
            #endif
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Private chat]  Type:") << hex << type
                << _T(" From:") << setw(8) << myId << _T(" '") << myName
                << _T("'  To:") << toId << _T(" '") << toName
                << _T("' ::") << text << endl
            #if PW_SERVER_VERSION >= 1420
                << _T("  itemData: ") << itemData << endl
            #endif
            ;
        }
    };

    typedef FragmentSpec<PrivateChatImpl>   FragmentPrivateChat;
    

    class PlayerBriefInfoImpl
    {
    public:
        enum { ID = 0x006B };

        DWORD               myId;
        DWORD               u1;
        std::vector<DWORD>  characters;
        byte                u2;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(myId).l(u1).arr(characters, &Serializer<mode>::b, &Serializer<mode>::l).b(u2);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Player Brief Info]  MyID:") << setw(8) << hex << myId 
                << _T(" u1:") << setw(8) << u1
                << _T(" u1:") << setw(2) << u2
                << _T(" Count:") << dec << characters.size()
                << endl << _T("  Characters");

            std::vector<DWORD>::const_iterator it = characters.begin();
            while (it != characters.end())
            {
                stream << _T(" : ") << setw(8) << hex << *it;
                it++;
            }
            stream << endl;
        }
    };

    typedef FragmentSpec<PlayerBriefInfoImpl>   FragmentPlayerBriefInfo;


    class GetFriendsImpl
    {
    public:
        enum { ID = 0x00CE };

        DWORD           myId;
        DWORD           unk;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(myId).l(unk);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Get Friends]  Unk:") << hex << setw(8) << unk << endl;
        }
    };

    typedef FragmentSpec<GetFriendsImpl>   FragmentGetFriends;


    class SetLockTimeImpl
    {
    public:
        enum { ID = 0x030E };

        DWORD   myId;
        DWORD   unk1;       // = 0
        DWORD   seconds;
        DWORD   unk2;       // = 0

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(myId).l(unk1).l(seconds).l(unk2);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Set Lock Time]  ") << dec 
                << _T(" seconds:") << seconds
                << hex
                << setw(8) << _T(" unk1:") << unk1
                << setw(8) << _T(" unk2:") << unk2
                << endl;
        }
    };

    typedef FragmentSpec<SetLockTimeImpl>   FragmentSetLockTime;


    class BattleGetMapImpl
    {
    public:
        enum { ID = 0x352 };

        DWORD   myId;
        DWORD   unk;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(myId).l(unk);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Get Battle Map]  MyID:") << setw(8) << hex << myId << _T(" Unk:") << setw(8) << unk << endl;
        }
    };

    typedef FragmentSpec<BattleGetMapImpl>   FragmentBattleGetMap;


    class ComissionShopRequestImpl
    {
    public:
        // ответы: ничего (лол)

        enum { ID = 0x39C };

        DWORD   id;         // myID
        DWORD   shopId;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(id).l(shopId);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [ComissionShopRequest]  ID:") << setw(8) << hex << id
                << _T(" shopId:") << setw(8) << shopId
                << endl;
        }
    };

    typedef FragmentSpec<ComissionShopRequestImpl>   FragmentComissionShopRequest;


    class ComissionShopListRequestImpl
    {
    public:
        enum { ID = 0x39E };

        DWORD   id;
        DWORD   unk1;       // ts?
        DWORD   filter;     // фильтр по типам, 0xFF

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(id).l(unk1).l(filter);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [ComissionShopListRequest]  ID:") << setw(8) << hex << id 
                << _T(" unk1:") << setw(8) << unk1
                << _T(" unk2:") << setw(8) << filter
                << endl;
        }
    };

    typedef FragmentSpec<ComissionShopListRequestImpl>   FragmentComissionShopListRequest;


    class TradeResponseImpl
    {
    public:
        enum { ID = 0xFA3 };

        enum {
            TradeConfirm    = 0,
            TradeReject     = 0x45,
        };

        DWORD tradeId;
        DWORD status;       

        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(tradeId).l(status);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Trade response]  id:") << hex << setw(8) << tradeId << _T(" status:") << status
                << endl;
        }
    };

    typedef FragmentSpec<TradeResponseImpl>   FragmentTradeResponse;


    class TradeAddGoodsImpl
    {
    public:
        enum { ID = 0xFA4 };

        DWORD tradeId1;
        DWORD myId;
        DWORD tradeId3;
        DWORD itemId;           // zero if no items
        DWORD inventorySlot;
        DWORD itemCount;
        DWORD u1[6];
        byte  u2;
        DWORD money;            // zero if no money

        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(tradeId1).l(myId).l(tradeId3).l(itemId).l(inventorySlot).l(itemCount)
                .m(u1, sizeof_array(u1), &Serializer<mode>::l).b(u2).l(money);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Trade Add Goods]  id1:") << hex 
                << setw(8) << tradeId1 
                << _T(" my:") << setw(8) << myId
                << _T(" id3:") << setw(8) << tradeId3
                << _T(" itemId:") << setw(8) << itemId
                << _T(" count:") << dec << itemCount << hex
                << _T(" InvSlot:") << inventorySlot
                << _T(" Money:") << dec << money
                << endl;
        }
    };

    typedef FragmentSpec<TradeAddGoodsImpl>   FragmentTradeAddGoods;


    class TradeRemoveGoodsImpl
    {
    public:
        enum { ID = 0xFA6 };

        DWORD tradeId1;
        DWORD myId;
        DWORD tradeId3;         // фактически не влияет
        DWORD itemId;           // zero if no items
        DWORD inventorySlot;    
        DWORD itemCount;
        DWORD u1[6];
        byte  u2;
        DWORD money;            // zero if no money

        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(tradeId1).l(myId).l(tradeId3).l(itemId).l(inventorySlot).l(itemCount)
                .m(u1, sizeof_array(u1), &Serializer<mode>::l).b(u2).l(money);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Trade Remove Goods]  id1:") << hex 
                << setw(8) << tradeId1 
                << _T(" my:") << setw(8) << myId
                << _T(" id3:") << setw(8) << tradeId3
                << _T(" itemId:") << setw(8) << itemId
                << _T(" count:") << dec << itemCount << hex
                << _T(" InvSlot:") << inventorySlot
                << _T(" Money:") << dec << money
                << endl;
        }
    };

    typedef FragmentSpec<TradeRemoveGoodsImpl>   FragmentTradeRemoveGoods;


    class TradeSubmitImpl
    {
    public:
        enum { ID = 0xFAA };

        DWORD tradeId1;
        DWORD myId;
        DWORD tradeId3;       

        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(tradeId1).l(myId).l(tradeId3);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Trade submit]  id1:") << hex
                << setw(8) << tradeId1 
                << _T(" my:") << setw(8) << myId
                << _T(" id3:") << setw(8) << tradeId3
                << endl;
        }
    };

    typedef FragmentSpec<TradeSubmitImpl>   FragmentTradeSubmit;


    class TradeConfirmImpl
    {
    public:
        enum { ID = 0xFAC };

        DWORD tradeId1;
        DWORD myId;
        DWORD tradeId3;       

        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(tradeId1).l(myId).l(tradeId3);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Trade confirm]  id1:") << hex
                << setw(8) << tradeId1 
                << _T(" my:") << setw(8) << myId
                << _T(" id3:") << setw(8) << tradeId3
                << endl;
        }
    };

    typedef FragmentSpec<TradeConfirmImpl>   FragmentTradeConfirm;


    class TradeDiscardImpl
    {
    public:
        enum { ID = 0xFAE };

        DWORD tradeId1;
        DWORD myId;
        DWORD tradeId3;       

        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(tradeId1).l(myId).l(tradeId3);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Trade discard]  id1:") << hex
                << setw(8) << tradeId1 
                << _T(" my:") << setw(8) << myId
                << _T(" id3:") << setw(8) << tradeId3
                << endl;
        }
    };

    typedef FragmentSpec<TradeDiscardImpl>   FragmentTradeDiscard;


    class FactionChatImpl
    {
    public:
        enum { ID = 0x12C3 };

        WORD            u1;
        DWORD           myId;
        std::wstring    text;
    #if PW_SERVER_VERSION >= 1420   // 1422 ?
        barray          itemData;
    #endif
        DWORD           u2;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.w(u1).l(myId).s(text)
            #if PW_SERVER_VERSION >= 1420   // 1422 ?
                .arr(itemData, &Serializer<mode>::cui, &Serializer<mode>::b)
            #endif
                .l(u2);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Faction Chat]  charId:") << setw(8) << hex << myId
                << _T(" T:") << setw(4) << u1
                << _T(" : ") << text
            #if PW_SERVER_VERSION >= 1420   // 1422 ?
                << endl << _T("  ItemData: ") << itemData
            #endif
                << endl;
        }
    };

    typedef FragmentSpec<FactionChatImpl>   FragmentFactionChat;


    class GetFactionBaseInfoImpl
    {
    public:
        enum { ID = 0x12CE };

        DWORD               myId;
        DWORD               unk;
    #if PW_SERVER_VERSION >= 1640
        DWORD               unk2;
    #endif
        std::vector<DWORD>  guilds;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(myId).l(unk)
            #if PW_SERVER_VERSION >= 1640
                .l(unk2)
            #endif
                .arr(guilds, &Serializer<mode>::b, &Serializer<mode>::l);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [Get Faction Base Info]  MyID:") << setw(8) << hex << myId 
                << _T(" Unk:") << setw(8) << unk 
            #if PW_SERVER_VERSION >= 1640
                << _T(" Unk2:") << setw(8) << unk2
            #endif
                << _T(" Count:") << dec << guilds.size() << endl
                << _T("  Guilds");

            for (std::vector<DWORD>::const_iterator it = guilds.begin();
                it != guilds.end(); ++it)
            {
                stream << _T(" : ") << setw(8) << hex << *it;
            }
            stream << endl;
        }
    };

    typedef FragmentSpec<GetFactionBaseInfoImpl>   FragmentGetFactionBaseInfo;


    class ACReportImpl
    {
    public:
        enum { ID = 0x1389 };

        DWORD       myID;
        unsigned    unpackedSize;
        barray      data;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            // [id] [Total] [Expanded] [Packed] [type] [data]
            // /data includes type as first byte
            unsigned size = data.size();
            unsigned s1 = getPackedCUISize(size);
            unsigned s2 = getPackedCUISize(unpackedSize);
            unsigned sR2 = s1 + s2 + size;
            s.l(myID).cui(sR2).cui(unpackedSize).cui(size);
            s.arr(data, size, &Serializer<mode>::b);
        }

        void print(tostream & stream) const
        {
            stream << _T(" [AC Report]") << endl;
        }
    };

    typedef FragmentSpec<ACReportImpl>   FragmentACReport;


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
    
} // namespace

#endif