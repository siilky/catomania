#include "stdafx.h"

#include "types.h"
#include "utils.h"
#include "json_spirit.h"
#include "netdata/serialize.h"
#include "game/data/elements.h"
#include "game/data/itemprops.h"
#include "game/data/itemeffects.h"


std::wstring ItemEffects::toString(const TypesMap & types, const FormatsMap & formats) const
{
    wchar_t buf[128];
    TypesMap::const_iterator iType = types.find(effectId());
    if (iType != types.end())
    {
        FormatsMap::const_iterator iFormat = formats.find(iType->second);
        if (iFormat != formats.end())
        {
            const std::wstring & fmt = iFormat->second;

            if (fmt.find(L"%.2f") != std::wstring::npos
                || fmt.find(L"%.1f") != std::wstring::npos
                || fmt.find(L"%+.2f") != std::wstring::npos
                || fmt.find(L"%+.0f") != std::wstring::npos
                || fmt.find(L"%.0f") != std::wstring::npos)
            {
                float fVal = *(float*)(&values[0]);
                if (fmt.find(L"%%") != std::wstring::npos)
                {
                    fVal *= 100;
                }

                _snwprintf_s(buf, _TRUNCATE, fmt.c_str(), fVal);
            }
            else
            {
                _snwprintf_s(buf, _TRUNCATE, fmt.c_str(), values[0]);
            }
            return buf;
        }
    }

    _snwprintf_s(buf, _TRUNCATE, L"(?) %02X: %u (%u)", effectType(), id, values[0]);
    return buf;
}

int ItemEffects::toClass(const TypesMap & types) const
{
    TypesMap::const_iterator iType = types.find(effectId());
    if (iType != types.end())
    {
        return iType->second;
    }
    return -1;
}

bool ItemEffects::operator==(const ItemEffects & r) const
{
    bool isEqual = id == r.id;
    int args = argsCount();
    while (--args >= 0 && isEqual)
    {
        isEqual &= values[args] == r.values[args];
    }
    return isEqual;
}

//

static bool loadItemPropDesc( const std::wstring & fileName
                            , std::vector<std::wstring> & result);
static bool loadItemPropTypes( const std::wstring & fileName
                             , TypesMap           & result);
static bool loadItemPropMap( const std::wstring               & fileName
                           , const std::vector<std::wstring>  & formats
                           , FormatsMap                       & result);

ItemPropsDecoder::ItemPropsDecoder( const wchar_t *propDesc
                                  , const wchar_t *propTypes
                                  , const wchar_t *propMap)
{
    std::vector<std::wstring> descriptions;
    if ( ! loadItemPropDesc(propDesc, descriptions)
        || ! loadItemPropTypes(propTypes, effectTypes_)
        || ! loadItemPropMap(propMap, descriptions, effectFormats_))
    {
        effectTypes_.clear();
        effectFormats_.clear();
        isLoaded_ = false;
    }
    else
    {
        isLoaded_ = true;
    }
};

void ItemPropsDecoder::setElements(const elements::ItemListCollection & items)
{
    items_ = items;
}

std::wstring ItemPropsDecoder::decode(const ItemProps * props)
{
    const ArmorPropsImpl *armor = dynamic_cast<const ArmorPropsImpl *>(props);
    if (armor != NULL)
    {
        return decode(armor);
    }
    else
    {
        const WeaponProps *weapon = dynamic_cast<const WeaponProps *>(props);
        if (weapon != NULL)
        {
            return decode(weapon);
        }
    }

    return L"";
}

std::wstring ItemPropsDecoder::decode(const ArmorPropsImpl * armor)
{
    json_spirit::wmObject js;

    try
    {
        js[L"SlotFlags"] = armor->slotFlags;
        decodeSlots(js, armor->sockets);

        decodeEffects(js, armor->effects);

        if ( ! armor->wearable.creator.empty())
        {
            js[L"Creator"] = armor->wearable.creator;
        }
    }
    catch(...)
    {
        return L"";
    }

    return json_spirit::write(js);
}

