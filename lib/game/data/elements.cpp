
#include "stdafx.h"

#include "utils.h"
#include "game/data/elements.h"
#include "game/data/filereader.h"

namespace elements
{

// ListConfig

class ListConfig
{
public:
    typedef elements::Value::ValueType  Type;

    bool setTypes(const std::vector<std::string> & types);

    std::string                                         name;
    std::vector<std::string>                            names;
    std::vector<std::pair<Type, unsigned /*width*/>>    types;
};

bool ListConfig::setTypes(const std::vector<std::string> & typeString)
{
    types.clear();

    for (std::vector<std::string>::const_iterator it = typeString.begin();
        it != typeString.end(); ++it)
    {
        if (_stricmp(it->c_str(), "int32") == 0)
        {
            types.push_back(std::make_pair(elements::Value::Int32, 0));
        }
        else if (_stricmp(it->c_str(), "float") == 0)
        {
            types.push_back(std::make_pair(elements::Value::Float, 0));
        }
        else if (_strnicmp(it->c_str(), "string:", sizeof("string:") - 1) == 0)
        {
            unsigned width = strtoul(it->c_str() + sizeof("string:") - 1, NULL, 10);
            if (width == 0 || width == ULONG_MAX)
            {
                return false;
            }
            types.push_back(std::make_pair(elements::Value::String, width));
        }
        else if (_strnicmp(it->c_str(), "wstring:", sizeof("wstring:") - 1) == 0)
        {
            unsigned long width = strtoul(it->c_str() + sizeof("wstring:") - 1, NULL, 10);
            if (width == 0 || width == ULONG_MAX)
            {
                return false;
            }
            types.push_back(std::make_pair(elements::Value::Wstring, width));
        }
        else if (_stricmp(it->c_str(), "sizedarray32") == 0)
        {
            types.push_back(std::make_pair(elements::Value::Barray, 4));
        }
        else if (_stricmp(it->c_str(), "raw") == 0)
        {
            types.push_back(std::make_pair(elements::Value::CountOverride, 0));
        }
        else if (_stricmp(it->c_str(), "npctalk") == 0)
        {
            types.push_back(std::make_pair(elements::Value::NpcTalk, 0));
        }
        /*else if (_strnicmp(it->c_str(), "byte:", sizeof("byte:") - 1) == 0)
        {
            unsigned long width = strtoul(it->c_str() + sizeof("byte:") - 1, NULL, 10);
            if (width == 0 || width == ULONG_MAX)
            {
                return false;
            }
            types.push_back(std::make_pair(elements::Value::Barray, width));
        }*/
        else
        {
            assert(0);
            return false;
        }
    }

    return true;
}


std::vector<std::string> split(const std::string source, char delim)
{
    std::vector<std::string> result;

    std::string::const_iterator first = source.begin(), last;
    do 
    {
        last = std::find(first, source.end(), delim);
        result.push_back(std::string(first, last));
        first = last;
        if (first != source.end())
        {
            if (*first == delim)
            {
                ++first;
            }
            else
            {
                // something is wrong here
                result.clear();
                break;
            }
        }
    }
    while(first != source.end());

    return result;
}

ErrorState ItemListCollection::load( const std::wstring & dataFile
                                   , const std::wstring & configFile)
{
    ErrorState result;

    //
    // Read element configuration
    //

    std::ifstream config(configFile.c_str());
    if ( ! config)
    {
        result.set(ERR_INVALID_DATA, 0, L"Elements: can't open config file");
        return result;
    }

    std::vector<ListConfig> elementConfig;

    enum state_t
    {
        READ_NAME = 0,
        READ_VARS,
        READ_TYPES,
    } state = READ_NAME;

    ListConfig current;
    unsigned lineCtr = 0;
    std::string line;

    while( ! config.eof())
    {
        getline(config, line);
        lineCtr++;

        if (line.empty())
        {
            continue;
        }

        switch (state)
        {
            case READ_NAME:
                current.name = line;
                state = READ_VARS;
                break;

            case READ_VARS:
                current.names = split(line, ';');
                if (current.names.empty())
                {
                    result.set(ERR_INVALID_DATA, 0, std::wstring(L"Wrong variable list at line ") + std::to_wstring(lineCtr));
                    return result;
                }
                state = READ_TYPES;
                break;

            case READ_TYPES:
            {
                std::vector< std::string > types = split(line, ';');
                if (types.empty() || ! current.setTypes(types))
                {
                    result.set(ERR_INVALID_DATA, 0, std::wstring(L"Wrong variable list at line ") + std::to_wstring(lineCtr));
                    return result;
                }

                if (current.names.size() != current.types.size())
                {
                    result.set(ERR_INVALID_DATA, 0, std::wstring(L"Names and types size mismatch at line ") + std::to_wstring(lineCtr));
                    return result;
                }

                elementConfig.push_back(current);

                current.name.clear();
                current.types.clear();
                current.names.clear();

                state = READ_NAME;
                break;
            }

            default:
                assert(0);
                state = READ_NAME;
        }
    }

    if (elementConfig.empty())
    {
        result.set(ERR_INVALID_DATA, 0, L"Elements configuration error");
        return result;
    }

    //
    // Read element data
    //

    FileReader fr;
    if ( ! fr.open(dataFile))
    {
        result.set(ERR_INVALID_DATA, 0, std::wstring(L"Error opening element file '") + dataFile + L"'");
        return result;
    }

    elVersion_ = fr.readWord();
    const unsigned elSign = fr.readWord();
    const unsigned elTs = fr.readDword();

    for (size_t listNumber = 0; listNumber < elementConfig.size() && fr; ++listNumber)
    {
        ListConfig & listConfig = elementConfig[listNumber];

        std::shared_ptr<ItemList> list(new ItemList());
        list->name = listConfig.name;

        std::shared_ptr< std::vector< std::string > > itemNames(new std::vector< std::string >(listConfig.names));
        itemNames->erase(std::remove_if(itemNames->begin(), itemNames->end(), [](const auto & s) { return s == "?"; })
                        , itemNames->end());

        if (listConfig.types[0].first == Value::NpcTalk)
        {
            unsigned dialogCount = fr.readDword();
            for (unsigned d = 0; d < dialogCount; ++d)
            {
//                 Dialogs[d]->DialogID = br->ReadInt32();
//                 Dialogs[d]->DialogName = br->ReadBytes(128);
                fr.move(4 + 128);

                unsigned questionCount = fr.readDword();
                for (unsigned q = 0; q < questionCount; ++q)
                {
//                     Dialogs[d]->Questions[q]->QuestionID = br->ReadInt32();
//                     Dialogs[d]->Questions[q]->Control = br->ReadInt32();
                    fr.move(4 + 4);
                    unsigned qTextLength = fr.readDword();
//                     Dialogs[d]->Questions[q]->QuestionText = br->ReadBytes(2*Dialogs[d]->Questions[q]->QuestionTextLength);
                    fr.move(2 * qTextLength);

                    unsigned choiceCount = fr.readDword();
                    for (unsigned c = 0; c < choiceCount; ++c)
                    {
//                         Dialogs[d]->Questions[q]->Choices[c]->Control = br->ReadInt32();
//                         Dialogs[d]->Questions[q]->Choices[c]->ChoiceText = br->ReadBytes(132);
                        fr.move(4 + 132);
                    }
                }
            }

            continue;
        }

        unsigned itemCount = 1;

        if (listConfig.types[0].first != Value::CountOverride)
        {
            if (elVersion_ >= 191 )
            {
                const unsigned listType = fr.readDword();
            }
            itemCount = fr.readDword();
            if (elVersion_ >= 191)
            {
                const unsigned elementSize = fr.readDword();
            }
        }
        else
        {
            listConfig.types.erase(listConfig.types.begin());
        }

        for (unsigned itemNumber = 0; itemNumber < itemCount && fr; ++itemNumber)
        {
            elements::Item  item;
            item.names_ = itemNames;

            item.values_.resize(item.names_->size());
            unsigned valueIndex = 0;

            for (size_t field = 0; field < listConfig.types.size(); ++field)
            {
                elements::Value value = getValue(fr, listConfig.types[field]);

                if (listConfig.names[field] != "?")
                {
                    assert(valueIndex < item.values_.size());
                    item.values_[valueIndex] = std::move(value);
                    valueIndex++;
                }
            }

            if (!item.values_.empty())
            {
                if ( ! list->add(item))
                {
                    result.set(ERR_INVALID_DATA, 0, std::wstring(L"Error adding element ") + std::to_wstring(itemNumber) + L" to list" + std::to_wstring(listNumber));
                    return result;
                }
            }
        }

        itemLists_.push_back(list);
    }

    if ( ! fr)
    {
        result.set(ERR_INVALID_DATA, 0, std::wstring(L"Error reading element file '") + dataFile + L"',\nmake sure your element config is correct");
        return result;
    }

    return result;
}

elements::Value ItemListCollection::getValue(FileReader &fr, std::pair<Value::ValueType, unsigned /*width*/> type)
{
    switch (type.first)
    {
        case ListConfig::Type::Int32:
        {
            int v = fr.readDword();
            return Value(v);
        }

        case ListConfig::Type::Float:
        {
            float v = fr.readFloat();
            return Value(v);
        }

        case ListConfig::Type::String:
        {
            std::string v = fr.readString(type.second);
            return Value(v);
        }

        case ListConfig::Type::Wstring:
        {
            std::wstring v = fr.readWstring(type.second / 2);
            return Value(v);
        }

        case ListConfig::Type::Barray:
        {
            int size = -1;
            switch (type.second)
            {
                case 2:
                    size = fr.readWord();
                    break;
                case 4:
                    size = fr.readDword();
                    break;
                default:
                    assert(0);
            }
            if (size > 0)
            {
                barray v = fr.readBytes(size);
                return Value(v);
            }
            return Value();
        }

        default:
            return Value();
    }
}

ItemListCollection ItemListCollection::operator[](const std::string & listName) const
{
    ItemListCollection result;

    for(ItemLists::const_iterator it = itemLists_.begin();
        it != itemLists_.end(); ++it)
    {
        if ((*it)->name == listName)
        {
            result.itemLists_.push_back(*it);
        }
    }

    result.elVersion_ = elVersion_;

    return result;
}

ItemListCollection ItemListCollection::operator[](const char * listName) const
{
    return operator[](std::string(listName));
}

std::shared_ptr<ItemList> ItemListCollection::list(const char * listName) const
{
    for(ItemLists::const_iterator it = itemLists_.begin();
        it != itemLists_.end(); ++it)
    {
        if ((*it)->name == listName)
        {
            return *it;
        }
    }

    return std::shared_ptr<ItemList>();
}

std::set<unsigned> ItemListCollection::allIds(const char * listName) const
{
    std::shared_ptr<elements::ItemList> list = ItemListCollection::list(listName);
    if (list)
    {
        return list->allIds();
    }
    return std::set<unsigned>();
}

ItemListCollection ItemListCollection::operator+(const ItemListCollection & r)
{
    std::copy(r.itemLists_.begin(), r.itemLists_.end(), std::back_inserter(itemLists_));
    elVersion_ = r.elVersion_;
    return *this;
}

bool ItemListCollection::getItem(unsigned id, Item & item) const
{
    for(ItemLists::const_iterator it = itemLists_.begin();
        it != itemLists_.end(); ++it)
    {
        if ((*it)->get(id, item))
        {
            return true;
        }
    }
    return false;
}

} // namespace
