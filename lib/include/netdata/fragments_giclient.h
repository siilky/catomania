#ifndef fragments_giclient_h
#define fragments_giclient_h

#include "netdata/fragments.h"
#include "netdata/content.h"
#include "netdata/serialize.h"


#define HANDLE_C_GI(cookies, connection, class, name) cookies.push_back(connection->bindClientHandler                  \
                                                    (std::function<void (const clientdata::FragmentGi##name *)>   \
                                                    (std::bind(&##class::on##name, this, _1) )))

namespace clientdata
{

    extern FragmentFactory fragmentGiFactory;

    // ------------------------------------------------------------------------------

    // Custom fragment used for sending
    class FragmentGiCustom : public FragmentGameinfo
    {
    public:
        WORD ID;

        FragmentGiCustom()
        {}
        FragmentGiCustom(barray & data, barray::const_iterator & curPos)
            : FragmentGameinfo(data, curPos)
        {}

        void setId(unsigned short id)
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

        virtual void print(tostream & /*stream*/)
        {
            assert(0);
        }

        virtual barray assemble()
        {
            return FragmentGameinfo::assemble(ID);
        }
    };

    // ------------------------------------------------------------------------------

//     class *Impl
//     {
//     public:
//         enum { ID = 0x0046 };
// 
//         DWORD   myId;
// 
//     protected:
//         template<int mode>
//         void format(Serializer<mode> & s)
//         {
//         }
// 
//         void print(tostream & stream) const const
//         {
//         }
//     };
//
//    typedef FragmentGiSpec<MoveLongImpl>   FragmentGiMoveLong;

    class MoveImpl
    {
    public:
        enum { ID = 0x0000 };

        Coord3D posFrom;
        Coord3D posTo;
        WORD    time;
        WORD    speed;
        byte    moveType;
        WORD    seqNum;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.fr(posFrom.x_).fr(posFrom.z_).fr(posFrom.y_)
             .fr(posTo.x_).fr(posTo.z_).fr(posTo.y_)
             .wr(time).wr(speed).b(moveType).wr(seqNum);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Move]  [") << posFrom << _T("] => [") << posTo 
                << _T("]  Type:") << hex << (unsigned)moveType << _T("  Seq:") << (unsigned)seqNum 
                << _T("  Time:") << (unsigned)time << _T(" Speed:") << (unsigned)speed << endl;
        }
    };

    typedef FragmentGiSpec<MoveImpl>   FragmentGiMove;


    class ExitImpl
    {
    public:
        enum { ID = 0x0001 };

        DWORD   type;   // enum PlayerLogoutType

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(type);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Exit]  type:") << hex << type << endl;
        }
    };

    typedef FragmentGiSpec<ExitImpl>   FragmentGiExit;


    class SelectTargetImpl
    {
        // reply: 22 34 ; 22 C2 (OOS)
    public:
        enum { ID = 0x0002 };

        DWORD   id;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Select Target]  Id:") << hex << setw(8) << id << endl;
        }
    };

    typedef FragmentGiSpec<SelectTargetImpl>   FragmentGiSelectTarget;


    class AttackImpl
    {
    public:
        enum { ID = 0x0003 };

        byte    unk;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.b(unk);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Attack]  Unk:") << hex << unk << endl;
        }
    };

    typedef FragmentGiSpec<AttackImpl>   FragmentGiAttack;


    class ResurrectRespawnImpl
    {
    public:
        enum { ID = 0x0004 };

        DWORD   unk;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(unk);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Resurrect to Respawn]  Unk:") << hex << unk << endl;
        }
    };

    typedef FragmentGiSpec<ResurrectRespawnImpl>   FragmentGiResurrectRespawn;


    class TakeItemImpl
    {
    public:
        enum { ID = 0x0006 };

        DWORD   id;
        DWORD   type;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id).lr(type);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Take Item]  Id:") << hex << setw(8) << id << _T("  Type:") << type<< endl;
        }
    };

    typedef FragmentGiSpec<TakeItemImpl>   FragmentGiTakeItem;


    class MoveStopImpl
    {
    public:
        enum { ID = 0x0007 };

        Coord3D pos;
        byte    orientation;
        byte    moveType;
        WORD    seqNum;
        WORD    speed;
        WORD    time;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.fr(pos.x_).fr(pos.z_).fr(pos.y_)
             .wr(speed).b(orientation).b(moveType)
             .wr(seqNum).wr(time);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Move Stop]  [") << pos << _T("]  Orient:") << (unsigned)orientation 
                << _T("  Type:") << hex << (unsigned)moveType << _T("  Seq:") << (unsigned)seqNum 
                << _T("  Time:") << (unsigned)time << _T(" Speed:") << (unsigned)speed << endl;
        }
    };

    typedef FragmentGiSpec<MoveStopImpl>   FragmentGiMoveStop;


    class CancelTargetImpl
    {
        // reply: 22 27
    public:
        enum { ID = 0x0008 };

    protected:
        template<int mode> void format(Serializer<mode> & /*s*/)
        {}

        void print(tostream & stream) const
        {
            stream << _T("  [Cancel Target]  ") << endl;
        }
    };

    typedef FragmentGiSpec<CancelTargetImpl>   FragmentGiCancelTarget;


    class GetSelfItemInfoImpl
    {
        // reply: 22 28
    public:
        enum { ID = 0x0009 };

        InventoryType   inventoryType;
        byte            slot;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            byte btype = static_cast<byte>(inventoryType);
            s.b(btype).b(slot);
            inventoryType = static_cast<InventoryType>(btype);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Get Self Item Info] ")
                << _T(" Type:") << (inventoryType == InventoryTypeBag ? _T("BAG") : (inventoryType == InventoryTypeEquip ? _T("EQUIP") : (inventoryType == InventoryTypeQuest ? _T("QUEST") : _T("UNKNOWN"))))
                << _T(" Slot:") << dec << slot 
                << endl;
        }
    };

    typedef FragmentGiSpec<GetSelfItemInfoImpl>   FragmentGiGetSelfItemInfo;


    class InventoryItemSwapImpl
    {
    public:
        enum { ID = 0x000C };

        byte    slot1;
        byte    slot2;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.b(slot1).b(slot2);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Inventory item swap]  From:") << dec << slot1 << _T(" To:") << slot2 << endl;
        }
    };

    typedef FragmentGiSpec<InventoryItemSwapImpl>   FragmentGiInventoryItemSwap;


    class InventoryItemMoveImpl
    {
    public:
        enum { ID = 0x000D };

        byte    srcSlot;
        byte    dstSlot;
        DWORD   count;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.b(srcSlot).b(dstSlot).lr(count);   //.w(count);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Inventory item move]  From:") << dec << srcSlot 
                << _T(" To:") << dstSlot
                << _T(" Count:") << count
                << endl;
        }
    };

    typedef FragmentGiSpec<InventoryItemMoveImpl>   FragmentGiInventoryItemMove;


    class InventoryToEquipSwapImpl
    {
    public:
        enum { ID = 0x0011 };

        byte    inventorySlot;
        byte    equipSlot;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.b(inventorySlot).b(equipSlot);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Inventory to Equip swap]  inventorySlot:") << dec << inventorySlot 
                   << _T(" equipSlot:") << equipSlot << endl;
        }
    };

    typedef FragmentGiSpec<InventoryToEquipSwapImpl>   FragmentGiInventoryToEquipSwap;


    class GetCharStatusImpl
    {
    public:
        enum { ID = 0x0015 };

    protected:
        template<int mode> void format(Serializer<mode> & /*s*/)
        {}

        void print(tostream & stream) const
        {
            stream << _T("  [Get Character Status]") << endl;
        }
    };

    typedef FragmentGiSpec<GetCharStatusImpl>   FragmentGiGetCharStatus;


    class GetEquipImpl
    {
    public:
        enum { ID = 0x0021 };

        std::vector<DWORD>  characters;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.arr(characters, &Serializer<mode>::wr, &Serializer<mode>::lr);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Get  Equip]  count:") << dec << characters.size() << endl 
                   << _T("  Characters");
            for (std::vector<DWORD>::const_iterator it = characters.begin();
                it != characters.end(); ++it)
            {
                stream << _T(" : ") << hex << setw(8) << *it;
            }
            stream << endl;
        }
    };

    typedef FragmentGiSpec<GetEquipImpl>   FragmentGiGetEquip;


    class OpenNpcImpl
    {
        // reply: 22 46 ; or no reply if too far
    public:
        enum { ID = 0x0023 };

        DWORD   id;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Open NPC]  id:") << setw(8) << hex << id << endl;
        }
    };

    typedef FragmentGiSpec<OpenNpcImpl>   FragmentGiOpenNpc;


    class NpcCommandImpl
    {
    public:
        enum { ID = 0x0024 };

        DWORD   cmd;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(cmd);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [NPC Command]  cmd:") << hex << cmd << endl;
        }
    };

    typedef FragmentGiSpec<NpcCommandImpl>   FragmentGiNpcCommand;


    class FragmentGiNpcInteract : public FragmentGameinfo
    {
    public:
        enum { ID = 0x0025 };

        FragmentGiNpcInteract(FragmentBase *fragment)   // takes ownership
            : nested(fragment)
        {}
        FragmentGiNpcInteract(const barray & data, barray::const_iterator & curPos)
            : FragmentGameinfo(data, curPos)
            , nested(0)
        {}
        ~FragmentGiNpcInteract();

        virtual bool parse();
        virtual barray assemble();
        virtual void print(tostream & /*stream*/) const;
    private:
        FragmentBase   *nested;
    };



    class GetFullInfoImpl
    {
    public:
        enum { ID = 0x0027 };

        BYTE unk1;
        BYTE unk2;
        BYTE unk3;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.b(unk1).b(unk2).b(unk3);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Get Full Info]  ") << hex << unk1 << _T(" ") << unk2 << _T(" ") << unk3 << endl;
        }
    };

    typedef FragmentGiSpec<GetFullInfoImpl>   FragmentGiGetFullInfo;


    class UseItemImpl
    {
    public:
        enum { ID = 0x0028 };

        InventoryType   type;
        BYTE            count;
        BYTE            slot;
        BYTE            unk2;
        DWORD           itemId;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            byte btype;
            btype = static_cast<byte>(type);
            s.b(btype).b(count).b(slot).b(unk2).lr(itemId);
            type = static_cast<InventoryType>(btype);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Use Item]  Inventory: ")
                   << (type == InventoryTypeBag ? _T("BAG") :
                    (type == InventoryTypeEquip ? _T("EQUIP") : (type == InventoryTypeQuest ? _T("QUEST") : _T("UNKNOWN"))))
                   << _T(" count:") << dec << int(count) << _T(" slot:") << int(slot)
                   << _T(" unk2:") << int(unk2) << _T(" itemId:") << dec << itemId << endl;
        }
    };

    typedef FragmentGiSpec<UseItemImpl>   FragmentGiUseItem;


    class AttackSkillImpl
    {
    public:
        enum { ID = 0x0029 };

        DWORD   skillID;
        WORD    unk;
        DWORD   targetID;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(skillID).wr(unk).lr(targetID);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Attack Skill]  Target:") << setw(8) << hex << targetID << _T(" Skill:") 
                << dec << skillID << _T(" unk:") << (unsigned)unk << endl;
        }
    };

    typedef FragmentGiSpec<AttackSkillImpl>   FragmentGiAttackSkill;


    class EmotionImpl
    {
    public:
        enum { ID = 0x0030 };

        WORD    emotionId;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.wr(emotionId);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Emotion]  id:") << dec << (unsigned)emotionId << endl;
        }
    };

    typedef FragmentGiSpec<EmotionImpl>   FragmentGiEmotion;


    class ActivateTaskImpl
    {
    public:
        enum { ID = 0x0031 };

        enum
        {
            CmdActivate = 1,
            CmdCancel   = 2,
        };

        DWORD   option;     // ?
        byte    command;
        WORD    taskId;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(option).b(command).wr(taskId);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Activate Task]  opt:") << dec << (unsigned) option
                << L" cmd:" << command << L" task:" << taskId << endl;
        }
    };

    typedef FragmentGiSpec<ActivateTaskImpl>   FragmentGiActivateTask;


    class GetCharAttrsImpl
    {
    public:
        enum { ID = 0x0043 };

        std::vector<DWORD>  characters;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.arr(characters, &Serializer<mode>::wr, &Serializer<mode>::lr);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Get Char Attrs]  count:") << dec << characters.size() << endl 
                   << _T("  Characters");
            for (std::vector<DWORD>::const_iterator it = characters.begin();
                it != characters.end(); ++it)
            {
                stream << _T(" : ") << hex << setw(8) << *it;
            }
            stream << endl;
        }
    };

    typedef FragmentGiSpec<GetCharAttrsImpl>   FragmentGiGetCharAttrs;


    class OpenMarketImpl
    {
    public:
        enum { ID = 0x004C };

        struct Item
        {
            DWORD   id;
            WORD    slot;
            WORD    unk;
            DWORD   count;
            DWORD   price;

            template<int mode> void format(Serializer<mode> & s)
            {
                s.lr(id).wr(slot).wr(unk).lr(count).lr(price);
            }
        };

        WCHAR               name[31];       // should be null-terminated string not longer than current market allows
        std::vector<Item>   items;

    protected:
        template<int mode>
        void format(Serializer<mode> & s);

        void format(SerializerIn & s)
        {
            WORD itemCount;
            s.wr(itemCount).m(name, sizeof_array(name), &SerializerIn::wr);
            s.arr(items, itemCount);
        }
        void format(SerializerLength & s)
        {
            WORD itemCount = (WORD)items.size();
            s.wr(itemCount).m(name, sizeof_array(name), &SerializerLength::wr);
            s.arr(items, itemCount);
        }
        void format(SerializerOut & s)
        {
            WORD itemCount = (WORD)items.size();
            s.wr(itemCount).m(name, sizeof_array(name), &SerializerOut::wr);
            s.arr(items, itemCount);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Open Market]  name:") << name
                   << _T(" items(") << dec << items.size() << _T(")") << endl;
            for (std::vector<Item>::const_iterator it = items.begin();
                it != items.end(); ++it)
            {
                stream << _T("  id:") << dec << it->id << _T(" count:") << it->count 
                       << _T(" price:") << it->price << _T(" unk:") << (unsigned)it->unk;
                if(it->slot != 0xFFFF)
                    stream << _T(" slot:") << (unsigned)it->slot << endl;
                else
                    stream << _T(" item for buy") << endl;
            }
        }
    };

    typedef FragmentGiSpec<OpenMarketImpl>   FragmentGiOpenMarket;


    class CancelMarketImpl
    {
    public:
        enum { ID = 0x004D };

    protected:
        template<int mode> void format(Serializer<mode> & /*s*/)
        {}

        void print(tostream & stream) const
        {
            stream << _T("  [Cancel Market]")<< endl;
        }
    };

    typedef FragmentGiSpec<CancelMarketImpl>   FragmentGiCancelMarket;


    class GetShopNameImpl
    {
    public:
        enum { ID = 0x004E };

        std::vector<DWORD>  shops;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.arr(shops, &Serializer<mode>::wr, &Serializer<mode>::lr);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Get Shop Name]  count:") << dec << shops.size() << endl 
                   << _T("  Characters");
            for (std::vector<DWORD>::const_iterator it = shops.begin();
                it != shops.end(); ++it)
            {
                stream << _T(" : ") << hex << setw(8) << *it;
            }
            stream << endl;
        }
    };

    typedef FragmentGiSpec<GetShopNameImpl>   FragmentGiGetShopName;


    class MarketSkillImpl
    {
        // replies: 22 BC, 22 19 error, or no answer in some conditions
    public:
        enum { ID = 0x0054 };

    protected:
        template<int mode> void format(Serializer<mode> & /*s*/)
        {}

        void print(tostream & stream) const
        {
            stream << _T("  [Market Skill]") << endl; 
        }
    };

    typedef FragmentGiSpec<MarketSkillImpl>   FragmentGiMarketSkill;


    class PlayerReviveImpl
    {
    public:
        enum { ID = 0x0057 };

        DWORD unk;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            #if PW_SERVER_VERSION >= 1500
                s.lr(unk);
            #else
                (void) s;
            #endif
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Player revive]  ")
            #if PW_SERVER_VERSION >= 1500
                << _T(" u:") << unk
            #endif
                << endl; 
        }
    };

    typedef FragmentGiSpec<PlayerReviveImpl>   FragmentGiPlayerRevive;


    class PurchaseShopItemImpl
    {
    public:
        enum { ID = 0x006A };

        DWORD   count;
        DWORD   itemId;
        DWORD   shopIndex;
        DWORD   u1;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(count).lr(itemId).lr(shopIndex).lr(u1);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Purchase item]  ")
                << _T(" ItemId:") << itemId
                << _T(" Count:") << count
                << _T(" shopIndex:") << shopIndex
                << _T(" u:") << u1
                << endl; 
        }
    };

    typedef FragmentGiSpec<PurchaseShopItemImpl>   FragmentGiPurchaseShopItem;


    class ExchangeNotesImpl
    {
    public:
        enum { ID = 0x0093 };

        byte    direction;      // 1 - notes to money, 0 - money to notes
        DWORD   count;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.b(direction).lr(count);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Exchange Notes] ")
                << _T(" D:") << (int) direction
                << _T(" count:") << count << endl; 
        }
    };

    typedef FragmentGiSpec<ExchangeNotesImpl>   FragmentGiExchangeNotes;


    class OnlineRegistrationImpl
    {
    public:
        enum { ID = 0x009C };

    protected:
        template<int mode> void format(Serializer<mode> & /*s*/)
        {}

        void print(tostream & stream) const
        {
            stream << _T("  [Online registration]") << endl; 
        }
    };

    typedef FragmentGiSpec<OnlineRegistrationImpl>   FragmentGiOnlineRegistration;

        
    class GetRegistrationInfoImpl
    {
    public:
        enum { ID = 0x009F };

    protected:
        template<int mode> void format(Serializer<mode> & /*s*/)
        {}

        void print(tostream & stream) const
        {
           stream << _T("  [Get online registration]") << endl; 
        }
    };

    typedef FragmentGiSpec<GetRegistrationInfoImpl>   FragmentGiGetRegistrationInfo;


} // namespace


#endif