std::wstring ItemPropsDecoder::decode(const WeaponProps * weapon)
{
    json_spirit::wmObject js;

    try
    {
        js[L"SlotFlags"] = weapon->slotFlags;
        decodeSlots(js, weapon->sockets);

        decodeEffects(js, weapon->effects);

        if ( ! weapon->wearable.creator.empty())
        {
            js[L"Creator"] = weapon->wearable.creator;
        }
    }
    catch(...)
    {
        return L"";
    }

    return json_spirit::write(js);
}

void ItemPropsDecoder::decodeEffects(json_spirit::wmObject & js, const std::vector<ItemEffects> & effects)
{
    if (effects.empty())
    {
        return;
    }

    json_spirit::wmArray    simpleEffects;
    json_spirit::wmArray    doubleEffects;
    //json_spirit::wmArray    slotEffects;

    for (size_t nEffect = 0; nEffect < effects.size(); nEffect++)
    {
        switch (effects[nEffect].argsCount())
        {
            case 0: // unknown parameters
            case 1:
            {
                simpleEffects.push_back(effects[nEffect].toString(effectTypes_, effectFormats_));
                break;
            }

            case 2:
            {
                json_spirit::wmObject effect;
                effect[L"EffectId"] = effects[nEffect].toClass(effectTypes_);
                effect[L"Effect"] = effects[nEffect].toString(effectTypes_, effectFormats_);
                effect[L"Level"] = int(effects[nEffect].values[1]);
                doubleEffects.push_back(effect);
                break;
            }

//         case ItemEffects::SlotEffect:
//             //slotEffects.push_back(get from elements?);
//             break;

        default:
            assert(0);
            break;
        }
    }

    js[L"Effects"] = simpleEffects;
    js[L"EffectsExt"] = doubleEffects;
    //js[L"SlotEffects"] = slotEffects;
}

void ItemPropsDecoder::decodeSlots(json_spirit::wmObject & js, const std::vector<DWORD> & slots)
{
    json_spirit::wmArray jsArray;

    int nSlots = slots.size();
    js[L"Slots"] = nSlots;

    if (nSlots != 0)
    {
        for(int i = 0; i < nSlots; i++)
        {
            elements::Item item;
            if (slots[i] != 0)
            {
                if (items_.getItem(slots[i], item))
                {
                    jsArray.push_back(std::wstring(item["Name"]));
                }
                else
                {
                    jsArray.push_back(int(slots[i]));
                }
            }
        }
    }

    js[L"UsedSlots"] = jsArray;
}

//

static bool loadItemPropDesc( const std::wstring & fileName
                            , std::vector<std::wstring> & result)
{
    result.clear();

    std::ifstream file(fileName.c_str(), std::ios::binary | std::ios::in);

    if ( ! file)
    {
        return false;
    }

    // file goes in unicode
    file.seekg(0, std::ios::end);
    int size = (int)file.tellg();
    std::shared_ptr<char> memblock(new char [size]);
    file.seekg(0, std::ios::beg);
    file.read(memblock.get(), size);
    file.close();

    std::wistringstream ssw(std::wstring((wchar_t *)memblock.get(), size / sizeof(wchar_t)));

    while ( ! ssw.eof())
    {
        std::wstring line;
        getline(ssw, line);

        stringTrim(line);

        if (file.bad())
        {
            return false;
        }

        if (line.empty())
        {
            continue;
        }

        if (line[0] == L'#')
        {
            continue;
        }

        if (line.size() > 1 && line.find_first_not_of(L'/') > 1)
        {
            continue;
        }

        size_t start = line.find(L'"');
        if (start != std::wstring::npos)
        {
            size_t end = line.find(L'"', start + 1);
            if (end != std::wstring::npos)
            {
                result.push_back(line.substr(start + 1, end - start - 1));
            }
        }
    }

    return true;
}

