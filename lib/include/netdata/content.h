#ifndef content_h
#define content_h

#include "types.h"
#include "sw_version.h"
#include "serialize.h"

enum MoveType
{
    MoveTypeWalk        = 0x20,
    MoveTypeRun         = 0x21,
    MoveTypeFallFly     = 0x28,
    MoveTypeSwim        = 0x50,
    MoveTypeMountFly    = 0x61,
};

enum PlayerLogoutType
{
    PlayerLogoutGame        = 0x00,
    PlayerLogoutCharSelect  = 0x01,
};

enum InventoryType
{
    InventoryTypeBag            = 0,
    InventoryTypeEquip          = 1,
    InventoryTypeQuest          = 2,
    InventoryTypeTrashbox       = 3,        // ����
    InventoryTypeTrashboxRes    = 4,        // ���� �����
};

enum EquipmentSlots
{
    Weapon          = 0,        // ������
    Helm            = 0x01,     // ����
    Necklace        = 0x02,     // ��������
    Manteau         = 0x03,     // �������
    Shirt           = 0x04,     // ����
    Waist           = 0x05,     // ����
    Pants           = 0x06,     // ���
    Footwear        = 0x07,     // ������
    WristBracer     = 0x08,     // ������
    Ring1           = 0x09,     // ������ 1
    Ring2           = 0x0A,     // ������ 2
    Arrows          = 0x0B,     // ������
    FlyMount        = 0x0C,     // �����
    StyleBody       = 0x0D,     // �����-����
    StyleLegs       = 0x0E,     // �����-���
    StyleShoes      = 0x0F,     // �����-�����
    StyleBracer     = 0x10,     // �����-��������
    //               0x11,      // ������
    //               0x12,      // �����
    //               0x13,      // ������
    HpHierogram     = 0x14,     // ����� �� ��
    MpHierogram     = 0x15,     // ����� �� ��
    Genie           = 0x17,     // ����
    CatShop         = 0x18,     // �������� �������
    StyleHead       = 0x19,     // �����-�����
};

enum CooldownType
{
    CooldownTypeHpMpPotion          = 0x03,
    CooldownTypeHpPotion            = 0x0B,
    CooldownTypeMpPotion            = 0x0C,
    CooldownTypeSpeedFly            = 0x0E,         // ����� �����
    CooldownTypePetFeeding          = 0x12,         // ��� �����
    CooldownTypeDefCharms           = 0x17,         // ���/��� �����
    CooldownTypeChemistry           = 0x15,         // �����
    CooldownTypeInventorySorting    = 0x2F,         // ���������� ���������
    CooldownTypeGenieSkill          = 0x1E,         // ������ �����
};

enum CharacterIdFlags
{
    //CHARACTER_ITEM_FLAG      = 0x40000000,
    CHARACTER_NPC_FLAG      = 0x80000000,
};

enum CharacterGender
{
    CharacterGenderMale     = 0,
    CharacterGenderFemale   = 1,
};

enum CharacterClass
{
    CharacterClassWarrior   = 0,
    CharacterClassWizard,
    CharacterClassPsychic,
    CharacterClassVenomancer,
    CharacterClassBarbarian,
    CharacterClassAssassin,
    CharacterClassArcher,
    CharacterClassCleric,
    CharacterClassSeeker,
    CharacterClassMystic,
};

//

class CharAttrs
{
public:
    CharAttrs()
    {}

    bool read(SerializerIn & s);

    bool isDead() const
    {
        return (primary_[0] & 0x80) != 0;
    }

    void setDead(bool value)
    {
        if (value)
        {
            primary_[0] |= 0x80;
        }
        else
        {
            primary_[0] &= ~0x80;
        }
    }

    bool isCat() const
    {
        return isCat_;
    }

    void setCat(bool cat)
    {
        isCat_ = cat;
    }

    std::pair< bool, DWORD > getGuild() const
    {
        std::pair< bool, DWORD > res;
        res.first = inGuild_;
        if (inGuild_)
        {
            res.second = guildId_;
        }
        return res;
    }

    bool isNull()
    {
        return isEmpty_;
    }

protected:
    void checkParsed();

    friend tostream & operator<<(tostream & stream, CharAttrs attrs);

private:
    bool    isEmpty_ = true;

    bool    isParsed_ = false;
#if PW_SERVER_VERSION < 1500
    byte    primary_[4];
#elif PW_SERVER_VERSION < 1760
    byte    primary_[8];
#else
    byte    primary_[12];
#endif
    barray  additional_;

    bool    inGuild_ = false;
    DWORD   guildId_ = 0;
    byte    guildRank_ = 0;

    bool    isCat_ = false;
};

tostream & operator<<(tostream & stream, CharAttrs attrs);

//------------------------------------------------------------------------------

// X-Y is horizontal plane
// Z is height

class Coord3D
{
public:
    Coord3D()
        : x_(0), y_(0), z_(0)
    {}

