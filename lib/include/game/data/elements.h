#ifndef elements_h
#define elements_h

#include "error.h"
#include "filereader.h"
    
#include <set>
#include <memory>

namespace elements
{
    // List names

    static const char * WEAPONS             = "WEAPON_ESSENCE";
    static const char * ARMORS              = "ARMOR_ESSENCE";
    static const char * ORNAMENTS           = "DECORATION_ESSENCE";
    static const char * POTIONS             = "MEDICINE_ESSENCE";
    static const char * MOLDS               = "MATERIAL_ESSENCE";
    static const char * ATK_CHARMS          = "DAMAGERUNE_ESSENCE";
    static const char * DEF_CHARMS          = "ARMORRUNE_ESSENCE";
    static const char * SKILLS              = "SKILLTOME_ESSENCE";
    static const char * FLY_MOUNTS          = "FLYSWORD_ESSENCE";
    static const char * ELF_WINGS           = "WINGMANWING_ESSENCE";
    static const char * TELEPORTS           = "TOWNSCROLL_ESSENCE";
    static const char * KEY_ITEMS           = "REVIVESCROLL_ESSENCE";
    static const char * CHI_STONES          = "ELEMENT_ESSENCE";
    static const char * QUEST_ITEMS         = "TASKMATTER_ESSENCE";
    static const char * AMMUNITION          = "TOSSMATTER_ESSENCE";
    static const char * AMMO                = "PROJECTILE_ESSENCE";
    static const char * QUIVERS             = "QUIVER_ESSENCE";
    static const char * SOULGEMS            = "STONE_ESSENCE";
    static const char * QUEST_ACTIVATORS    = "TASKDICE_ESSENCE";
    static const char * QUEST_REWARDS       = "TASKNORMALMATTER_ESSENCE";
    static const char * RESOURCES           = "MINE_ESSENCE";
    static const char * FASHION             = "FASHION_ESSENCE";
    static const char * TRANS_SCROLLS       = "FACETICKET_ESSENCE";
    //static const char * HEAD_BOLUS          = "SUITE_ESSENCE";
	static const char * PET_ESSENCE			= "PET_ESSENCE";
    static const char * PET_EGGS            = "PET_EGG_ESSENCE";
    static const char * PET_FOOD            = "PET_FOOD_ESSENCE";
    static const char * PET_TRANSFIGURATION = "PET_FACETICKET_ESSENCE";
    static const char * FIREWORKS           = "FIREWORKS_ESSENCE";
    static const char * CATAPULT            = "WAR_TANKCALLIN_ESSENCE";
    static const char * CHEMISTRY           = "SKILLMATTER_ESSENCE";
    static const char * REFINING            = "REFINE_TICKET_ESSENCE";
    static const char * HEAVEN_BOOKS        = "BIBLE_ESSENCE";
    static const char * CHAT                = "SPEAKER_ESSENCE";
    static const char * HIEROGRAMS_HP       = "AUTOHP_ESSENCE";
    static const char * HIEROGRAMS_MP       = "AUTOMP_ESSENCE";
    static const char * EXP_MULTIPLIERS     = "DOUBLE_EXP_ESSENCE";
    static const char * TELEPORT_SCROLLS    = "TRANSMITSCROLL_ESSENCE";
    static const char * DYE                 = "DYE_TICKET_ESSENCE";
    static const char * GENIES              = "GOBLIN_ESSENCE";
    static const char * GENIE_ARMOR         = "GOBLIN_EQUIP_ESSENCE";
    static const char * GENIE_EXP_CUBES     = "GOBLIN_EXPPILL_ESSENCE";
    static const char * VENDOR_SKINS        = "SELL_CERTIFICATE_ESSENCE";
    static const char * SPY_EYE             = "TARGET_ITEM_ESSENCE";
    static const char * CRAFT_SCROLLS       = "INC_SKILL_ABILITY_ESSENCE";
    static const char * MARRIAGE_ITEMS      = "WEDDING_BOOKCARD_ESSENCE";
    static const char * MARRIAGE_CARDS      = "WEDDING_INVITECARD_ESSENCE";
    static const char * RUNES               = "SHARPENER_ESSENCE";
    static const char * GUILD_MATERIALS     = "FACTION_MATERIAL_ESSENCE";
    static const char * CONGREGATE_ESSENCE  = "CONGREGATE_ESSENCE";
    // 1.4.5
    static const char * VANGUARD_ODRERS     = "FORCE_TOKEN_ESSENCE";
    static const char * BANKNOTES           = "MONEY_CONVERTIBLE_ESSENCE";
    // 1.5.1
    static const char * UNIVERSAL_TOKEN     = "UNIVERSAL_TOKEN_ESSENCE";
	static const char * POKER_ESSENCE		= "POKER_ESSENCE";
    static const char * CARDS               = "POKER_DICE_ESSENCE";
    // 1.5.2
    static const char * ASTROLABE_ESSENCE                       = "ASTROLABE_ESSENCE";  
    static const char * ASTROLABE_RANDOM_ADDON_ESSENCE          = "ASTROLABE_RANDOM_ADDON_ESSENCE";
    static const char * ASTROLABE_INC_INNER_POINT_VALUE_ESSENCE = "ASTROLABE_INC_INNER_POINT_VALUE_ESSENCE";
    static const char * ASTROLABE_INC_EXP_ESSENCE               = "ASTROLABE_INC_EXP_ESSENCE";
    static const char * ITEM_PACKAGE_BY_PROFESSION_ESSENCE      = "ITEM_PACKAGE_BY_PROFESSION_ESSENCE";
    // 1.5.3.1
    static const char * FIREWORKS2_ESSENCE                      = "FIREWORKS2_ESSENCE";
    static const char * FIX_POSITION_TRANSMIT_ESSENCE           = "FIX_POSITION_TRANSMIT_ESSENCE";
    // 1.5.5
    static const char * HOME_FORMULAS_ITEM_ESSENCE              = "HOME_FORMULAS_ITEM_ESSENCE";
    static const char * WISH_TRIBUTE_ESSENCE                    = "WISH_TRIBUTE_ESSENCE";
    static const char * EASY_PRODUCE_ITEM_ESSENCE               = "EASY_PRODUCE_ITEM_ESSENCE";
    static const char * UNLOCK_RUNE_SLOT_ITEM_ESSENCE           = "UNLOCK_RUNE_SLOT_ITEM_ESSENCE";
    static const char * RUNE_ITEM_ESSENCE                       = "RUNE_ITEM_ESSENCE";

