#ifndef fragments_giserver_h
#define fragments_giserver_h

#include <iomanip>

#include "types.h"
#include "netdata/fragments.h"
#include "netdata/content.h"
#include "netdata/serialize.h"


#define HANDLE_S_GI(cookies, connection, class, name)  cookies.push_back(connection->bindServerHandler                 \
                                                    (std::function<void (const serverdata::FragmentGi##name *)>   \
                                                    (std::bind(&##class::on##name, this, std::placeholders::_1) )))

namespace serverdata
{
    struct Character
    {
        DWORD       id;         // 0 3
        CoordEx     position;   // 4 7 8 11 12 15
        WORD        unk1;       // 16 17
        WORD        unk2;       // 18 19
                                // 20 angle
        BYTE        unk3;       // 21
        CharAttrs   attrs;      // 22 23 24 25  26 27 28 29

        template<int mode> void format(Serializer<mode> & /*s*/)    { assert(0); }

        template<> void format(SerializerIn & s)
        {
            s.lr(id).fr(position.x_).fr(position.z_).fr(position.y_)
                .wr(unk1).wr(unk2).b(position.angle).b(unk3);

            if ( ! attrs.read(s))
            {
                s.fail();
            }
        }
    };

    extern FragmentFactory fragmentGiFactory;

    // ------------------------------------------------------------------------------

//     class *Impl
//     {
//     public:
//         enum { ID = 0x00* };
// 
//         DWORD   myId;
// 
//     protected:
//         template<int mode> void format(Serializer<mode> & s)
//         {
//         }
// 
//         void print(tostream & stream) const const
//         {
//         }
//     };
// 
//    typedef FragmentGiSpec<*Impl>   FragmentGi*;

    class CharactersImpl
    {
    public:
        // this always notifies about players
        enum { ID = 0x0004 };

        std::vector<struct Character>  characters;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.arr(characters, &Serializer<mode>::wr);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Characters] (") << dec << characters.size() << _T(')') << endl;

            for ( std::vector<struct Character>::const_iterator it = characters.begin();
                it != characters.end(); ++it)
            {
                const struct Character & c = *it;

                stream << _T("    ID:") << hex << setw(8) << c.id << _T(" [") << c.position 
                    << _T("] Orient:") << (unsigned)c.position.angle
                    << _T("  Unk1:") << hex << (unsigned)c.unk1 << _T(" unk2:") << (unsigned)c.unk2
                    << _T(" unk3:") << (unsigned)c.unk3 << endl
                    << _T("     Status: ") << c.attrs << endl;
            }
        }
    };

    typedef FragmentGiSpec<CharactersImpl>   FragmentGiCharacters;


    class SelfInfoImpl
    {
    public:
        enum { ID = 0x0008 };

        DWORD   id;
        CoordEx position;
        DWORD   exp;
        DWORD   soul;
        DWORD   unk;        // как-то зависит от надетого шмота.
                            // если персоонаж голый - =FFFF.
        BYTE    unk2;
        CharAttrs  attrs;

    protected:
        template<int mode> void format(Serializer<mode> & /*s*/)    { assert(0); }

        template<> void format(SerializerIn & s)
        {
            s.lr(exp).lr(soul).lr(id).fr(position.x_).fr(position.z_).fr(position.y_).lr(unk);
            s.b(position.angle).b(unk2);

            if ( ! attrs.read(s))
            {
                s.fail();
            }
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Self Info]  ID:") << hex << setw(8) << id << _T(" [") << position
                   << _T("] Orient:") << (unsigned)position.angle << _T("  Exp:") << dec << exp 
                   << _T(" soul:") << soul << hex << _T("  unk:") << unk << _T(" unk2:") << unk2 << endl
                   << _T("               Status: ") << attrs << endl;
        }
    };

    typedef FragmentGiSpec<SelfInfoImpl>   FragmentGiSelfInfo;


    class ObjectsAppearImpl
    {
    public:
        // this always notifies about NPCs
        enum { ID = 0x0009 };

        struct objData
        {
            struct ExtStatus2
            {
                DWORD   v1;
                byte    v2;

                template<int mode> void format(Serializer<mode> & s)
                {
                    s.lr(v1).b(v2);
                }
            };

            DWORD   id;
            DWORD   type1;
            DWORD   type2;
            CoordEx position;
            WORD    unk2;

            DWORD   status;
            #if PW_SERVER_VERSION >= 1500
                DWORD   status2;
            #endif
            barray  bStatus;
            DWORD   aStatus;
            barray  extStatus;
            #if PW_SERVER_VERSION >= 1511
            std::vector<ExtStatus2>  extStatus2;
            #endif

            template<int mode> void format(Serializer<mode> & s)
            {
                s.lr(id).lr(type1)
                #if PW_SERVER_VERSION >= 1520
                    .lr(type2)
                #endif
                    .fr(position.x_).fr(position.z_).fr(position.y_)
                    ;
                #if PW_SERVER_VERSION < 1520
                    type2 = type1;
                #endif

                s.wr(unk2).b(position.angle);

                s.lr(status)
                #if PW_SERVER_VERSION >= 1500
                    .lr(status2)
                #endif
                    ;

                // что-то вроде статусной строки как в пакете с персоонажами
                /*  005B58E0 @142
                 *  00677270 @145
                 *  006CA480 @150
                 *  00713550 @151 *
                 *  007292C0 @1511 4BADB0
                    00779920 @1522 4D4F90
                    sub_760350 @ 1.5.6
                    do
                    {
                    flags = *(_DWORD *)(v8 + 27);
                    v10 = 35;
                    if ( flags & 0x40 )
                        v10 = 59;
                    if ( BYTE1(flags) & 0x10 )
                        v10 += 4;
                    if ( BYTE1(flags) & 0x20 )
                        v10 += *(_BYTE *)(v10 + v8) + 1;
                    if ( flags & 0x40000000 )
                        v10 += *(_DWORD *)(v10 + v8) + 4 * *(_DWORD *)(v10 + v8) + 4;
                    if ( BYTE1(flags) & 0x80 )
                        v10 += 4;
                    CECNPCMan::NPCEnter(v14, (unsigned int *)v8, 0);
                    v8 += v10;
                    ++v7;
                    }
                    while ( v7 < *v6 );
                */

                if ((status & 0x40) != 0)
                {
                    #if PW_SERVER_VERSION <= 1510
                        s.arr(bStatus, 16, &Serializer<mode>::b);
                    #elif PW_SERVER_VERSION < 1600
                        s.arr(bStatus, 24, &Serializer<mode>::b);
                    #elif PW_SERVER_VERSION < 1700
                        s.arr(bStatus, 28, &Serializer<mode>::b);
                    #else      
                        s.arr(bStatus, 36, &Serializer<mode>::b);
                    #endif
                }
                if ((status & 0x1000) != 0)
                {
                    s.lr(aStatus);
                }
                if ((status & 0x2000) != 0) 
                {
                    s.arr(extStatus, &Serializer<mode>::b, &Serializer<mode>::b);
                }
            #if PW_SERVER_VERSION >= 1511
                if ((status & 0x40000000) != 0)
                {
                    // some array (dword + byte) like in player status
                    s.arr(extStatus2, &Serializer<mode>::lr);
                }
                if ((status & 0x8000) != 0)
                {
                    s.skip(4);
                }
            #endif
            #if PW_SERVER_VERSION >= 1551
                if ((status & 0x40000) != 0)
                {
                    s.skip(4);
                }
            #endif
            #if PW_SERVER_VERSION >= 1560
                if ((status & 0x80000) != 0)
                {
                    s.skip(8);
                }
                if ((status & 0x100000) != 0)
                {
                    DWORD count = 0;
                    s.lr(count);
                    s.skip(4 * count).skip(8);
                }
                if ((status & 0x200000) != 0)
                {
                    s.skip(4);
                }
            #endif
            #if PW_SERVER_VERSION >= 1600
                if ((status2 & 0x08) != 0)
                {
                    s.skip(4);
                }
            #endif
            #if PW_SERVER_VERSION >= 1720
                if ((status2 & 0x200000) != 0)
                {
                    s.skip(20);
                }
            #endif
            }
        };

        std::vector<objData>  objects;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.arr(objects, &Serializer<mode>::wr);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Objects Appear] (") << dec << objects.size() << _T(')') << endl;

            for (std::vector<struct objData>::const_iterator it = objects.begin();
                it != objects.end(); ++it)
            {
                const struct objData &o = *it;

                stream << _T("    ID:") << hex << setw(8) << o.id << _T(" [") << o.position 
                       << _T("] Orient:") << (unsigned)o.position.angle << _T("  Type:") << o.type1 << _T(' ') << o.type2
                       << _T("  Unk2:") << (unsigned)o.unk2 << _T(" Unk3:") << (unsigned)o.status << endl
                       << _T("     Status:") << o.status;
                if ((o.status & 0x1000) != 0)
                {
                    stream << _T(" A/Status:") << o.aStatus;
                }
                stream << _T(" ExtStatus:") << o.extStatus << endl;
            }
        }
    };

    typedef FragmentGiSpec<ObjectsAppearImpl>   FragmentGiObjectsAppear;


    class ItemsImpl
    {
    public:
        enum { ID = 0x000A };

        struct itemData
        {
            DWORD       id;
            DWORD       type;
            Coord3D     pos;
            DWORD       unk1;
            byte        unk2;

            template<int mode> void format(Serializer<mode> & s)
            {
                s.lr(id).lr(type).fr(pos.x_).fr(pos.z_).fr(pos.y_).lr(unk1).b(unk2);
            }
        };

        std::vector<struct itemData>  items;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.arr(items, &Serializer<mode>::wr);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Items] (") << dec << items.size() << _T(')') << endl;

            for (std::vector<struct itemData>::const_iterator it = items.begin();
                it != items.end(); ++it)
            {
                const struct itemData &i = *it;

                stream << _T("    ID:") << hex << setw(8) << i.id << _T(" [") << i.pos << _T("] Type:") << i.type 
                       << _T("  Unk1:") << hex << (unsigned)i.unk1 << _T(" Unk2:") << (unsigned)i.unk2 << endl;
            }
        }
    };

    typedef FragmentGiSpec<ItemsImpl>   FragmentGiItems;


    class NpcEnterSliceImpl
    {
    public:
        enum { ID  = 0x000B };

        DWORD   id;
        DWORD   type;
        Coord3D pos;
        //WORD    1;
        //WORD    2;
        //byte    3;
        //WORD    3;
        //DWORD   4;
        //DWORD   5;
        //DWORD   6;
        //DWORD   7;


    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id).lr(type).fr(pos.x_).fr(pos.z_).fr(pos.y_); // TBD ?
        }

        void print(tostream & stream) const
        {
            stream << _T("  [NPC enter slice]  ID:") << hex << setw(8) << id << _T(" Type:") << type
                << _T(" [") << pos
                << _T("] (more)") << endl;
        }
    };

    typedef FragmentGiSpec<NpcEnterSliceImpl>   FragmentGiNpcEnterSlice;


    class RemoveObjectImplBase
    {
    public:

        DWORD   id;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Remove Object]  ID:") << hex << setw(8) << id << endl;
        }
    };

    class RemoveObjectImpl1 : public RemoveObjectImplBase
    {
    public:
        enum { ID = 0x000D };   // leave_slice
    };

    class RemoveObjectImpl2 : public RemoveObjectImplBase
    {
    public:
        enum { ID = 0x0013};    // player_leave_world
    };

    class RemoveObjectImpl3 : public RemoveObjectImplBase
    {
    public:
        enum { ID = 0x0015};    // object_disappear
    };

    class RemoveObjectImpl4 : public RemoveObjectImplBase
    {
    public:
        enum { ID = 0x00C2};    // object_is_invalid
    };

    typedef FragmentGiSpec<RemoveObjectImpl1>   FragmentGiRemoveObject1;
    typedef FragmentGiSpec<RemoveObjectImpl2>   FragmentGiRemoveObject2;
    typedef FragmentGiSpec<RemoveObjectImpl3>   FragmentGiRemoveObject3;
    typedef FragmentGiSpec<RemoveObjectImpl4>   FragmentGiRemoveObject4;


    class NotifyPosImpl
    {
    public:
        enum { ID = 0x000E };

        Coord3D     pos;
        DWORD       instanceId;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.fr(pos.x_).fr(pos.z_).fr(pos.y_).lr(instanceId);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Notify Pos]  [") << pos << _T("] instanceId:") << hex << instanceId << endl;
        }
    };

    typedef FragmentGiSpec<NotifyPosImpl>   FragmentGiNotifyPos;


    class ObjectMoveImpl
    {
    public:
        enum { ID = 0x000F };

        DWORD       id;
        Coord3D     pos;
        byte        moveType;
        WORD        unk1;
        WORD        unk2;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id).fr(pos.x_).fr(pos.z_).fr(pos.y_).wr(unk1).wr(unk2).b(moveType);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Object Move]  ID:") << hex << setw(8) << id << _T(" [") << pos 
                   << _T("]  Type:") << hex << (unsigned)moveType 
                   << " Unk1:" << (unsigned)unk1 << _T(" Unk2:") << (unsigned)unk2 << endl;
        }
    };

    typedef FragmentGiSpec<ObjectMoveImpl>   FragmentGiObjectMove;


    class NpcEnterWorldImpl
    {
    public:
        enum { ID  = 0x0010 };

        DWORD   id;
        DWORD   type;
        Coord3D pos;
        WORD    unk1;   // TBD angle?
        WORD    unk2;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id).lr(type).fr(pos.x_).fr(pos.z_).fr(pos.y_).wr(unk1).wr(unk2);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [NPC enter world]  ID:") << hex << setw(8) << id << _T(" Type:") << type
                << _T(" [") << pos 
                << _T("]  U1:") << (unsigned)unk1 << _T(" U2:") << (unsigned)unk2 
                << _T(" (more)") << endl;
        }
    };

    typedef FragmentGiSpec<NpcEnterWorldImpl>   FragmentGiNpcEnterWorld;


    class PlayerEnterImplBase
    {
    public:

        Character    character;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.t(character);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Player Enter]  ID:") << hex << setw(8) << character.id 
                   << _T(" [") << character.position
                   << _T("]  Unk1:") << hex << (unsigned)character.unk1 << _T(" Unk2:") << (unsigned)character.unk2
                   << _T(" unk3:") << character.unk3 
                   << _T(" Angle:") << character.position.angle << endl
                   << _T("     Status: ") << character.attrs << endl;
        }
    };

    class PlayerEnterImpl1 : public PlayerEnterImplBase
    {
    public:
        enum { ID = 0x000C};    // player_enter_slice
    };

    class PlayerEnterImpl2 : public PlayerEnterImplBase
    {
    public:
        enum { ID = 0x0011};    // player_enter_world
    };

    typedef FragmentGiSpec<PlayerEnterImpl1>   FragmentGiPlayerEnter1;
    typedef FragmentGiSpec<PlayerEnterImpl2>   FragmentGiPlayerEnter2;


    class MatterEnterWorldImpl
    {
    public:
        enum { ID = 0x0012 };

        DWORD   id;
        WORD    type;
        WORD    attr;       //цвет шмотки
        Coord3D pos;
        DWORD   unk1;
        byte    unk2;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id).w(type).w(attr).fr(pos.x_).fr(pos.z_).fr(pos.y_).lr(unk1).b(unk2);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Matter Enter World]  ID:") << hex << setw(8) << id << _T(" [") << pos 
                   << _T("]  Unk1:") << hex << (unsigned)unk1 
                   << _T(" Unk2:") << (unsigned)unk2 << endl;
        }
    };

    typedef FragmentGiSpec<MatterEnterWorldImpl>   FragmentGiMatterEnterWorld;


    class NpcDeadImpl
    {
    public:
        enum { ID = 0x0014 };

        DWORD   id;
        DWORD	charId;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id).lr(charId);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Npc Dead]  ID:") << hex << setw(8) << id << _T(" charId:") << charId << endl;
        }
    };

    typedef FragmentGiSpec<NpcDeadImpl>   FragmentGiNpcDead;


    class SelfAttackResultImpl
    {
    public:
        enum { ID = 0x0018 };

        DWORD       id;
        DWORD	    damage;
        WORD	    unk;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id).lr(damage).wr(unk);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Self Attack Resul]  ID:") << hex << setw(8) << id << _T(" Damage:") << damage 
                << _T("  Unk:") << (unsigned)unk << endl;
        }
    };

    typedef FragmentGiSpec<SelfAttackResultImpl>   FragmentGiSelfAttackResul;


    class ErrorMsgImpl
    {
    public:
        enum { ID = 0x0019 };

        DWORD   id;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Error]  ID:") << hex << setw(8) << id << endl;
        }
    };

    typedef FragmentGiSpec<ErrorMsgImpl>   FragmentGiErrorMsg;


    class BeAttackedImpl
    {
    public:
        enum { ID = 0x001A };

        DWORD       attackerId;
        DWORD	    damage;
        WORD	    effects;
        WORD	    unk2;
        WORD	    unk3;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(attackerId).lr(damage).wr(effects).wr(unk2).wr(unk3);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Attacked]  ID:") << hex << setw(8) << attackerId << _T(" Damage:") << dec << damage 
                << _T("  Eff:") << hex << effects << _T("  Unk2:") << (unsigned)unk2
                << _T("  Unk2:") << (unsigned)unk3 << endl;
        }
    };

    typedef FragmentGiSpec<BeAttackedImpl>   FragmentGiBeAttacked;


    class BeKilledImpl
    {
    public:
        enum { ID = 0x001C };

        DWORD	    attackerId;     // killer id
        Coord3D     pos;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(attackerId).fr(pos.x_).fr(pos.z_).fr(pos.y_);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [I'm killed]  By:") << hex << setw(8) << attackerId << _T(" [") << pos << _T(']') << endl;
        }
    };

    typedef FragmentGiSpec<BeKilledImpl>   FragmentGiBeKilled;


    class PlayerRevivalImpl
    {
        // персонаж воскрешен
    public:
        enum { ID = 0x001D };

        DWORD	    id;     // id
        WORD        stage;  // 0 - на респе, 1 дл€ первой фазы (иммун) и 2 дл€ второй фазы
        Coord3D     pos;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id).wr(stage).fr(pos.x_).fr(pos.z_).fr(pos.y_);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Player revived]  Id:") << hex << setw(8) << id << _T(" [") << pos 
                   << _T("]  stage: ") << (unsigned)stage << endl;
        }
    };

    typedef FragmentGiSpec<PlayerRevivalImpl>   FragmentGiPlayerRevival;


    class PickupMoneyImpl
    {
    public:
        enum { ID = 0x001E };

        DWORD	    money;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(money);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Pickup Money]  Money:") << dec << money << endl;
        }
    };

    typedef FragmentGiSpec<PickupMoneyImpl>   FragmentGiPickupMoney;


    class PlayerPickupItemImpl
    {
    public:
        enum { ID = 0x001F };

        DWORD           itemId;
        DWORD           timeLimit;
        #if PW_SERVER_VERSION > 1442
            DWORD       amount;
            DWORD       totalInSlot;
        #else
            WORD        amount;
            WORD        totalInSlot;
        #endif
        InventoryType   type;
        byte            slot;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            byte btype = static_cast<byte>(type);
            #if PW_SERVER_VERSION > 1442
                s.lr(itemId).lr(timeLimit).lr(amount).lr(totalInSlot).b(btype).b(slot);
            #else
                s.lr(itemId).lr(timeLimit).wr(amount).wr(totalInSlot).b(btype).b(slot);
            #endif
            type = static_cast<InventoryType>(btype);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Pickup Item]  Inventory: ") 
                << (type == InventoryTypeBag ? _T("BAG") : (type == InventoryTypeEquip ? _T("EQUIP") : (type == InventoryTypeQuest ? _T("QUEST") : _T("UNKNOWN"))))
                << _T(" Slot:") << dec << slot 
                << _T(" ItemId:") << itemId 
                << _T(" Time:") << hex << (unsigned)timeLimit 
                << _T(" Amount:") << int(amount) << _T(" TotalInSlot:") << int(totalInSlot)
                << endl;
        }
    };

    typedef FragmentGiSpec<PlayerPickupItemImpl>   FragmentGiPlayerPickupItem;


    class PlayerInfoImpl
    {
    public:
        enum { ID = 0x0020 };

        DWORD   id;
        WORD	lvl;
        DWORD   hp;
        DWORD   maxHp;
        DWORD	mp;
        DWORD	maxMp;
        WORD	unk;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id).wr(lvl).wr(unk).lr(hp).lr(maxHp).lr(mp).lr(maxMp);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Player Info]  ID:") << hex << setw(8) << id << dec << _T(" Lvl:") << (unsigned)lvl
                   << _T("  HP:") << hp << _T('/') << maxHp 
                   << _T("  MP:") << mp << _T('/') << maxMp 
                   << _T(" Unk:") << hex << (unsigned)unk << endl;
        }
    };

    typedef FragmentGiSpec<PlayerInfoImpl>   FragmentGiPlayerInfo;


    class NpcInfoImpl
    {
    public:
        enum { ID = 0x0021 };

        DWORD   id;
        DWORD   hp;
        DWORD   maxHp;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id).lr(hp).lr(maxHp);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [NPC Info]  ID:") << hex << setw(8) << id << dec << _T("  HP:") << hp << _T('/') << maxHp <<endl;
        }
    };

    typedef FragmentGiSpec<NpcInfoImpl>   FragmentGiNpcInfo;


    class OOSListImpl
    {
    public:
        enum { ID = 0x0022 };

        std::vector<DWORD>  objIds;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.arr(objIds, &Serializer<mode>::lr, &Serializer<mode>::lr);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [OOS List] (") << dec << objIds.size() << _T(")  ID(s):") << hex;

            for ( std::vector<DWORD>::const_iterator it = objIds.begin();
                 it != objIds.end(); ++it)
            {
                stream << setw(8) << *it << _T(' ');
            }
            stream << endl;
        }
    };

    typedef FragmentGiSpec<OOSListImpl>   FragmentGiOOSList;


    class ObjectStopMoveImpl
    {
    public:
        enum { ID = 0x0023 };

        DWORD   id;
        Coord3D pos;
        WORD    unk1;
        WORD    unk2;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id).fr(pos.x_).fr(pos.z_).fr(pos.y_).wr(unk1).wr(unk2);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [StopMove]  ID:") << hex << setw(8) << id << _T(" [") << pos 
                   << _T("]  Unk1:") << hex << (unsigned)unk1 
                   << _T(" Unk2:") << (unsigned)unk2 << endl;
        }
    };

    typedef FragmentGiSpec<ObjectStopMoveImpl>   FragmentGiObjectStopMove;


    class ReceiveExpImpl
    {
    public:
        enum { ID = 0x0024 };

        DWORD   exp;
        DWORD   soul;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(exp).lr(soul);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Receive Exp]  Exp:") << dec << exp << _T(" Soul:") << soul << endl;
        }
    };

    typedef FragmentGiSpec<ReceiveExpImpl>   FragmentGiReceiveExp;


    class LevelUpImpl
    {
    public:
        enum { ID = 0x0025 };

        DWORD   id;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Level Up]  ID:") << hex<< setw(8) << id << endl;
        }
    };

    typedef FragmentGiSpec<LevelUpImpl>   FragmentGiLevelUp;


    class MyStatusImpl  // self_info_00
    {
    public:
        enum { ID = 0x0026 };

        WORD    lvl;
        DWORD   hp;
        DWORD   maxHp;
        DWORD	mp;
        DWORD	maxMp;
        DWORD   exp;
        DWORD   soul;
        DWORD   fury;
        DWORD   maxfury;
        byte    inBattle;
        byte    unk;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.wr(lvl).b(inBattle).b(unk).lr(hp).lr(maxHp).lr(mp).lr(maxMp);
            s.lr(exp).lr(soul).lr(fury).lr(maxfury);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [My Status]  Lvl:") << dec << (unsigned)lvl << _T("  HP:") << hp << _T('/') << maxHp << _T("  MP:") << mp 
                   << _T('/') << maxMp << _T("  Exp:") << exp << _T(" Soul:") << soul << _T(" Fury:") << fury << _T('/') << maxfury 
                   << _T(" inBattle:") << (unsigned)inBattle << _T(" unk:") << (unsigned)unk << endl;
        }
    };

    typedef FragmentGiSpec<MyStatusImpl>   FragmentGiMyStatus;


    class UnselectImpl
    {
    public:
        enum { ID = 0x0027 };

    protected:
        template<int mode> void format(Serializer<mode> & /*s*/)
        {}

        void print(tostream & stream) const
        {
            stream << _T("  [Unselect]") << endl;
        }
    };

    typedef FragmentGiSpec<UnselectImpl>   FragmentGiUnselect;


    class SelfItemInfoImpl
    {
    public:
        enum { ID = 0x0028 };

        InventoryType   inventoryType;
        byte            slot;
        DWORD           itemId;
        DWORD           timeLimit;   
        DWORD           unk2;
        DWORD           count;
        WORD            unk3;
        barray          attrs;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            byte btype = static_cast<byte>(inventoryType);
            s.b(btype).b(slot).lr(itemId).lr(timeLimit).lr(unk2).lr(count).wr(unk3)
                .arr(attrs, &Serializer<mode>::wr, &Serializer<mode>::b);
            inventoryType = static_cast<InventoryType>(btype);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Item info] ")
                << _T(" Type:") << (inventoryType == InventoryTypeBag ? _T("BAG") : (inventoryType == InventoryTypeEquip ? _T("EQUIP") : (inventoryType == InventoryTypeQuest ? _T("QUEST") : _T("UNKNOWN"))))
                << _T(" Slot:") << dec << slot 
                << _T(" Id:") << itemId 
                << _T(" Count:") << count
                << _T(" Time:") << hex << timeLimit
                << _T("  ") << unk2 << _T(" : ") << (unsigned)unk3 
                << _T(" A: ") << attrs 
                << endl;
        }
    };

    typedef FragmentGiSpec<SelfItemInfoImpl>   FragmentGiSelfItemInfo;


    class SelfItemEmptyInfoImpl
    {
    public:
        enum { ID = 0x0029 };

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
            stream << _T("  [Item info] ")
                << _T(" Type:") << (inventoryType == InventoryTypeBag ? _T("BAG") : (inventoryType == InventoryTypeEquip ? _T("EQUIP") : (inventoryType == InventoryTypeQuest ? _T("QUEST") : _T("UNKNOWN"))))
                << _T(" Slot:") << dec << slot 
                << endl;
        }
    };

    typedef FragmentGiSpec<SelfItemEmptyInfoImpl>   FragmentGiSelfItemEmptyInfo;


    class SelfInventoryDetailImpl
    {
    public:
        enum { ID = 0x002B };

        struct Item
        {
            DWORD   slot;           // слоты считаютс€ с нул€
            DWORD   id;             // pItem
            DWORD   timeLimit;      // +4
            DWORD   opts;           // +8
            DWORD   count;          // +12
            WORD    unk3;           // +14
            barray  attrs;          // +16
            DWORD   eattr1;
            DWORD   eattr2;

            template<int mode> void format(Serializer<mode> & s)
            {
                s.lr(slot).lr(id).lr(timeLimit).lr(opts).lr(count).wr(unk3);
                #if PW_SERVER_VERSION >= 1640
                if (opts & 0x400000)
                {
                    s.lr(eattr1).lr(eattr2);
                }
                #endif
                s.arr(attrs, &Serializer<mode>::wr, &Serializer<mode>::b);
            }
        };

        InventoryType       type;
        byte                cells;
        DWORD               totalItemsSize;
        std::vector<Item>   items;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            byte btype = static_cast<byte>(type);
            s.b(btype).b(cells).lr(totalItemsSize).arr(items, &Serializer<mode>::lr);
            type = static_cast<InventoryType>(btype);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Inventory detail]  Type: ") 
                   << (type == InventoryTypeBag ? _T("BAG") : (type == InventoryTypeEquip ? _T("EQUIP") : (type == InventoryTypeQuest ? _T("QUEST") : _T("UNKNOWN"))))
                   << _T(" Slots:") << dec << cells << _T(" Items:") << items.size()
                   << _T(" sz:") << hex << totalItemsSize << endl;
            
            for (std::vector<struct Item>::const_iterator it = items.begin();
                it != items.end(); ++it)
            {
                stream << _T("    Slot:") << dec << it->slot
                    << _T(" Id:") << it->id
                    << _T(" Count:") << it->count
                    << _T(" Time:") << hex << it->timeLimit
                    << _T("  ") << it->opts << _T(" : ") << (unsigned)it->unk3;
                    #if PW_SERVER_VERSION >= 1640
                    if (it->opts & 0x400000)
                    {
                        stream << it->eattr1 << it->eattr2 << _T(" : ");
                    }
                    #endif
                    stream << _T(" A: ") << it->attrs << endl;
            }
        }
    };

    typedef FragmentGiSpec<SelfInventoryDetailImpl>   FragmentGiSelfInventoryDetail;


    class ExchangeInventoryItemImpl
    {
    public:
        enum { ID = 0x002C };

        byte    from;
        byte    to;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.b(from).b(to);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Exchange Inventory Item]  From:") << dec << from << _T(" To:") << to << endl;
        }
    };

    typedef FragmentGiSpec<ExchangeInventoryItemImpl>   FragmentGiExchangeInventoryItem;


    class MoveInventoryItemImpl
    {
    public:
        enum { ID = 0x002D };

        byte            from;
        byte            to;
        WORD            amount;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.b(from).b(to).wr(amount);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Move Inventory Item]  From:") << dec << from <<_T(" To:") 
                << to << _T(" Amount:") << (int)amount << endl;
        }
    };

    typedef FragmentGiSpec<MoveInventoryItemImpl>   FragmentGiMoveInventoryItem;


    class PlayerDropItemImpl
    {
    public:
        enum { ID = 0x002E };

        InventoryType   type;   
        byte            slot;
        #if PW_SERVER_VERSION > 1442
            DWORD       amount;
        #else
            WORD        amount;
        #endif
        DWORD           itemId;
        byte            unk;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            byte btype = static_cast<byte>(type);
            #if PW_SERVER_VERSION > 1442
                s.b(btype).b(slot).lr(amount).lr(itemId).b(unk);
            #else
                s.b(btype).b(slot).wr(amount).lr(itemId).b(unk);
            #endif
            type = static_cast<InventoryType>(btype);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Player Drop Item]  Inventory:") 
                   << (type == InventoryTypeBag ? _T("BAG") : (type == InventoryTypeEquip ? _T("EQUIP") : (type == InventoryTypeQuest ? _T("QUEST") : _T("UNKNOWN"))))
                   << _T(" slot:") << dec << slot << _T(" amount:") << int(amount) << _T(" itemId:") << itemId 
                   << _T(" u2:") << unk << endl;
        }
    };

    typedef FragmentGiSpec<PlayerDropItemImpl>   FragmentGiPlayerDropItem;


    class ExchangeEquipmentItemImpl
    {
    public:
        enum { ID = 0x002F };

        byte    from;
        byte    to;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.b(from).b(to);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Exchange Equipment Item]  From:") << dec << from << _T(" To:") << to << endl;
        }
    };

    typedef FragmentGiSpec<ExchangeEquipmentItemImpl>   FragmentGiExchangeEquipmentItem;


    class EquipItemImpl
    {
    public:
        enum { ID = 0x0030 };

        byte        bagSlot;
        byte        equipSlot;
        #if PW_SERVER_VERSION > 1442
            DWORD   bagAmount;
            DWORD   equipAmount;
        #else
            WORD    bagAmount;
            WORD    equipAmount;
        #endif

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            #if PW_SERVER_VERSION > 1442
                s.b(bagSlot).b(equipSlot).lr(bagAmount).lr(equipAmount);
            #else
                s.b(bagSlot).b(equipSlot).wr(bagAmount).wr(equipAmount);
            #endif
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Player Equip Item]  Bag slot:") << dec << bagSlot << _T(" Equip slot:") << equipSlot
                   << _T(" Bag items:") << (int)bagAmount << _T(" Equip items:") << (int)equipAmount << endl;
        }
    };

    typedef FragmentGiSpec<EquipItemImpl>   FragmentGiEquipItem;


    class MoveEquipmentItemImpl
    {
    public:
        enum { ID = 0x0031 };

        byte        bagSlot;
        byte        equipSlot;
        WORD        amount;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.b(bagSlot).b(equipSlot).wr(amount);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Move Inventory Item]  BagSlot:") << dec << bagSlot <<_T(" EquipSlot:") << equipSlot 
                << _T(" Amount:") << (unsigned)amount << endl;
        }
    };

    typedef FragmentGiSpec<MoveEquipmentItemImpl>   FragmentGiMoveEquipmentItem;


    class SelfGetPropertyImpl
    {
    public:
        enum { ID = 0x0032 };

        DWORD   statusPoints;

        DWORD   vit_points;
        DWORD   int_points;
        DWORD   str_points;
        DWORD   agi_points;

        DWORD   maxHp;
        DWORD   maxMp;
        DWORD   maxfury;

        DWORD   hit;
        DWORD   flee;
        float   crit;
        float   walkSpeed;
        float   flySpeed;

        DWORD   minFAtk;
        DWORD   maxFAtk;
        DWORD   minMAtk;
        DWORD   maxMAtk;
        DWORD   aspd;

        DWORD   defPhys;
        DWORD   defMetal;
        DWORD   defWood;
        DWORD   defWater;
        DWORD   defFire;
        DWORD   defEarth;

        DWORD   unk1;
        DWORD   unk2;
        DWORD   unk3;
        DWORD   unk4;
        float   unk5;
        float   unk6;

        barray  unk7;

    #if PW_SERVER_VERSION >= 1420
        DWORD   unk8;
        DWORD   unk9;
    #endif
    #if PW_SERVER_VERSION >= 1422
        DWORD   unk10;
        DWORD   unk11;
    #endif
    #if PW_SERVER_VERSION >= 1450
        DWORD   unk12;
        DWORD   unk13;
    #endif
    #if PW_SERVER_VERSION >= 1510
        DWORD   unk14;
    #endif
    #if PW_SERVER_VERSION >= 1530
        DWORD   unk15;
        DWORD   unk16;
    #endif

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(statusPoints).lr(unk1).lr(unk2)
        #if PW_SERVER_VERSION >= 1420
            .lr(unk8).lr(unk9)
        #endif
        #if PW_SERVER_VERSION >= 1422
            .lr(unk10).lr(unk11)
        #endif
        #if PW_SERVER_VERSION >= 1450
            .lr(unk12).lr(unk13)
        #endif
        #if PW_SERVER_VERSION >= 1510
            .lr(unk14)
        #endif
        #if PW_SERVER_VERSION >= 1530
            .lr(unk15).lr(unk16)
        #endif
            .lr(vit_points).lr(int_points).lr(str_points).lr(agi_points)
            .lr(maxHp).lr(maxMp).lr(unk3).lr(unk4)
            .fr(crit).fr(walkSpeed).fr(unk5).fr(flySpeed)
            .lr(hit).lr(minFAtk).lr(maxFAtk).lr(aspd)
            .fr(unk6).arr(unk7, 40, &Serializer<mode>::b)
            .lr(minMAtk).lr(maxMAtk)
            .lr(defMetal).lr(defWood).lr(defWater).lr(defFire).lr(defEarth).lr(defPhys)
            .lr(flee).lr(maxfury)
            ;
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Self Get Property]  Int:") << dec << int_points << _T(" Vit:") << vit_points << _T(" Str:") << str_points << _T(" Agi:") << agi_points << _T("  Atk:") << minFAtk << _T('-') << maxFAtk << _T(" Matk:") << minMAtk << _T('-') << maxMAtk << _T("  Aspd:") << aspd << endl
                   << _T("                       DEF Phys:") << defPhys << _T(" Metal:") << defMetal << _T(" Wood:") << defWood << _T(" Water:") << defWater << _T(" Fire:") << defFire << _T(" Earth:") << defEarth << endl
                   << _T("                       Hit:") << hit << _T(" Flee:") << flee << _T(" Crit:") << crit << _T(" WalkSpeed:") << walkSpeed << _T(" FlySpeed:") << flySpeed << _T("  Hp:") << maxHp << _T(" Mp:") << maxMp << _T(" Fury:") << maxfury << endl
                   << _T("                       StatusPoints:") << statusPoints << _T(" Unk1:") << unk1 << _T(" Unk2:") << unk2 << _T(" Unk3:") << unk3 << _T(" Unk4:") << unk4 << _T(" Unk5:") << unk5 << _T(" Unk6:") << unk6 << _T(" Unk7:") << unk7 << endl
            #if PW_SERVER_VERSION >= 1420
                   << _T("                       Unk8:") << unk8 << _T(" Unk9:") << unk9
            #endif
            #if PW_SERVER_VERSION >= 1422
                   << _T(" Unk10:") << unk10 << _T(" Unk11:") << unk11
            #endif
            #if PW_SERVER_VERSION >= 1450
                   << _T(" Unk12:") << unk12 << _T(" Unk13:") << unk13
            #endif
            #if PW_SERVER_VERSION >= 1510
                   << _T(" Unk14:") << unk14
            #endif
            #if PW_SERVER_VERSION >= 1530
                   << _T(" Unk15:") << unk15 << _T(" Unk16:") << unk16
            #endif
                   << endl;
        }
    };

    typedef FragmentGiSpec<SelfGetPropertyImpl>   FragmentGiSelfGetProperty;


    class PlayerSelectTargetImpl
    {
    public:
        enum { ID = 0x0034 };

        DWORD   id;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Player Select Target]  ID:") << hex << setw(8) << id <<endl;
        }
    };

    typedef FragmentGiSpec<PlayerSelectTargetImpl>   FragmentGiPlayerSelectTarget;


    class SendEquipmentInfoImpl
    {
    public:
        enum { ID = 0x0042 };

        struct Item
        {
            WORD    id;
            WORD    type;

            template<int mode> void format(Serializer<mode> & s)
            {
                s.wr(id).wr(type);
            }
        };

        WORD    unk;
        DWORD   characterID;
        DWORD   status;

        std::vector<struct Item>  items;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.wr(unk).lr(characterID).lr(status);
            if (status != 0)
            {
                s.arr(items, bitCounter(status));
            }
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Equipment Info]  ID:") << hex << setw(8) << characterID << _T(" Unk:") << (unsigned)unk
                   << _T(" Status:") << setw(8) << status <<endl
                   << _T("  Items(") << dec << items.size() << _T(")");
            for (std::vector<struct Item>::const_iterator it = items.begin();
                it != items.end(); ++it)
            {
                stream << _T(" : ") << dec << (unsigned)it->id << _T("(") << (unsigned)it->type << _T(")");
            }
            stream << endl;
        }
    };

    typedef FragmentGiSpec<SendEquipmentInfoImpl>   FragmentGiSendEquipmentInfo;


    class NpcGreetingImpl
    {
    public:
        enum { ID = 0x0046 };

        DWORD   id;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [NPC Greeting]  ID:") << hex << setw(8) << id << endl;
        }
    };

    typedef FragmentGiSpec<NpcGreetingImpl>   FragmentGiNpcGreeting;


    class PlayerPurchaseItemImpl
    {
    public:
        enum { ID = 0x0048 };

        struct Item
        {
            DWORD   id;
            #if PW_SERVER_VERSION >= 1450
                DWORD   count;
            #else
                WORD    count;
            #endif
            WORD    slot;
            byte    tradeSlot;      // tradeSlot is a slot in market list of <22><A9>. 
                                    // Slots are not reordered after sell/buy.
            DWORD   unk;

            template<int mode> void format(Serializer<mode> & s)
            {
                #if PW_SERVER_VERSION >= 1450
                    s.lr(id).lr(unk).lr(count).wr(slot).b(tradeSlot);
                #else
                    s.lr(id).lr(unk).wr(count).wr(slot).b(tradeSlot);
                #endif
            }
        };

        DWORD   moneyLost;
        DWORD   unk;
        BYTE    unk2;

        std::vector<struct Item> items;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(moneyLost).lr(unk).b(unk2).arr(items, &Serializer<mode>::wr);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Purchase Item]  Money lost:") << dec << moneyLost 
                   << _T(" items(") << items.size() << _T(")") << endl;
            for (std::vector<struct Item>::const_iterator it = items.begin();
                it != items.end(); ++it)
            {
                stream << _T("  id:") << it->id << _T(" count:") << dec << (int)it->count
                       << _T(" slot:") << int(it->slot)
                       << _T(" unk:") << hex << setw(8)  << it->unk 
                       << _T(" Trade Slot:") << dec << it->tradeSlot << endl;
            }
        }
    };

    typedef FragmentGiSpec<PlayerPurchaseItemImpl>   FragmentGiPlayerPurchaseItem;


    class ItemToMoneyImpl
    {
    public:
        enum { ID = 0x0049 };

        WORD    slot;       // bag slot
        DWORD   itemId;
        DWORD   amount;     // sold items amount
        DWORD   money;
    #if PW_SERVER_VERSION >= 1640
        uint64_t    silverMoney;
    #endif

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.wr(slot).lr(itemId).lr(amount).lr(money);
        #if PW_SERVER_VERSION >= 1640
            s.qr(silverMoney);
        #endif
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Item to Money]  Slot:") << dec << (unsigned)slot << _T(" Id:") << hex << setw(8) << itemId
                << _T(" Amount:") << dec << amount 
                << _T(" Money:") << money 
            #if PW_SERVER_VERSION >= 1640
                << _T(" Silver:") << silverMoney
            #endif
                << endl;
        }
    };

    typedef FragmentGiSpec<ItemToMoneyImpl>   FragmentGiItemToMoney;


    class SpendMoneyImpl
    {
    public:
        enum { ID = 0x004D };

        DWORD   money;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(money);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Spend Money]  Money:") << money << endl;
        }
    };

    typedef FragmentGiSpec<SpendMoneyImpl>   FragmentGiSpendMoney;


    class GetOwnMoneyImpl
    {
    public:
        enum { ID = 0x0052 };

        DWORD       money;
        DWORD       maxMoney;
    #if PW_SERVER_VERSION >= 1640
        uint64_t    silverMoney;
    #endif

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(money).lr(maxMoney);
        #if PW_SERVER_VERSION >= 1640
            s.qr(silverMoney);;
        #endif
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Own Money] ") << dec << money << _T("/") << maxMoney
            #if PW_SERVER_VERSION >= 1640
                << L" silver:" << silverMoney
            #endif
                << endl;
        }
    };

    typedef FragmentGiSpec<GetOwnMoneyImpl>   FragmentGiGetOwnMoney;


    class ObjectCastSkillImpl
    {
    public:
        enum { ID = 0x0055 };

        DWORD   attackerId;
        DWORD   victimId;
        DWORD   skillId;
        WORD    time;
        BYTE    lvl;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(attackerId).lr(victimId).lr(skillId).wr(time).b(lvl);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Object Cast Skill]  attacker:") << hex << setw(8) << attackerId 
                   << _T(" victim:") << setw(8) << hex << victimId << _T(" cast time:") << dec << (unsigned)time
                   << _T(" skill:") << skillId << _T(" lvl:") << dec << lvl << endl;
        }
    };

    typedef FragmentGiSpec<ObjectCastSkillImpl>   FragmentGiObjectCastSkill;


    class SkillDataImpl
    {
    public:
        enum { ID = 0x005A };

        struct Skill 
        {
            WORD    id;
            BYTE    lvl;
            WORD    exp;

            template<int mode> void format(Serializer<mode> & s)
            {
                s.wr(id).b(lvl).wr(exp);
            }
        };

        std::vector<struct Skill> skills;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.arr(skills, &Serializer<mode>::lr);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Skill Data]  count:") << dec << skills.size() << endl;
            for (std::vector<struct Skill>::const_iterator it = skills.begin();
                it != skills.end(); ++it)
            {
                stream << _T("    ID:") << setw(4) << hex << (unsigned)it->id << _T(" lvl:") 
                       << dec << it->lvl << _T(" exp:") << dec << (unsigned)it->exp << endl;
            }
        }
    };

    typedef FragmentGiSpec<SkillDataImpl>   FragmentGiSkillData;


    class PlayerUseItemImpl
    {
    public:
        enum { ID = 0x005B };

        InventoryType   type;
        BYTE            slot;
        DWORD           itemId;
        WORD            count;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            byte btype = static_cast<byte>(type);
            s.b(btype).b(slot).lr(itemId).wr(count);
            type = static_cast<InventoryType>(btype);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Player use item]  Inventory:") 
                   << (type == InventoryTypeBag ? _T("BAG") : (type == InventoryTypeEquip ? _T("EQUIP") : (type == InventoryTypeQuest ? _T("QUEST") : _T("UNKNOWN"))))
                   << _T(" slot:") << dec << slot << _T(" ItemId:") << dec << itemId
                   << _T(" Count:") << (unsigned)count << endl;
        }
    };

    typedef FragmentGiSpec<PlayerUseItemImpl>   FragmentGiPlayerUseItem;


    class ObjectTakeoffImpl
    {
    public:
        enum { ID = 0x0060 };

        DWORD           id;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Object takeoff]  ID:")
                << _T(" Id:") << hex << id << endl;
        }
    };

    typedef FragmentGiSpec<ObjectTakeoffImpl>   FragmentGiObjectTakeoff;


    class ObjectLandingImpl
    {
    public:
        enum { ID = 0x0061 };

        DWORD           id;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Object takeoff]  ID:")
                << _T(" Id:") << hex << id << endl;
        }
    };

    typedef FragmentGiSpec<ObjectLandingImpl>   FragmentGiObjectLanding;


    class PlayerObtainItemImpl
    {
    public:
        enum { ID = 0x0063 };

        DWORD           itemId;
        DWORD           unk;
        #if PW_SERVER_VERSION > 1442
            DWORD       amount;
            DWORD       totalInSlot;
        #else
            WORD        amount;
            WORD        totalInSlot;
        #endif
        InventoryType   type;
        byte            slot;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            byte btype = static_cast<byte>(type);
            #if PW_SERVER_VERSION > 1442
                s.lr(itemId).lr(unk).lr(amount).lr(totalInSlot).b(btype).b(slot);
            #else
                s.lr(itemId).lr(unk).wr(amount).wr(totalInSlot).b(btype).b(slot);
            #endif
            type = static_cast<InventoryType>(btype);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Obtain Item]  Inventory:") 
                << (type == InventoryTypeBag ? _T("BAG") : (type == InventoryTypeEquip ? _T("EQUIP") : (type == InventoryTypeQuest ? _T("QUEST") : _T("UNKNOWN"))))
                << _T(" Slot:") << dec << int(slot) << _T(" Count:") << int(amount) << _T(" TotalInSlot:") << int(totalInSlot)
                << _T(" ItemID:") << itemId << _T(" Unk:") << (unsigned)unk  << endl;
        }
    };

    typedef FragmentGiSpec<PlayerObtainItemImpl>   FragmentGiPlayerObtainItem;


    class ProduceOnceImpl
    {
    public:
        enum { ID = 0x0065 };

        DWORD           itemId;
        #if PW_SERVER_VERSION > 1442
            DWORD       amount;
            DWORD       totalInSlot;
        #else
            WORD        amount;         // produced items amount
            WORD        totalInSlot;    // total items in slot
        #endif
        InventoryType   type;
        byte            slot;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            byte btype = static_cast<byte>(type);
            #if PW_SERVER_VERSION > 1442
                s.lr(itemId).lr(amount).lr(totalInSlot).b(btype).b(slot);
            #else
                s.lr(itemId).wr(amount).wr(totalInSlot).b(btype).b(slot);
            #endif
            type = static_cast<InventoryType>(btype);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Produce Once]  Inventory:") 
                << (type == InventoryTypeBag ? _T("BAG") : (type == InventoryTypeEquip ? _T("EQUIP") : (type == InventoryTypeQuest ? _T("QUEST") : _T("UNKNOWN"))))
                << _T(" Slot:") << dec << slot << _T(" ItemId:") << itemId
                << _T(" Amount:") << int(amount) << _T(" TotalInSlot:") << int(totalInSlot) << endl;
        }
    };

    typedef FragmentGiSpec<ProduceOnceImpl>   FragmentGiProduceOnce;


    class TaskVarDataImpl
    {
    public:
        enum { ID = 0x006A };

        enum // command
        {
            CmdAddTask      = 1,    // .wr(taskId).lr(timestamp)
            CmdRemoveTask   = 2,    // .wr(taskId)
        };

        DWORD   length;     // same as c22 32 probably
        byte    command;
        barray  data;
        // more

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(length).b(command);
            s.arr(data, length - 1, &Serializer<mode>::b);  // updated lengthS
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Task var data]  opt:") << dec << (unsigned) length
                << L" cmd:" << command << endl;
        }
    };

    typedef FragmentGiSpec<TaskVarDataImpl>   FragmentGiTaskVarData;


    class DoEmotionImpl
    {
    public:
        enum { ID = 0x0071 };

        DWORD   charId;
        WORD    emotionId;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(charId).wr(emotionId);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Do Emotion]  charID:") << hex << setw(8) << charId 
                << _T(" emotionID:") << (unsigned)emotionId << endl;
        }
    };

    typedef FragmentGiSpec<DoEmotionImpl>   FragmentGiDoEmotion;


    class ServerTimestampImpl
    {
    public:
        enum { ID = 0x0072 };

        DWORD   timestamp;      // unix time
        DWORD   tz_min;
        DWORD   luaVersion;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(timestamp).lr(tz_min).lr(luaVersion);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Server timestamp]  TS:") << hex << setw(8) << timestamp 
                   << _T("  TZ:") << dec << (int)tz_min
                   << _T("  Version:") << luaVersion << endl;
        }
    };

    typedef FragmentGiSpec<ServerTimestampImpl>   FragmentGiServerTimestamp;


    class ObjectAttackResultImpl
    {
    public:
        enum { ID = 0x0078 };

        DWORD   attackerId;
        DWORD   victimId;
        DWORD   damage;
        WORD    unk;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(attackerId).lr(victimId).lr(damage).wr(unk);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Attack Result]  attacker:") << hex << setw(8) << attackerId << _T(" victim:") << setw(8) << victimId 
                << _T(" damage:") << dec << damage << _T(" Unk:") << hex << (unsigned)unk << endl;
        }
    };

    typedef FragmentGiSpec<ObjectAttackResultImpl>   FragmentGiObjectAttackResult;


    class BeHurtImpl
    {
    public:
        enum { ID = 0x0079 };

        DWORD   attackerId;
        DWORD   damage;
        byte    unk;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(attackerId).lr(damage).b(unk);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [I'm hurt]  attacker:") << hex << setw(8) << attackerId
                << _T(" damage:") << dec << damage << _T(" Unk:") << hex << (unsigned)unk << endl;
        }
    };

    typedef FragmentGiSpec<BeHurtImpl>   FragmentGiBeHurt;


    class PlayerGatherStartImpl
    {
    public:
        enum { ID = 0x007E };

        DWORD   charId;
        DWORD   resourceId;
        byte    timeToEnd;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(charId).lr(resourceId).b(timeToEnd);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Player Gather Start]  CharID:") << hex << setw(8) << charId << _T(" ResourceID:") << setw(8) << resourceId 
                   << _T(" Time to end:") << dec << timeToEnd << endl;
        }
    };

    typedef FragmentGiSpec<PlayerGatherStartImpl>   FragmentGiPlayerGatherStart;


    class PlayerGatherStopImpl
    {
    public:
        enum { ID = 0x007F };

        DWORD   charId;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(charId);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Player Gather Stop]  CharID:") << hex << setw(8) << charId << endl;
        }
    };

    typedef FragmentGiSpec<PlayerGatherStopImpl>   FragmentGiPlayerGatherStop;


    class ExchangeTrashboxItemImpl
    {
    public:
        enum { ID = 0x0085 };

        InventoryType   type;
        byte            from;
        byte            to;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            byte btype;
            btype = static_cast<byte>(type);
            s.b(btype).b(from).b(to);
            type = static_cast<InventoryType>(btype);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Exchange Trashbox Item]  Inventory:") 
                   << (type == InventoryTypeTrashbox ? _T(" BANK") : _T(" UNKNOWN"))
                   << _T("From:") << dec << from << _T(" To:") << to << endl;
        }
    };

    typedef FragmentGiSpec<ExchangeTrashboxItemImpl>   FragmentGiExchangeTrashboxItem;


    class MoveTrashboxItemImpl
    {
    public:
        enum { ID = 0x0086 };

        InventoryType   type;
        byte            from;
        byte            to;
        DWORD           amount;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            byte btype;
            btype = static_cast<byte>(type);
            s.b(btype).b(from).b(to).lr(amount);
            type = static_cast<InventoryType>(btype);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Move Inventory Item]  Inventory:") 
                   << (type == InventoryTypeTrashbox ? _T(" BANK") : _T(" UNKNOWN"))
                   << _T("From:") << dec << from <<_T(" To:") << to << _T(" Amount:") << amount << endl;
        }
    };

    typedef FragmentGiSpec<MoveTrashboxItemImpl>   FragmentGiMoveTrashboxItem;


    class ExchangeTrashboxInventoryImpl
    {
    public:
        enum { ID = 0x0087 };

        InventoryType   type;
        byte            trashboxSlot;
        byte            bagSlot;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            byte btype;
            btype = static_cast<byte>(type);
            s.b(btype).b(trashboxSlot).b(bagSlot);
            type = static_cast<InventoryType>(btype);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Exchange Trashbox to Inventory]  Inventory:") 
                << (type == InventoryTypeTrashbox ? _T(" BANK") : _T(" UNKNOWN"))
                << _T("Bag slot:") << dec << bagSlot << _T(" Trashbox slot:") << trashboxSlot << endl;
        }
    };

    typedef FragmentGiSpec<ExchangeTrashboxInventoryImpl>   FragmentGiExchangeTrashboxInventory;


    class InventoryItemToTrashImpl
    {
    public:
        enum { ID = 0x0088 };

        InventoryType   type;
        byte            bagSlot;
        byte            trashboxSlot;
        DWORD           amount;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            byte btype;
            btype = static_cast<byte>(type);
            s.b(btype).b(bagSlot).b(trashboxSlot).lr(amount);
            type = static_cast<InventoryType>(btype);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Exchange Inventory to Trashbox]  Inventory:") 
                << (type == InventoryTypeTrashbox ? _T(" BANK") : _T(" UNKNOWN"))
                << _T("Bag slot:") << dec << bagSlot << _T(" Trashbox slot:") << trashboxSlot << _T(" Amount:") << amount << endl;
        }
    };

    typedef FragmentGiSpec<InventoryItemToTrashImpl>   FragmentGiInventoryItemToTrash;


    class TrashItemToInventoryImpl
    {
    public:
        enum { ID = 0x0089 };

        InventoryType   type;
        byte            trashboxSlot;
        byte            bagSlot;
        DWORD           amount;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            byte btype;
            btype = static_cast<byte>(type);
            s.b(btype).b(trashboxSlot).b(bagSlot).lr(amount);
            type = static_cast<InventoryType>(btype);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Exchange Trashbox to Inventory ]  Inventory:") 
                << (type == InventoryTypeTrashbox ? _T(" BANK") : _T(" UNKNOWN"))
                << _T(" Trashbox slot:") << trashboxSlot << _T("Bag slot:") << dec << bagSlot << _T(" Amount:") << amount << endl;
        }
    };

    typedef FragmentGiSpec<TrashItemToInventoryImpl>   FragmentGiTrashItemToInventory;


    class EnchantResultImpl
    {
    public:
        enum { ID = 0x008B };
        // cast skill result

        DWORD       sourceId;
        DWORD       targetId;
        DWORD       skillId;
        DWORD       skillLv;
        WORD        unk;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(sourceId).lr(targetId).lr(skillId).lr(skillLv).wr(unk);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Enchant Result]  Source ID: " << hex << setw(8) << sourceId
                << " Target ID:") << hex << setw(8) << targetId
                << _T(" SkillID:") << skillId
                << _T(" Lv:") << dec << (unsigned)skillLv
                << _T(" unk:") << (unsigned)unk << endl;
        }
    };

    typedef FragmentGiSpec<EnchantResultImpl>   FragmentGiEnchantResult;


    class SelfSkillAttackResultImpl
    {
    public:
        enum { ID = 0x008E };

        DWORD       targetId;
        DWORD       skillId;
        DWORD       damage;
        WORD        unk;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(targetId).lr(skillId).lr(damage).wr(unk);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [SelfSkill Attack Result]  Target ID:") << hex << setw(8) << targetId 
                   << _T(" SkillID:") << skillId << _T(" Damage:") << dec << damage 
                   << _T(" unk:") << (unsigned)unk << endl;
        }
    };

    typedef FragmentGiSpec<SelfSkillAttackResultImpl>   FragmentGiSelfSkillAttackResult;


    class ObjectSkillAttackResultImpl
    {
    public:
        enum { ID = 0x008F };

        DWORD       attackerId;
        DWORD       victimId;
        DWORD       unk1;
        DWORD       damage;
        WORD        unk2;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(attackerId).lr(victimId).lr(unk1).lr(damage).wr(unk2);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Object Skill Attack Result]  attacker:") << hex << setw(8) << attackerId 
                   << _T(" victim:") << setw(8) << victimId << _T(" Damage:") << dec << damage 
                   << _T(" unk:") << hex << unk1 <<_T("/") << (unsigned)unk2 << endl;
        }
    };

    typedef FragmentGiSpec<ObjectSkillAttackResultImpl>   FragmentGiObjectSkillAttackResult;


    class BeSkillAttackedImpl
    {
    public:
        enum { ID = 0x0090 };

        DWORD       attackerId;         // attacker id
        DWORD       skillId;
        DWORD	    damage;
        WORD	    effects;
        WORD	    unk2;
        WORD	    unk3;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(attackerId).lr(skillId).lr(damage).wr(effects).wr(unk2).wr(unk3);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Skill Attacked]  ID:") << hex << setw(8) << attackerId
                << dec << _T(" Skill:") << damage << _T(" Damage:") << damage 
                << hex << _T("  Eff:") << (unsigned)effects 
                << _T("  Unk2:") << (unsigned)unk2 << _T("  Unk2:") << (unsigned)unk3 << endl;
        }
    };

    typedef FragmentGiSpec<BeSkillAttackedImpl>   FragmentGiBeSkillAttacked;


    class MatterPickupImpl
    {
    public:
        enum { ID = 0x0098 };

        DWORD       id;
        DWORD	    charId;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id).lr(charId);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Matter Pickup]  ID:") << hex << id << _T(" CharID:") << setw(8) << charId << endl;
        }
    };

    typedef FragmentGiSpec<MatterPickupImpl>   FragmentGiMatterPickup;


    class TaskDeliverItemImpl
    {
    public:
        enum { ID = 0x009C };

        DWORD           itemId;
        #if PW_SERVER_VERSION > 1442
            DWORD       amount;
            DWORD       totalInSlot;
        #else
            WORD        amount;
            WORD        totalInSlot;
        #endif
        InventoryType   type;
        byte            slot;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            byte btype = static_cast<byte>(type);
            #if PW_SERVER_VERSION > 1442
                s.lr(itemId).lr(amount).lr(totalInSlot).b(btype).b(slot);
            #else
                s.lr(itemId).wr(amount).wr(totalInSlot).b(btype).b(slot);
            #endif
            type = static_cast<InventoryType>(btype);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Task Deliver Item]  Inventory: ") 
                << (type == InventoryTypeBag ? _T("BAG") : (type == InventoryTypeEquip ? _T("EQUIP") : (type == InventoryTypeQuest ? _T("QUEST") : _T("UNKNOWN"))))
                << _T(" slot:") << slot << _T(" item:") << dec << itemId << _T(" Amount:") << int(amount)
                << _T(" Total:") << int(totalInSlot) << endl;
        }
    };

    typedef FragmentGiSpec<TaskDeliverItemImpl>   FragmentGiTaskDeliverItem;


    class PlayerOpenMarketImpl
    {
    public:
        enum { ID = 0x00A6 };

        DWORD       id;
        BYTE        chk;
        wstring     name;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            // <stringLen>.b
            s.lr(id).b(chk).s(name);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Player Open Market]  ID:") << hex << setw(8) << id << _T(" Name:") << name << _T(" chk:") << chk << endl;
        }
    };

    typedef FragmentGiSpec<PlayerOpenMarketImpl>   FragmentGiPlayerOpenMarket;


    class SelfOpenMarketImpl
    {
    public:
        enum { ID = 0x00A7 };

        struct Item
        {
            DWORD       id;
            WORD        slot;               // inventory slot of item. slot < 0 is buying item
            #if PW_SERVER_VERSION >= 1450
                DWORD   count;
            #else
                WORD    count;
            #endif
            DWORD       price;

            template<int mode> void format(Serializer<mode> & s)
            {
                #if PW_SERVER_VERSION >= 1450
                    s.lr(id).wr(slot).lr(count).lr(price);
                #else
                    s.lr(id).wr(slot).wr(count).lr(price);
                #endif
                
            }
        };

        std::vector<Item> items;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.arr(items, &Serializer<mode>::wr);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Self Open Market]  items(") << items.size() << _T(")") << endl;
            for (std::vector<Item>::const_iterator it = items.begin();
                 it != items.end(); ++it)
            {
                stream << _T("  id:") << dec << it->id << _T(" count:") << int(it->count) 
                    << _T(" price:") << it->price;
                if(it->slot != 0xFFFF)
                    stream << _T(" slot:") << (unsigned)it->slot << endl;
                else
                    stream << _T(" item for buy") << endl;
            }
        }
    };

    typedef FragmentGiSpec<SelfOpenMarketImpl>   FragmentGiSelfOpenMarket;


    class PlayerCancelMarketImpl
    {
    public:
       enum { ID = 0x00A8 };

       DWORD   id;

    protected:
       template<int mode> void format(Serializer<mode> & s)
       {
           s.lr(id);
       }

       void print(tostream & stream) const
       {
           stream << _T("  [Player Cancel Market]  ID:") << hex << setw(8) << id << endl;
       }
    };

    typedef FragmentGiSpec<PlayerCancelMarketImpl>   FragmentGiPlayerCancelMarket;


    class PlayerMarketInfoImpl
    {
    public:
        enum { ID = 0x00A9 };

        struct ItemData
        {
            // itemID = 0 means invalid item
            DWORD   itemID;
            int     count;
            DWORD   price;
            DWORD   unk;
            DWORD   unk2;
            barray  attrs;

            template<int mode> void format(Serializer<mode> & s)
            {
                s.lr(itemID);

                if(itemID != 0)
                {
                    s.lr((DWORD&)count).lr(price);

                    if(count > 0)
                    {
                        // предмет на продажу
                        s.lr(unk)
                        #if PW_SERVER_VERSION >= 1620
                            .lr(unk2)
                        #endif
                            .arr(attrs, &Serializer<mode>::wr, &Serializer<mode>::b);
                    }
                    else
                    {
                        // предмет на покупку
                    }
                }
                else
                {
                    //предмет, который здесь был, уже продан
                }
            }
        };

        DWORD   characterID;
        DWORD   chk;

        std::vector<struct ItemData>  items;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(characterID).lr(chk).arr(items, &Serializer<mode>::lr);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Player Market Info] (") << dec << items.size() <<_T(") chk:") << hex << chk << endl;

            for (std::vector<struct ItemData>::const_iterator it = items.begin();
                it != items.end(); ++it)
            {
                const struct ItemData &i = *it;
                stream << _T("    ID:") << hex << setw(8) << i.itemID << dec << _T(" Count:") << i.count
                       << _T(" Price:") << i.price << _T(" Unk:") << i.unk 
                       << _T(" Attrs:") << i.attrs << endl;
            }
        }
    };

    typedef FragmentGiSpec<PlayerMarketInfoImpl>   FragmentGiPlayerMarketInfo;


    class PlayerSuccessMarketImpl
    {
        // обычно означает некий факт, что торговл€ закончилась. Ќе обозначает ни пустой магазин, низакрытие кота.
        // может присылатьс€ покупателю.
    public:
        enum { ID = 0x00AA };

        DWORD   id;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Player Success Market]  ID:") << hex << setw(8) << id << endl;
        }
    };

    typedef FragmentGiSpec<PlayerSuccessMarketImpl>   FragmentGiPlayerSuccessMarket;


    class PlayerMarketNameImpl
    {
    public:
        enum { ID = 0x00AB };

        DWORD       id;
        byte        unk;
        wstring     name;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id).b(unk).s(name);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Player Market Name]  ID:") << hex << setw(8) << id << _T(" Name:") << name 
                   << _T(" unk:") << unk << endl;
        }
    };

    typedef FragmentGiSpec<PlayerMarketNameImpl>   FragmentGiPlayerMarketName;


    class SelfTraceCurPosImpl
    {
    public:
        enum { ID = 0x00B1 };

        Coord3D     pos;
        WORD        seqNum;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.fr(pos.x_).fr(pos.z_).fr(pos.y_).wr(seqNum);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Trace CurPos]  [") << pos << _T("]  Seq:") << hex << (unsigned)seqNum << endl;
        }
    };

    typedef FragmentGiSpec<SelfTraceCurPosImpl>   FragmentGiSelfTraceCurPos;


    class PlayerWaypointListImpl
    {
    public:
        enum { ID = 0x00B4 };

        std::vector<WORD>    points;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.arr(points, &Serializer<mode>::lr, &Serializer<mode>::wr);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Player Waypoint List]") << _T("   points: ") << dec << points.size() << _T(" - ");
            for (const auto & it : points)
            {
                stream << _T(" : ") << hex << setw(4) << (int)it;
            }
            stream << endl;
        }
    };

    typedef FragmentGiSpec<PlayerWaypointListImpl>   FragmentGiPlayerWaypointList;


    class UnlockInventorySlotImpl
    {
    public:
        enum { ID = 0x00B5 };

        byte    inventoryType;
        WORD    slot;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.b(inventoryType).wr(slot);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Unlock Inventory Slot]") 
                << _T("   InvType: ") << dec << inventoryType
                << _T(" slot: ") << slot
                << endl;
        }
    };

    typedef FragmentGiSpec<UnlockInventorySlotImpl>   FragmentGiUnlockInventorySlot;


    class PersonalMarketAvailableImpl
    {
    public:
        enum { ID = 0x00BC };

    protected:
        template<int mode> void format(Serializer<mode> & /*s*/)
        {}

        void print(tostream & stream) const
        {
            stream << _T("  [Personal Market Available]") << endl;
        }
    };

    typedef FragmentGiSpec<PersonalMarketAvailableImpl>   FragmentGiPersonalMarketAvailable;


    class TradeAwayItemImpl
    {
    public:
        enum { ID = 0x00BF };

        WORD    slot;
        DWORD   itemId;
        DWORD   count;
        DWORD   buyerId;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.wr(slot).lr(itemId).lr(count).lr(buyerId);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [TradeAway Item] slot:") << dec << (unsigned)slot << _T(" item id:") << itemId
                   << _T(" count:") << count << _T(" buyer id:") << hex << setw(8) << buyerId << endl;
        }
    };

    typedef FragmentGiSpec<TradeAwayItemImpl>   FragmentGiTradeAwayItem;


    class EnableResurrectStateImpl
    {
    public:
        enum { ID = 0x00C5 };

        DWORD   type;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(type);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Enable Resurrect State]  Type:") << hex << setw(8) << type << endl;
        }
    };

    typedef FragmentGiSpec<EnableResurrectStateImpl>   FragmentGiEnableResurrectState;


    class SetCooldownImpl
    {
    public:
        enum { ID = 0x00C6 };

        DWORD   id;
        DWORD   time;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(id).lr(time);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Set cooldown]  Type:") << hex << setw(8) << id << _T("  Time:") << dec << time << endl;
        }
    };

    typedef FragmentGiSpec<SetCooldownImpl>   FragmentGiSetCooldown;


    class ServerConfigImpl
    {
    public:
        enum { ID = 0x00CE };

        DWORD   instanceId;
        DWORD   regionTag;
        DWORD   precinctTag;
        DWORD   malltimestamp;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(instanceId).lr(regionTag).lr(precinctTag).lr(malltimestamp);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [ServerConfig]  tags  World:") << hex << instanceId << _T("  Region:") << regionTag 
                   << _T("  Precinct:") << precinctTag << _T("  MailTS:") << malltimestamp << endl;
        }
    };

    typedef FragmentGiSpec<ServerConfigImpl>   FragmentGiServerConfig;


    class PlayerRushMode
    {
    public:
        enum { ID = 0x00CF };

        byte    mode;
        // 00 - walk
        // 01 - fly

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.b(PlayerRushMode::mode);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [RushMode]  Mode:") << hex << (int)mode;
        }
    };

    typedef FragmentGiSpec<PlayerRushMode>   FragmentGiPlayerRushMode;


    class PlayerCash
    {
    public:
        enum { ID = 0x00FD };

        DWORD   count;  // in 'silvers'

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.lr(count);
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Player Cash]  Silver:") << count;
        }
    };

    typedef FragmentGiSpec<PlayerCash>   FragmentGiPlayerCash;


    class ProtectionInfoImpl
    {
    public:
        enum { ID = 0x0105 };

        enum LockState
        {
            LockStateUnknown = 0,
            LockStateOnV1 = 1,
            LockStateOff = 2,
            LockStateOn = 3,
        };

        byte    lock;           // LockState
        DWORD   unlockTS;       // server timestamp of unlocking
        DWORD   lockTime;
    #if PW_SERVER_VERSION >= 1640
        byte    lock2;
    #endif

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
            s.b(lock).lr(unlockTS).lr(lockTime);
        #if PW_SERVER_VERSION >= 1640
            s.b(lock2);
        #endif
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Protection Info] lock:") << lock 
                << _T(" UnlockTS:") << hex << setw(8) << unlockTS
                << _T(" Time in sec:") << dec << lockTime 
            #if PW_SERVER_VERSION >= 1640
                << _T(" v2:") << dec << lock2
            #endif
                << endl;
        }
    };

    typedef FragmentGiSpec<ProtectionInfoImpl>   FragmentGiProtectionInfo;


    class OnlineRegistrationInfoImpl
    {
    public:
        enum { ID = 0x016E };

        byte    unk1;
        DWORD   daysMask;   // bits of days left to right: day1 is 001 day2 is 010 ...
        DWORD   unk2;
        DWORD   monthsMask;
        DWORD   serverTs;

    protected:
        template<int mode> void format(Serializer<mode> & s)
        {
        #if PW_SERVER_VERSION < 1720
            s.b(unk1).lr(daysMask).lr(unk2).lr(monthsMask).lr(serverTs);
        #else
            s.b(unk1).lr(daysMask).lr(monthsMask).lr(serverTs).skip(3);
        #endif // PW_SERVER_VERSION < 1720
        }

        void print(tostream & stream) const
        {
            stream << _T("  [Online Registration Info] days:") << setw(8) << hex
                << daysMask
                << _T(" months:") << monthsMask
                << _T(" TS:") << serverTs
                << _T(" u1:") << int(unk1)
                << _T(" u2:") << unk2
                << endl;
        }
    };

    typedef FragmentGiSpec<OnlineRegistrationInfoImpl>   FragmentGiOnlineRegistrationInfo;

} // namespace

#endif
