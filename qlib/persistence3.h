#ifndef persistence3_h
#define persistence3_h

// #include <QJsonDocument>
// #include <QJsonObject>
// #include <QJsonValue>
// #include <QJsonArray>


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

//
// Value  = Object | Array | string, number, etc
//          Object = name : value
// 

class JsonValue : private QJsonObject
{
public:
    // direct mapping from Value to type
//     template<class T>
//     struct NoConversion
//     {
//         JsonValue convert(const T & v) const
//         {
//             return JsonValue(v, NULL);
//         }
// 
//         T convertIn(const JsonValue & v) const
//         {
//             return static_cast<T>(v);
//         }
//     };

    template<typename T>
    static T typeConverter(const QJsonValue & v)
    {
        T r; toType(v, r); return r;
    }
    template<typename T>
    static QJsonValue typeConverter(const T & t)
    {
        return fromType(t);
    }

    // mapping from Value to type with constructor / operator JsonValue
    template<typename T>
    static T directConverter(const QJsonValue & v)
    {
        return T(v);
    }
    template<typename T>
    static QJsonValue directConverter(const T & v)
    {
        return QJsonValue(v);
    }

    // object converter
    template<typename T>
    static T objectConverter(const QJsonValue & v)
    {
        if (!v.isObject()) { return T();}
        return T(JsonValue(v.toObject()));
    }
    template<typename T>
    static QJsonValue objectConverter(const T & v)
    {
        JsonValue json(v);
        return QJsonValue(json);
    }


//     // mapping from Value to type pointer with constructor / operator JsonValue
//     template<class T>
//     struct DirectPtrConversion
//     {
//         JsonValue convert(T * v) const
//         {
//             return static_cast<JsonValue>(*v);
//         }
// 
//         T * convertIn(const JsonValue & v) const
//         {
//             return new T(v);
//         }
//     };
// 
//     // mapping from Value reference to type with constructor / operator JsonValue
//     template<class T>
//     struct TransparentConversion
//     {
//         JsonValue & convert(T & v) const
//         {
//             return static_cast<JsonValue &>(v);
//         }
//     };
// 
//     // mapping from Value reference to type pointer with constructor / operator JsonValue
//     template<class T>
//     struct TransparentPtrConversion
//     {
//         JsonValue & convert(T * v) const
//         {
//             return static_cast<JsonValue &>(*v);
//         }
// 
//         T * convertIn(const JsonValue & v)
//         {
//             return new T(v);
//         }
//     };

    //
    JsonValue()
    {}
    JsonValue(const QJsonObject & object)
        : QJsonObject(object)
    {}

    // Get

    template<typename T>
    bool get(const wchar_t * name, T & value, const T & defaultVal = T()) const
    {
        return get(QString::fromWCharArray(name), value, defaultVal);
    }

    template<typename T>
    bool get(const QString & name, T & value, const T & defaultVal = T()) const
    {
        QJsonObject::const_iterator it = find(name);
        if (it != end()
            && !it->isObject()
            && !it->isArray()
            && !it->isNull())
        {
            toType(*it, value);
            return true;
        }

        value = defaultVal;
        return false;
    }

    template< template<typename, typename ...> class Array, typename Value, typename ... A>
    bool getArray(const QString & name, typename Array<Value, A ...> & array
                  , Value (*converter)(const QJsonValue &) = objectConverter) const
//                  , std::function<Value (const QJsonValue &)> converter = toTypeConverter<Value>) const
    {
        array.clear();

        QJsonObject::const_iterator it = find(name);
        if (it != end()
            && it->isArray())
        {
            const QJsonArray arr = it->toArray();
            for (int i = 0; i < arr.size(); i++)
            {
                array.push_back(converter(arr[i]));
            }
            return true;
        }

        return false;
    }

//     bool getArray(const QString & name, std::vector<T> & array, T(*converter)(const QJsonValue &)) const
//     {
//         array.clear();
// 
//         QJsonObject::const_iterator it = find(name);
//         if (it != end()
//             && it->isArray())
//         {
//             const QJsonArray arr = it->toArray();
//             for (int i = 0; i < arr.size(); i++)
//             {
//                 array.push_back(converter(arr[i]));
//             }
//             return true;
//         }
// 
//         return false;
//     }

    // get subObject

    JsonValue get(const wchar_t * name)
    {
        return get(QString::fromWCharArray(name));
    }

    JsonValue get(const QString & name)
    {
        QJsonObject::const_iterator it = find(name);
        if (it != end()
            && it->isObject())
        {
            return JsonValue(it->toObject());
        }

        return JsonValue();
    }

    // Set

    template<typename T>
    void set(const wchar_t * name, const T & value)
    {
        return set(QString::fromWCharArray(name), value);
    }

    template<typename T>
    void set(const QString & name, const T & value)
    {
        (*this)[name] = fromType(value);
    }

    template< template<typename, typename ...> class Array, typename Value, typename ... A>
    void setArray( const QString & name, const typename Array<Value, A ...> & array
                 , QJsonValue (*converter)(const Value &) = objectConverter )
//                 , typename Conv converter = objectConverter )
//                 , std::function<QJsonValue (const Value &)> converter = objectConverter<Value> )
    {
        QJsonArray arr;
        for (unsigned i = 0; i < unsigned(array.size()); i++)
        {
            arr.push_back(converter(array[i]));
        }

        (*this)[name] = arr;
    }

    bool saveTo(const QString & filename);
    bool loadFrom(const QString & filename);
    //std::wstring string() const;

private:
    static void toType(const QJsonValue & v, unsigned char & t) { t = (unsigned char)v.toInt(); }
    static void toType(const QJsonValue & v, unsigned int & t)  { t = (unsigned int)v.toDouble(); }
    static void toType(const QJsonValue & v, int & t)           { t = v.toInt(); }
    static void toType(const QJsonValue & v, bool & t)          { t = v.toBool(); }
    static void toType(const QJsonValue & v, double & t)        { t = v.toDouble(); }
    static void toType(const QJsonValue & v, QString & t)       { t = v.toString(); }
    static void toType(const QJsonValue & v, std::wstring & t)  { t = v.toString().toStdWString(); }

    static QJsonValue fromType(int t)                           { return QJsonValue(t); }
    static QJsonValue fromType(unsigned int t)                  { return QJsonValue(double(t)); }
    static QJsonValue fromType(bool t)                          { return QJsonValue(t); }
    static QJsonValue fromType(double t)                        { return QJsonValue(t); }
    static QJsonValue fromType(const QString & t)               { return QJsonValue(t); }
    static QJsonValue fromType(const std::wstring & t)          { return QJsonValue(QString::fromStdWString(t)); }
};


class Persistence
{
public:
    Persistence()
        : isRestored_(false)
    {}
    virtual ~Persistence();

    bool open(const QString & filename, bool & restored);
    bool save();

    JsonValue root()
    {
        return JsonValue(json_.object());
    }

private:
//     friend class JsonValue;

    Persistence(const Persistence &);
    Persistence &operator=(const Persistence &);

    QString         fileName_;
    QByteArray      fileCache_;
    bool            isRestored_;    // нужно ли сохранять при совпадении кешей для обновления файлов
    QJsonDocument   json_;
};


#endif