    // Create from 'external' values
    Coord3D(float x, float y, float z)
        : x_((x * 10) - 4000)
        , y_((y * 10) - 5500)
        , z_(z * 10)
    {}

    // Create from internal values
    static Coord3D fromInternal(float x, float z, float y)
    {
        Coord3D result;
        result.x_ = x;
        result.z_ = z;
        result.y_ = y;
        return result;
    }

    // Readable format
    float x() const
    {
        return (x_ + 4000)/10;
    }

    float y() const
    {
        return (y_ + 5500)/10;
    }

    float z() const
    {
        return z_/10;
    }

    bool isNull() const
    {
        return (x_ == 0 && y_ == 0 && z_ == 0);
    }

    // Returns distance between two points in INTERNAL coordinate system
    float operator-(const Coord3D & r) const
    {
        if (*this == r)
        {
            return 0;
        }
        return sqrt( (x_ - r.x_)*(x_ - r.x_)
                   + (y_ - r.y_)*(y_ - r.y_)
                   + (z_ - r.z_)*(z_ - r.z_));
    }

    // move point toward destination point to the specified offsets.
    Coord3D moved(float dx, float dy, float dz) const
    {
        Coord3D result;
        result.x_ = x_ + dx;
        result.z_ = z_ + dz;
        result.y_ = y_ + dy;
        return result;
    }

    // Precision is in INTERNAL coordinate system
    bool equalsTo(const Coord3D & r, float precision = 0.01) const
    {
        return abs(*this - r) < precision;
    }

    // move point toward destination point to the specified distance.
    // distance measured in INTERNAL coordinate system
    // return: distance moved, could be less than ToMove if destination is nearer
    float moveTo(Coord3D destination, float distanceToMove)
    {
        // ���������, ���������� �������� �� ������ ���������� ��� �����������
        // �� �������� ���������� �� ������ ��������������� ���������
        // ������� ���������� �� ������ � ��������� ����������.

        float distance = *this - destination;
        if (distanceToMove >= distance)
        {
            *this = destination;
            return distance;
        }

        float factor = distanceToMove / distance;

        x_ += factor * (destination.x_ - x_);
        y_ += factor * (destination.y_ - y_);
        z_ += factor * (destination.z_ - z_);

        return distanceToMove;
    }

    // returns angle of line p->r, 360 is 0xFF
    byte directionTo(const Coord3D & r) const
    {
        float dx = r.x_ - x_;
        float dy = r.y_ - y_;

        if (dx == 0 && dy == 0)
        {
            return 0;
        }

        float angle = atan2(dy, dx) * 180 / 3.1415926535f;
        if (angle < 0)
        {
            angle = 360 + angle;
        }
        return byte(angle * 255 / 360);
    }

    bool operator==(const Coord3D & r) const
    {
        return x_ == r.x_ && y_ == r.y_ && z_ == r.z_;
    }

    // Internal representation
    float   x_, y_, z_;
};


class CoordEx : public Coord3D
{
public:
    CoordEx()
        : angle(0)
    {}

    CoordEx(const Coord3D & pos)
        : Coord3D(pos)
        , angle(0)
    {}

    CoordEx(const Coord3D & pos, byte ang)
        : Coord3D(pos)
        , angle(ang)
    {}

    byte    angle;
};

tostream & operator<<(tostream & stream, const Coord3D & coords);


struct CharacterRecord
{
    struct EquipItem
    {
        DWORD       id;
        DWORD       pos;
        DWORD       count;
        DWORD       maxCount;
        barray      attrs;
        DWORD       proctype;
        DWORD       expiration; // ts
        DWORD       guid1;
        DWORD       guid2;
        DWORD       mask;

        template<int mode> void format(Serializer<mode> & s)
        {
            s.l(id).l(pos).l(count).l(maxCount)
                .arr(attrs, &Serializer<mode>::cui, &Serializer<mode>::b)
                .l(proctype).l(expiration).l(guid1).l(guid2).l(mask);
        }
    };

    enum CharacterStatus
    {
        CharacterStatusNone     = 0,
        CharacterStatusActive   = 1,
        CharacterStatusDeleing  = 3,
    };

    barray                  faceData;
    std::vector<EquipItem>  equip;
    byte        status;           // 1 - active, 0 ? - deleted, 3 - deleting
    DWORD       deleteTime;         // TimeStamp
    DWORD       createTime;         // TimeStamp
    DWORD       lastLoginTime;      // TimeStamp
    Coord3D     position;
    DWORD       worldId;

    template<int mode> void format(Serializer<mode> & s)
    {
        s.arr(faceData, &Serializer<mode>::cui, &Serializer<mode>::b)
            .arr(equip, &Serializer<mode>::cui)
            .b(status).l(deleteTime).l(createTime).l(lastLoginTime)
            .f(position.x_).fr(position.z_).fr(position.y_)
            .l(worldId);
    }
};

#endif