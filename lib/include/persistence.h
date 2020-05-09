// $Id: persistence.h 1674 2018-06-14 11:43:43Z jerry $
//
#ifndef persistence_h
#define persistence_h

#include "common.h"
#include "json_spirit.h"

// #include "boost/program_options.hpp"
// 
// template<class T>
// boost::program_options::typed_value<T, TCHAR> * tvalue(T* v)
// {
//     return new boost::program_options::typed_value<T, TCHAR>(v);
// }
// 
// template<class T>
// boost::program_options::typed_value<T, wchar_t> * tvalue()
// {
//     return tvalue<T>(0);
// }

#error Migrate to persistence 2 
namespace json = json_spirit;

//
// Object = name : value
// Value = Object | Array | string, number, etc
//
// .. this thing should be right one. A stuff it below it should be obsolete
// 

typedef json::wmArray   JsonArray;

class JsonObject
    : private json::wmObject
{
public:
    JsonObject()
    {}

    JsonObject(const json::wmObject & obj)
        : json::wmObject(obj)
    {}

    JsonObject(const json::wmValue& val)
        : json::wmObject(val)
    {}

    operator json::wmValue() const
    {
        return json::wmValue(*this);
    }
    //

    template< typename T >
    bool get(const WCHAR *name, T &value) const
    {
        json::wmObject::const_iterator it = find(name);
        if (it != end()
            && (it->second.type() != json::obj_type 
                && it->second.type() != json::array_type
                && it->second.type() != json::null_type))
        {
            value = it->second.get_value<T>();
            return true;
        }
        return false;
    }

    template< typename T >
    bool get(const WCHAR *name, T &value, const T & defaultVal) const
    {
        json::wmObject::const_iterator it = find(name);
        if (it != end()
            && (it->second.type() != json::obj_type 
            && it->second.type() != json::array_type
            && it->second.type() != json::null_type))
        {
            value = it->second.get_value<T>();
            return true;
        }
        else
        {
            value = defaultVal;
            return false;
        }
    }

    // get vector of objects
    template< typename T >
    void get(const WCHAR *name, std::vector<T> &array) const
    {
        json::wmObject::const_iterator it = find(name);
        if (it != end() && it->second.type() == json::array_type)
        {
            array.clear();

            const json::wmArray & arr = it->second;
            for (size_t i = 0; i < arr.size(); i++)
            {
                array.push_back(T(arr[i]));
            }
        }
    }

    template< typename T >
    void get(const WCHAR *name, std::vector<T *> &array) const
    {
        json::wmObject::const_iterator it = find(name);
        if (it != end() && it->second.type() == json::array_type)
        {
            array.clear();

            const json::wmArray & arr = it->second;
            for (size_t i = 0; i < arr.size(); i++)
            {
                array.push_back(new T(JsonObject(arr[i])));
            }
        }
    }

    // get jsonArray
    bool get(const WCHAR *key, JsonArray & array) const
    {
        json::wmObject::const_iterator it = find(key);
        if (it != end() && it->second.type() == json::array_type)
        {
            array = it->second;
            return true;
        }

        return false;
    }

    bool get(const WCHAR *name, JsonObject & obj) const
    {
        json::wmObject::const_iterator it = find(name);
        if (it != end()
            && it->second.type() == json::obj_type)
        {
            obj = it->second;
            return true;
        }

        return false;
    }

    // set vector of objects
    template< typename T >
    void set(const WCHAR *name, const std::vector<T*> &array)
    {
        json::wmArray arr;
        for (size_t i = 0; i < array.size(); i++)
        {
            arr.push_back(JsonObject(*(array[i])));
        }
        operator[](name) = arr;
    }

    template< typename T >
    void set(const WCHAR *name, const std::vector<T> &array)
    {
        json::wmArray arr;
        for (size_t i = 0; i < array.size(); i++)
        {
            arr.push_back(array[i]);
        }
        operator[](name) = arr;
    }

    template< typename T >
    void set(const WCHAR *name, const T &value)
    {
        operator[](name) = value;
    }

    void set(const WCHAR *name, JsonArray & arr)
    {
        operator[](name) = arr;
    }
};

class ConfigFile
{
public:
    typedef json::wmValue   Value_type;
    typedef json::wmArray   Array_type;
    typedef json::wmObject  Node_type;

    ConfigFile()
        : isDirty_(false)
    {}

    virtual ~ConfigFile();

    bool open(const tstring &filename);
    void fflush();


    void setValue(Node_type & node, const WCHAR *name, const Value_type & value)
    {
        isDirty_ = true;
        node[name] = value;
    }

