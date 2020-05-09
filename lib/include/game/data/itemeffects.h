#ifndef itemeffects_h
#define itemeffects_h

#include "elements.h"
#include "netdata/serialize.h"
#include "json_spirit.h"

class ItemProps;
template<class T> class ItemPropsT;
class ArmorPropsImpl;
class WeaponPropsImpl;
typedef ItemPropsT<ArmorPropsImpl> ArmorProps;
typedef ItemPropsT<WeaponPropsImpl> WeaponProps;


typedef std::map<int /*effectId*/, int /*effectType*/>          TypesMap;
typedef std::map<int /*effectType*/, std::wstring /*format*/>   FormatsMap;

class ItemEffects
{
public:

    enum EffectType
    {
        Unknown         = -1,
        Unspecified     = 0x00,
        StoneBonus      = 0x04,
        EngravingBonus  = 0x08,
        BuffBonus       = 0x10,
    };

    ItemEffects()
        : id(0)
    {}

    EffectType effectType() const
    {
        return (EffectType)((id & 0xF8000) >> 14);
    }

    unsigned effectId() const
    {
        return (id & 0x01FFF);
    }

    int argsCount() const
    {
        return (id & 0x06000) >> 13;
    }

    DWORD       id;
    DWORD       values[4];

    template<int mode>
    void format(Serializer<mode> & s)
    {
        s.lr(id);
        int count = argsCount();
        s.m(values, count, &Serializer<mode>::lr);
    }
    bool operator==(const ItemEffects & r) const;

    std::wstring toString(const TypesMap & types, const FormatsMap & formats) const;
    int toClass(const TypesMap & types) const;
};


class ItemPropsDecoder
{
public:
    ItemPropsDecoder( const wchar_t *propDesc
                    , const wchar_t *propTypes
                    , const wchar_t *propMap);

    bool isLoaded() const
    {
        return isLoaded_;
    }

    void setElements(const elements::ItemListCollection & items);
    std::wstring decode(const ItemProps * props);

private:
    std::wstring decode(const ArmorPropsImpl * armor);
    std::wstring decode(const WeaponProps * weapon);

    void decodeEffects(json_spirit::wmObject & js, const std::vector<ItemEffects> & effects);
    void decodeSlots(json_spirit::wmObject & js, const std::vector<DWORD> & slots);

private:
    elements::ItemListCollection    items_;

    TypesMap    effectTypes_;
    FormatsMap  effectFormats_;

    bool        isLoaded_;
};

#endif