static bool loadItemPropTypes( const std::wstring & fileName
                             , TypesMap           & result)
{
    result.clear();

    std::wifstream file(fileName.c_str(), std::ios_base::in | std::ios_base::binary);

    if ( ! file)
    {
        return false;
    }

    // file is pretty utf-8 but we don't care at the moment
    // ...

    enum Stage
    {
        outerLookup,    // 'type'
        startLookup,    // '{'
        innerLookup,    // '}'
    } stage = outerLookup;

    int currentType = -1;

    while ( ! file.eof())
    {
        std::wstring line;
        getline(file, line);

        stringTrim(line);

        if (file.bad())
        {
            return false;
        }

        if (line.empty())
        {
            continue;
        }

        if (line[0] == L'#')
        {
            continue;
        }

        if (line.size() > 1 && line.find_first_not_of(L'/') > 1)
        {
            continue;
        }

        switch (stage)
        {
            case outerLookup:
                // start from 'type:' word
                if (line.substr(0, 5) == L"type:")
                {
                    std::wstring sType = line.substr(5);
                    stringTrim(sType);
                    currentType = _wtoi(sType.c_str());
                    stage = startLookup;
                }
                break;

            case startLookup:
                if (line.find(L"{") != std::string::npos)
                {
                    stage = innerLookup;
                }
                break;

            case innerLookup:
                if (line.find(L"}") != std::string::npos)
                {
                    currentType = -1;
                    stage = outerLookup;
                }
                else
                {
                    size_t pos = 0, next = 0;
                    size_t end = line.size();
                    while (next != std::wstring::npos
                            && pos < end)
                    {
                        next = line.find(L',', pos);

                        std::wstring sub = line.substr(pos, next - pos);
                        stringTrim(sub);
                        if ( ! sub.empty())
                        {
                            int propertyId = _wtoi(sub.c_str());
                            if (result.find(propertyId) != result.end())
                            {
                                assert(0);
                                return false;
                            }
                            result[propertyId] = currentType;
                        }

                        pos = next + 1;
                    }
                }
                break;
        }
    }

    return true;
}

static bool loadItemPropMap( const std::wstring               & fileName
                           , const std::vector<std::wstring>  & formats
                           , FormatsMap                       & result)
{
    result.clear();

    std::ifstream file(fileName.c_str(), std::ios::binary | std::ios::in);

    if ( ! file)
    {
        return false;
    }

    // file goes in unicode
    file.seekg(0, std::ios::end);
    int size = (int)file.tellg();
    std::shared_ptr<char> memblock(new char [size]);
    file.seekg(0, std::ios::beg);
    file.read(memblock.get(), size);
    file.close();

    std::wistringstream ssw(std::wstring((wchar_t *)memblock.get(), size / sizeof(wchar_t)));

    while ( ! ssw.eof())
    {
        std::wstring line;
        getline(ssw, line);

        stringTrim(line);

        if (file.bad())
        {
            return false;
        }

        if (line.empty())
        {
            continue;
        }

        if (line[0] == L'#')
        {
            continue;
        }

        if (line.size() > 1 && line.find_first_not_of(L'/') > 1)
        {
            continue;
        }

        // look for 'type:'
        size_t colon = line.find(L":");
        if (colon == std::string::npos)
        {
            continue;;
        }

        std::wstring sType = line.substr(0, colon);
        stringTrim(sType);
        int currentType = _wtoi(sType.c_str());

        if (result.find(currentType) != result.end())
        {
            assert(0);
            return false;
        }

        size_t comma = line.find(L",", colon + 1);
        std::wstring sFormat;
        if (comma == std::string::npos)
        {
            sFormat = line.substr(colon + 1);
        }
        else
        {
            sFormat = line.substr(colon + 1, comma - colon - 1);
        }

        stringTrim(sFormat);
        int format = _wtoi(sFormat.c_str());
        format --;  // off by one - first line is index 1

        if (size_t(format) > formats.size())
        {
            result[currentType] = L"[No Format]";
            continue;
        }

        std::wstring sF = formats[format];

        if (comma != std::string::npos)
        {
            size_t start = line.find(L'"', comma + 1);
            if (start != std::wstring::npos)
            {
                size_t end = line.find(L'"', start + 1);
                if (end != std::wstring::npos)
                {
                    sF += line.substr(start + 1, end - start - 1);
                }
            }
        }

        result[currentType] = sF;
    }

    return true;
}
