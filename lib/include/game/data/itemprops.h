#ifndef itemprops_h
#define itemprops_h

#include "elements.h"
#include "netdata/serialize.h"
#include "game/data/itemeffects.h"

enum ItemPropType
{
    ItemPropTypeUnspecified,
    ItemPropTypeArmor,
    ItemPropTypeWeapon,
    ItemPropTypeFashion,
    ItemPropTypeFlyMount,
    ItemPropTypeGenie,
};

class ItemProps
{
public:
    virtual ~ItemProps() {}

    virtual void parse(const barray & b) = 0;
    virtual barray assemble() = 0;

    ItemPropType    type;

    virtual bool operator==(const ItemProps & r) const
    {
        return type == r.type;
    }

protected:
    ItemProps(ItemPropType t)
        : type(t)
    {}
};

template<class Impl>
class ItemPropsT 
    : public Impl
{
public:

    virtual bool operator==(const ItemProps & r) const
    {
        return type == r.type
            && Impl::operator==(*static_cast<const Impl*>(&r));
    }

    virtual void parse(const barray & b)
    {
        SerializerIn s(b);
        Impl::format(s);
    }

    virtual barray assemble()
    {
        barray b;
        SerializerLength sl;
        Impl::format(sl);
        b.resize(sl.size());

        SerializerOut s(b);
        Impl::format(s);
        return b;
    }
};

//

class WearableProps
{
public:
    WORD    reqLv;
    WORD    reqClass;
    WORD    reqStr;
    WORD    reqCon;
    WORD    reqAgi;
    WORD    reqInt;
    DWORD   duration;
    DWORD   maxDuration;
    WORD    variableLength;      // x24 - armor  x2C - weapon
    byte    itemFlags;
    std::wstring creator;

    template<int mode>
    void format(Serializer<mode> & s)
    {
        s.wr(reqLv).wr(reqClass)
        .wr(reqStr).wr(reqCon).wr(reqAgi).wr(reqInt)
        .lr(duration).lr(maxDuration)
        .wr(variableLength).b(itemFlags)
        .s(creator);
    }

    bool operator==(const WearableProps & r) const
    {
        return
           reqLv            == r.reqLv
        && reqClass         == r.reqClass
        && reqStr           == r.reqStr
        && reqCon           == r.reqCon
        && reqAgi           == r.reqAgi
        && reqInt           == r.reqInt
        && duration         == r.duration
        && maxDuration      == r.maxDuration
        && variableLength   == r.variableLength
        && itemFlags        == r.itemFlags
        && creator          == r.creator;
    }
};


class ArmorPropsImpl : public ItemProps
{
public:
    ArmorPropsImpl()
        : ItemProps(ItemPropTypeArmor)
    {}


    WearableProps   wearable;

    DWORD           def;
    DWORD           agi;
    DWORD           mp;
    DWORD           hp;
    DWORD           metalMdef;
    DWORD           woodMdef;
    DWORD           waterMdef;
    DWORD           fireMdef;           // 
    DWORD           earthMdef;          // x24 up to here

    WORD            slotNum;
    WORD            slotFlags;
    std::vector<DWORD>          sockets;
    std::vector<ItemEffects>    effects;

    template<int mode>
    void format(Serializer<mode> & s)
    {
        wearable.format(s);
        if (wearable.variableLength == 0x24)
        {
            s.lr(def).lr(agi).lr(mp).lr(hp)
            .lr(metalMdef).lr(woodMdef).lr(waterMdef).lr(fireMdef).lr(earthMdef);
        }
        else
        {
            assert(0);
            s.skip(wearable.variableLength);
        }
        s.wr(slotNum).wr(slotFlags);
        s.arr(sockets, slotNum, &Serializer<mode>::lr)
        .arr(effects, &Serializer<mode>::lr);
    }

    bool operator==(const ArmorPropsImpl & r) const
    {
        return
           wearable     == r.wearable
        && def          == r.def      
        && agi          == r.agi
        && mp           == r.mp
        && hp           == r.hp
        && metalMdef    == r.metalMdef
        && woodMdef     == r.woodMdef
        && waterMdef    == r.waterMdef
        && fireMdef     == r.fireMdef
        && earthMdef    == r.earthMdef
        && slotNum      == r.slotNum
        && slotFlags    == r.slotFlags
        && sockets      == r.sockets
        && effects      == r.effects;
    }
};

typedef ItemPropsT<ArmorPropsImpl> ArmorProps;


class WeaponPropsImpl : public ItemProps
{
public:
    WeaponPropsImpl()
        : ItemProps(ItemPropTypeWeapon)
    {}


    WearableProps   wearable;

    DWORD           ammoReq;
    DWORD           weaponType;
    DWORD           level;
    DWORD           ammoType;   // elements Id
    DWORD           minAtk;
    DWORD           maxAtk;
    DWORD           minMatk;
    DWORD           maxMatk;
    DWORD           atkSpeed;
    float           atkRange;
    float           fragDistance;       // x2C up to here

    WORD            slotNum;
    WORD            slotFlags;
    std::vector<DWORD>          sockets;
    std::vector<ItemEffects>    effects;

    template<int mode>
    void format(Serializer<mode> & s)
    {
        wearable.format(s);
        if (wearable.variableLength == 0x2C)
        {
            s.lr(ammoReq).lr(weaponType).lr(level).lr(ammoType)
            .lr(minAtk).lr(maxAtk).lr(minMatk).lr(maxMatk)
            .lr(atkSpeed).fr(atkRange).fr(fragDistance);
        }
        else
        {
            assert(0);
            s.skip(wearable.variableLength);
        }
        s.wr(slotNum).wr(slotFlags);
        s.arr(sockets, slotNum, &Serializer<mode>::lr)
        .arr(effects, &Serializer<mode>::lr);
    }

