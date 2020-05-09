#ifndef persistence2_h
#define persistence2_h

#include "common.h"
#include "json_spirit.h"

/*
class tc
{
public:
    tc(const std::wstring & str)
        : variable_(str)
    {}

    tc(const JsonValue & cfg)
        : cfg_(cfg)
    {
        cfg_.get(L"string", variable_);
        variable_ += L'.';
        cfg_.set(L"string", variable_);
    }

    operator JsonValue &()
    {
        cfg_.set(L"string", variable_);
        return cfg_;
    }

private:
    JsonValue       cfg_;
    std::wstring    variable_;
};

    Persistence config;
    if (config.open(L"test.jcfg"))
    {
        JsonValue root = config.root();
        JsonValue cfg1 = root.get(L"section1");
        cfg1.set(L"number", 50);

        JsonValue tcs = cfg1.get(L"tcs");
        //tc tc1(L"default");
        // tcs.get(L"TC1", tc1); isn't pod type
        tc tc1(tcs.get(L"TC1"));

        std::vector<tc> vtc1;
        tcs.get(L"TCV1", vtc1);
        vtc1.push_back(tc(L"10"));
        vtc1.push_back(tc(L"11"));
        tcs.set(L"TCV1", vtc1);

        std::vector<tc *> vtc2;
        tcs.get(L"TCV2", vtc2);
        vtc2.push_back(new tc(L"20"));
        vtc2.push_back(new tc(L"21"));
        tcs.set(L"TCV2", vtc2);
    }

    config.save();
*/

namespace json = json_spirit;

namespace json_spirit
{
    template< class T>
    class NoConversion;

    template< class T>
    class DirectConversion;
}

//
// Value  = Object | Array | string, number, etc
//          Object = name : value
// 

typedef json::wmArray   JsonArray;
class Persistence;

class JsonValue
    : public json::wmValue
{
public:
    JsonValue()
        : json::wmValue(json::wmObject())  // object by default
        , container_(NULL)
    {}

    JsonValue(const json::wmValue & value, Persistence * container)
        : json::wmValue(value)
        , container_(container)
    {}

    // Get

    template< typename T >
    bool get(const WCHAR *name, T &value, const T & defaultVal = T()) const
    {
        if ( ! isObject())
        {
            value = defaultVal;
            return false;
        }

        const json::wmObject & obj = *this;
        json::wmObject::const_iterator it = obj.find(name);
        if (it != obj.end()
            && (it->second.type() != json::obj_type 
            && it->second.type() != json::array_type
            && it->second.type() != json::null_type))
        {
            value = static_cast<T>(it->second);
            return true;
        }
        else
        {
            value = defaultVal;
            return false;
        }
    }

    template< typename T, class Converter  >
    bool get(const WCHAR *name, std::vector<T> & array, const Converter & conv) const
    {
        array.clear();

        if ( ! isObject())
        {
            return false;
        }

        const json::wmObject & obj = *this;
        json::wmObject::const_iterator it = obj.find(name);
        if (it != obj.end()
            && it->second.type() == json::array_type)
        {
            const json::wmArray & arr = it->second;
            for (size_t i = 0; i < arr.size(); i++)
            {
                array.push_back(conv.convertIn(JsonValue(arr[i], container_)));
            }
            return true;
        }
        else
        {
            return false;
        }
    }

    //

    bool contains(const WCHAR *name)
    {
        if ( ! isObject())
        {
            return false;
        }

        json::wmObject & obj = *this;
        json::wmObject::const_iterator it = obj.find(name);
        return it != obj.end();
    }

    JsonValue get(const WCHAR *name)
    {
        json::wmObject & obj = *this;
        json::wmObject::const_iterator it = obj.find(name);
        if (it != obj.end())
        {
            return JsonValue(it->second, container_);
        }
        else
        {
            json::wmValue & nv(obj[name] = json::wmObject());
            return JsonValue(nv, container_);
        }
    }

    // Set

    template< typename T >
    void set(const WCHAR *name, const T &value)
    {
        if ( ! isObject())
        {
            *static_cast<json::wmValue*>(this) = json::wmValue(json::wmObject());
        }
        json::wmObject & obj = *this;
        obj[name] = json::wmValue(value);
    }

    template< typename T, class Converter >
    void set(const WCHAR *name, const std::vector<T> &array, const Converter & conv)
    {
        json::wmArray arr;
        for (size_t i = 0; i < array.size(); i++)
        {
            const JsonValue & v = conv.convert(array[i]);
            if (v.container_ == NULL)
            {
                v.container_ = container_;
            }
            arr.push_back(v);
        }

        if ( ! isObject())
        {
            *static_cast<json::wmValue*>(this) = json::wmObject();
        }
        json::wmObject & obj = *this;
        obj[name] = arr;
    }

    bool isObject() const
    {
        return type() == json::obj_type;
    }

    void save();
    void saveTo(const std::wstring & filename, bool doBackup = true);
    bool loadFrom(const std::wstring & filename);
    bool read(const std::wstring & string);
    std::wstring string() const;

private:
    mutable Persistence * container_;   // weak
};


class Persistence
{
public:
    Persistence()
        : isRestored_(false)
    {}

    virtual ~Persistence();

    bool open(const std::wstring & filename, bool & restored);
    void save();

    JsonValue root()
    {
        return JsonValue(jsonRoot_, this);
    }

private:
    friend class JsonValue;

    Persistence(const Persistence &);
    Persistence &operator=(const Persistence &);

    std::wstring    fileName_;
    std::wstring    fileCache_;
    bool            isRestored_;
    json::wmValue   jsonRoot_;
};

namespace json_spirit
{
    // direct mapping from wmValue to type
    template< class T>
    class NoConversion
    {
    public:
        JsonValue convert(const T & v) const
        {
            return JsonValue(v, NULL);
        }

        T convertIn(const JsonValue & v) const
        {
            return static_cast<T>(v);
        }
    };

    // mapping from wmValue to type with constructor / operator JsonValue
    template< class T>
    class DirectConversion
    {
    public:
        JsonValue convert(const T & v) const
        {
            return static_cast<JsonValue>(v);
        }

        T convertIn(const JsonValue & v) const
        {
            return T(v);
        }
    };

    // mapping from wmValue to type pointer with constructor / operator JsonValue
    template< class T>
    class DirectPtrConversion
    {
    public:
        JsonValue convert(T * v) const
        {
            return static_cast<JsonValue>(*v);
        }

        T * convertIn(const JsonValue & v) const
        {
            return new T(v);
        }
    };

    // mapping from wmValue reference to type with constructor / operator JsonValue
    template< class T>
    class TransparentConversion
    {
    public:
        JsonValue & convert(T & v) const
        {
            return static_cast<JsonValue &>(v);
        }
    };

    // mapping from wmValue reference to type pointer with constructor / operator JsonValue
    template< class T>
    class TransparentPtrConversion
    {
    public:
        JsonValue & convert(T * v) const
        {
            return static_cast<JsonValue &>(*v);
        }

        T * convertIn(const JsonValue & v)
        {
            return new T(v);
        }
    };
}



#endif