    // Get existing or return false
    bool getValue(Node_type & node, const WCHAR *name, Value_type & value)
    {
        Value_type::Object::const_iterator it = node.find(name);
        if (it == node.end())
        {
            return false;
        }

        value = it->second;
        if (value.type() == json::obj_type            // cannot convert object to scalar
            || value.type() == json::null_type)
        {
            return false;
        }

        return true;
    }

    void setArray(Node_type & node, const WCHAR *name, Array_type & array)
    {
        isDirty_ = true;
        node[name] = array;
    }

    bool getArray(Node_type & node, const WCHAR *name, Array_type & array)
    {
        Value_type::Object::const_iterator it = node.find(name);
        if (it == node.end())
        {
            return false;
        }

        Value_type value = it->second;
        if (value.type() != json::array_type)        // cannot convert any to array
        {
            return false;
        }

        array = value;
        return true;
    }

    // Get existing or create new one
    Node_type & subnode(Node_type & node, const WCHAR *name)
    {
        Value_type & subNode = node[name];
        if (subNode.type() != json::obj_type)
        {
            isDirty_ = true;
            subNode = Node_type();
        }

        return subNode;
    }

    // Get existing or create new node
    Node_type & rootNode()
    {
        if (jsonRoot_.type() != json::obj_type)
        {
            isDirty_ = true;
            jsonRoot_ = json::wmObject();
        }

        return jsonRoot_;
    }

private:
    bool                isDirty_;

    tstring             filename_;
    json::wmValue       jsonRoot_;
};


class ConfigNode 
    : public Singleton<ConfigFile>
{
public:
    ConfigNode(ConfigFile::Node_type & node)
        : node_(node)
    {}

    virtual ~ConfigNode()
    {
        Instance().fflush();
    }

    ConfigNode & operator=(const ConfigNode & r)
    {
        node_ = r.node_;
    }

    static bool SetFileName(const tstring & name)
    {
        Instance().fflush();
        return Instance().open(name);
    }

    void Save()
    {
        Instance().fflush();
    }

    template< typename T >
    T getValue(const WCHAR *key, const T defaultVal) const
    {
        ConfigFile::Value_type  value;
        if (Instance().getValue(node_, key, value))
        {
            return value.get_value<T>();
        }

        return defaultVal;
    }

    template< typename T >
    void WriteValue(const WCHAR *key, T val) const
    {
        ConfigFile::Value_type v(val);
        Instance().setValue(node_, key, v);
    }

    template< typename T >
    void GetArray(const WCHAR *key, std::vector<T> & array) const
    {
        array.clear();
        ConfigFile::Array_type  arr;
        if (Instance().getArray(node_, key, arr))
        {
            std::copy(arr.begin(), arr.end(), back_inserter(array));
        }
    }

    template< typename T >
    void WriteArray(const WCHAR *key, std::vector<T> & array) const
    {
        ConfigFile::Array_type  arr;
        std::copy(array.begin(), array.end(), back_inserter(arr));
        Instance().setArray(node_, key, arr);
    }

    //  String Access

    std::wstring GetString(const WCHAR *key, const std::wstring & defaultVal = std::wstring()) const;
    void WriteString(const WCHAR *key, std::wstring val) const;
    void WriteString(const WCHAR *key, WCHAR *val) const;

    //  Primitive Data Type Access

    int GetInt(const WCHAR *key, int defaultVal = 0) const;
    void WriteInt(const WCHAR *key, int val) const;

    unsigned GetUInt(const WCHAR *key, unsigned defaultVal = 0) const;
    void WriteUInt(const WCHAR *key, UINT val) const;

    bool GetBool(const WCHAR *key, bool defaultVal) const;
    void WriteBool(const WCHAR *key, bool val) const;

    double GetDouble(const WCHAR *key, double defaultVal) const;
    void WriteDouble(const WCHAR *key, double val) const;


private:
    ConfigFile::Node_type & node_;
};


class Persistence : public ConfigNode
{
public:
    Persistence(const WCHAR * section)
        : ConfigNode(Instance().subnode(Instance().rootNode(), section))
    {}

    static ConfigNode getNode(const WCHAR *name)
    {
        return ConfigNode(Instance().subnode(Instance().rootNode(), name));
    }
};


template<typename T>
class ConfigValue
{
public:
    ConfigValue(ConfigNode & node, const std::wstring & name)
        : config_(node)
        , name_(name)
    {
        value_ = config_.getValue<T>(name.c_str(), T());
    }

    ConfigValue & operator=(const T & value)
    {
        if (value != value_)
        {
            value_ = value;
            config_.WriteValue<T>(name_.c_str(), value);
        }
    }


    operator T()
    {
        return value_;
    }

private:
    ConfigValue & operator=(const ConfigValue & r);

    ConfigNode     &config_;
    std::wstring    name_;
    T               value_;
};

#endif