	static const char * HOME_UNLOCK_PAPER_ESSENCE				= "HOME_UNLOCK_PAPER_ESSENCE";
    static const char * FIX_MONSTER_ITEM_ESSENCE                = "FIX_MONSTER_ITEM_ESSENCE";
    static const char * ITEM_USED_FOR_AREA_ESSENCE              = "ITEM_USED_FOR_AREA_ESSENCE";
    static const char * CAPTURE_ITEM_ESSENCE                    = "CAPTURE_ITEM_ESSENCE";
    static const char * BIBLE_REFINE_TICKET_ESSENCE             = "BIBLE_REFINE_TICKET_ESSENCE";
    static const char * NEW_ARMOR_ESSENCE                       = "NEW_ARMOR_ESSENCE";
    static const char * QIHUN_ESSENCE                           = "QIHUN_ESSENCE";
    static const char * QILING_ESSENCE                          = "QILING_ESSENCE";
    static const char * STORY_BOOK_MAJOR_TYPE                   = "STORY_BOOK_MAJOR_TYPE";
    static const char * STORY_BOOK_CONFIG                       = "STORY_BOOK_CONFIG";
    static const char * QIHUN_COVER_ESSENCE                     = "QIHUN_COVER_ESSENCE";
    // 1.6.
    static const char * SLIDE_SKILL_ESSENCE          = "SLIDE_SKILL_ESSENCE";
    static const char * USE_FOR_SELF_ITEM_ESSENCE    = "USE_FOR_SELF_ITEM_ESSENCE";
    // 1.7.0    // 1.7.0"
    static const char * RED_BOOK_UPGRADE_ITEM        = "RED_BOOK_UPGRADE_ITEM";
    static const char * CAMP_TOKEN_ESSENCE           = "CAMP_TOKEN_ESSENCE";
    static const char * FASHION_NEW_ESSENCE          = "FASHION_NEW_ESSENCE";
    static const char * ILLUSTRATED_FASHION_ESSENCE  = "ILLUSTRATED_FASHION_ESSENCE";
    static const char * ILLUSTRATED_WING_EGG_ESSENCE = "ILLUSTRATED_WING_EGG_ESSENCE";
    static const char * ILLUSTRATED_PET_EGG_ESSENCE  = "ILLUSTRATED_PET_EGG_ESSENCE";
    static const char * FAST_PRODUCE_ITEM_ESSENCE    = "FAST_PRODUCE_ITEM_ESSENCE";
    static const char * KID_DEBRIS_ESSENCE           = "KID_DEBRIS_ESSENCE";
    static const char * KID_DEBRIS_GENERATOR_ESSENCE = "KID_DEBRIS_GENERATOR_ESSENCE";


class Error : public std::runtime_error
{
public:
    Error(const std::string & msg)
        : std::runtime_error(msg)
    {}
};

class ValueError : public Error
{
public:
    ValueError(const char * from, const char * to)
        : Error( std::string("Invalid attempt to get '")
               + to + "' type from '"
               + from + "'")
    {}
};

class ItemValueNotFound : public Error
{
public:
    ItemValueNotFound(const std::string & itemName, const char *name)
        : Error( std::string("Element value '") + name + "' could not be found"
               + (itemName.empty() ? "." : (std::string(" in item '") + itemName + "'.")) )
    {}
};


/*
    Item:                A single item. Typical query is bool = get("name", <type> result)
    ItemList:            Id-based collection of objects of the same type. Typical query is get(id, item).
    ItemListCollection:  A set of ItemLists. Typical query is get(id, item); list = [id]
*/

class Value
{
public:
    enum ValueType
    {
        Int32   = 0,
        Float,
        Barray,
        String,
        Wstring,
        CountOverride,
        NpcTalk,
        Unknown,
    };