    bool operator==(const WeaponPropsImpl & r) const
    {
        return
            wearable    == r.wearable
        && ammoReq      == r.ammoReq
        && weaponType   == r.weaponType
        && level        == r.level
        && ammoType     == r.ammoType
        && minAtk       == r.minAtk
        && maxAtk       == r.maxAtk
        && minMatk      == r.minMatk
        && maxMatk      == r.maxMatk
        && atkSpeed     == r.atkSpeed
        && atkRange     == r.atkRange
        && fragDistance == r.fragDistance
        && slotNum      == r.slotNum
        && slotFlags    == r.slotFlags
        && sockets      == r.sockets
        && effects      == r.effects;
    }
};

typedef ItemPropsT<WeaponPropsImpl> WeaponProps;


class FashionPropsImpl : public ItemProps
{
public:
    FashionPropsImpl()
        : ItemProps(ItemPropTypeFashion)
    {}

    DWORD   levelReq;
    DWORD   color;      // 0RGB

    template<int mode>
    void format(Serializer<mode> & s)
    {
        DWORD c = (color & 0x0000F8) >> 3
                | (color & 0x00F800) >> 6
                | (color & 0xF80000) >> 9;
        s.lr(levelReq).lr(c).skip(4);
        color = (c & 0x001F) << 3
              | (c & 0x03E0) << 6
              | (c & 0x7C00) << 9;
    }

    bool operator==(const FashionPropsImpl & r) const
    {
        return
           levelReq == r.levelReq
        && color == r.color;
    }
};

typedef ItemPropsT<FashionPropsImpl> FashionProps;

class FlyMountPropsImpl : public ItemProps
{
public:
    FlyMountPropsImpl()
        : ItemProps(ItemPropTypeFlyMount)
    {}

    DWORD           energy;
    DWORD           maxEnergy;
    WORD            levelReq;
    WORD            level;
    float           speed;
    float           maxSpeed;
    byte            flags;
    std::wstring    creator;

    template<int mode>
    void format(Serializer<mode> & s)
    {
        s.lr(energy).lr(maxEnergy).wr(levelReq).wr(level).skip(8)
        .fr(speed).fr(maxSpeed).b(flags).s(creator).skip(2);
    }

    bool operator==(const FlyMountPropsImpl & r) const
    {
        return
           energy       == r.energy
        && maxEnergy    == r.maxEnergy
        && levelReq     == r.levelReq
        && level        == r.level
        && speed        == r.speed
        && maxSpeed     == r.maxSpeed
        && flags        == r.flags
        && creator      == r.creator;
    }
};

typedef ItemPropsT<FlyMountPropsImpl> FlyMountProps;


class GeniePropsImpl : public ItemProps
{
public:
    GeniePropsImpl()
        : ItemProps(ItemPropTypeGenie)
    {}

    DWORD       exp;
    WORD        level;
    WORD        totalPoints;

    WORD        strPoints;
    WORD        vitPoints;
    WORD        agiPoints;
    WORD        intPoints;

    WORD        totalAffinity;          // ?

    WORD        metal;
    WORD        wood;
    WORD        water;
    WORD        fire;
    WORD        earth;

    DWORD       energy;
    DWORD       tradeTime;

    std::vector<DWORD>  equipList;
    std::vector<DWORD>  skillList;

    template<int mode>
    void format(Serializer<mode> & s)
    {
        s.lr(exp).wr(level).wr(totalPoints)
        .wr(strPoints).wr(vitPoints).wr(agiPoints).wr(intPoints)
        .wr(totalAffinity)
        .wr(metal).wr(wood).wr(water).wr(fire).wr(earth)
        .skip(2)
        .lr(energy).lr(tradeTime)
        .arr(equipList, &Serializer<mode>::lr, &Serializer<mode>::lr)
        .arr(skillList, &Serializer<mode>::lr, &Serializer<mode>::lr);
    }

    bool operator==(const GeniePropsImpl & r) const
    {
        return
           exp              == r.exp          
        && level            == r.level
        && totalPoints      == r.totalPoints
        && strPoints        == r.strPoints
        && vitPoints        == r.vitPoints
        && agiPoints        == r.agiPoints
        && intPoints        == r.intPoints
        && totalAffinity    == r.totalAffinity
        && metal            == r.metal
        && wood             == r.wood
        && water            == r.water
        && fire             == r.fire
        && earth            == r.earth
        && energy           == r.energy
        && equipList        == r.equipList
        && skillList        == r.skillList;
    }
};

typedef ItemPropsT<GeniePropsImpl> GenieProps;


// Constructors

typedef ItemProps * (*itemPropsStaticCtor_fn)(const barray & data);

template<class T>
ItemProps * itemPropsStaticCtor(const barray & data)
{
    T *t = new T();
    t->parse(data);
    return t;
}

//

struct ItemPropClass
{
    ItemPropClass( const std::set<unsigned> & ids
                 , itemPropsStaticCtor_fn ct)
        : itemIds(ids)
        , ctor(ct)
    {}

    std::set<unsigned>      itemIds;
    itemPropsStaticCtor_fn  ctor;
};

class ItemPropsFactory
{
public:
    void addSet(const ItemPropClass & c);

    ItemProps * create(DWORD itemId, const barray & data);

private:
    std::vector<ItemPropClass>  itemClasses_;
};


// test
#if defined(DEBUG)
    void itemPropTest(const ItemPropClass & wpClass);
#endif

#endif