    Value() : type(Unknown), vBarray(0), vString(0), vWstring(0) {}

    virtual ~Value()
    {
        if (type == Barray)
        {
            delete vBarray;
        }
        else if (type == String)
        {
            delete vString;
        }
        else if (type == Wstring)
        {
            delete vWstring;
        }
    }

    Value(const Value & r)
    {
        *this = r;
    }

    Value(Value && r)
    {
        *this = r;
    }

    Value & operator=(Value && r)
    {
        type = r.type;
        intValue = r.intValue;
        vBarray = r.vBarray;
        r.vBarray = 0;
        vString = r.vString;
        r.vString = 0;
        vWstring = r.vWstring;
        r.vWstring = 0;
        return *this;
    }
    Value & operator=(const Value & r)
    {
        type = r.type;
        intValue = r.intValue;
        vBarray = type == Barray ? new barray(*r.vBarray) : 0;
        vString = type == String ? new std::string(*r.vString) : 0;
        vWstring = type == Wstring ? new std::wstring(*r.vWstring) : 0;
        return *this;
    }

    Value(int v)                    { *this = v; }
    Value(float v)                  { *this = v; }
    Value(const barray & v)         { *this = v; }
    Value(const std::string & v)    { *this = v; }
    Value(const std::wstring & v)   { *this = v; }

    operator bool() const           { return type != Unknown; }

    operator unsigned() const       { checkType(Int32);     return (unsigned)intValue.vInt; }
    operator int() const            { checkType(Int32);     return intValue.vInt; }
    operator float() const          { checkType(Float);     return intValue.vFloat; }
    operator std::string() const    { checkType(String);    return *vString; }
    operator std::wstring() const   { checkType(Wstring);   return *vWstring; }
    operator barray() const         { checkType(Barray);    return *vBarray; }

    Value & operator=(int v)                    { type = Int32;     intValue.vInt = v; return *this; }
    Value & operator=(float v)                  { type = Float;     intValue.vFloat = v; return *this; }
    Value & operator=(const barray & v)         { type = Barray;    vBarray = new barray(v); return *this; }
    Value & operator=(const std::string & v)    { type = String;    vString = new std::string(v); return *this; }
    Value & operator=(const std::wstring & v)   { type = Wstring;   vWstring = new std::wstring(v); return *this; }

private:
    void checkType(ValueType vTo) const
    {
        if (type == vTo)
        {
            return;
        }

        static const char *typeNames[] =
        {
            "Int32",
            "Float",
            "Binary array",
            "String",
            "Wide string",
            "Unknown",
        };

        int toIndex = static_cast<int>(vTo);
        int fromIndex = static_cast<int>(type);

        if (toIndex < 0 || toIndex >= sizeof(typeNames)
            || fromIndex < 0 || fromIndex >= sizeof(typeNames))
        {
            throw std::runtime_error("Bad value type");
        }
        else
        {
            throw ValueError(typeNames[fromIndex], typeNames[toIndex]);
        }
    }

    ValueType       type;

    union {
        int     vInt;
        float   vFloat;
    }               intValue;
    // cannot union types with copy-constructor
    barray          *vBarray;
    std::string     *vString;
    std::wstring    *vWstring;
};

class Item
{
public:
    Item()
        : names_(new std::vector< std::string >())
    {}
    Item(const Item & r)
    {
        *this = r;
    }
    Item(Item && r)
    {
        *this = r;
    }
    Item & operator=(Item && r)
    {
        names_ = r.names_;
        values_.swap(r.values_);
        return *this;
    }
    Item & operator=(const Item & r)
    {
        names_ = r.names_;
        values_ = r.values_;
        return *this;
    }

    const Value & operator[](const char *name) const
    {
        // stupid /analyze
        std::vector< std::string > *names = names_.get();

        for (size_t i = 0; i < names->size(); ++i)
        {
            if (_stricmp(name, (*names)[i].c_str()) == 0)
            {
                return values_[i];
            }
        }

        // lookup for name

        std::string itemName;

        for (size_t i = 0; i < names->size(); ++i)
        {
            if (_stricmp("name", (*names)[i].c_str()) == 0)
            {
                itemName = values_[i];
            }
        }

        throw ItemValueNotFound(itemName, name);
    }

private:
    //typedef std::multimap<std::string, Value>    Values;
    typedef std::shared_ptr< std::vector< std::string > >   Names;
    typedef std::vector< Value >                            Values;

    friend class ItemListCollection;

    // names should correspond to values
    Names       names_;
    Values      values_;
};


class ItemList
    : public std::map<unsigned, Item>
{
public:
    bool get(unsigned id, Item & item) const
    {
        ItemList::const_iterator it = find(id);
        if (it != end())
        {
            item = it->second;
            return true;
        }
        return false;
    }

    std::set<unsigned> allIds() const
    {
        std::set<unsigned> result;
        for (std::map<unsigned, Item>::const_iterator it = begin(); it != end(); ++it)
        {
            result.insert(it->first);
        }
        return result;
    }

    std::string     name;

private:
    friend class ItemListCollection;

    //

    bool add(const Item & item)
    {
        const Value & val = item["ID"];
        if (val)
        {
            unsigned id = val;
            operator[](id) = item;
            return true;
        }
        return false;
    }
};

class ItemListCollection
{
    // typical usage:
    //
    // ItemListCollection lc;
    // lc.load(...)
    // ItemListCollection someItems = lc["npc"] + lc["quests"]
    //

public:

    ErrorState load( const std::wstring & dataFile
                   , const std::wstring & configFile);

    // returns a new collection containing lists with given name,
    // or empty collection if list wasn't found
    ItemListCollection operator[](const std::string & listName) const;
    ItemListCollection operator[](const char * listName) const;

    std::shared_ptr<ItemList> list(const char * listName) const;
    std::set<unsigned> allIds(const char * listName) const;

    //ItemListCollection operator[](size_t index);

    // returns collection containing both of list sets (adds right collection)
    ItemListCollection operator+(const ItemListCollection & r);

    bool getItem(unsigned id, Item & item) const;
    //const Item & getItem(const char *listName, unsigned id);

	auto begin() const
	{
		return itemLists_.begin();
	}
	auto end() const
	{
		return itemLists_.end();
	}

private:
    Value getValue(FileReader &fr, std::pair<Value::ValueType, unsigned /*width*/> type);

    typedef std::vector< std::shared_ptr<ItemList> >  ItemLists;

    ItemLists   itemLists_;
};



} // namespace

#endif