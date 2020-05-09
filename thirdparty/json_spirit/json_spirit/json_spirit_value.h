#ifndef JSON_SPIRIT_VALUE
#define JSON_SPIRIT_VALUE

//          Copyright John W. Wilkinson 2007 - 2009.
// Distributed under the MIT License, see accompanying file LICENSE.txt

// json spirit version 4.03

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <vector>
#include <map>
#include <string>
#include <cassert>
#include <sstream>
#include <stdexcept>
#include <boost/config.hpp> 
#include <boost/cstdint.hpp> 
#include <boost/variant.hpp> 

namespace json_spirit
{
    enum Value_type{ null_type, obj_type, array_type, str_type, bool_type, int64_type, real_type, };

    template< class Config >    // Config determines whether the value uses std::string or std::wstring and
                                // whether JSON Objects are represented as vectors or maps
    class Value_impl
    {
    public:

        typedef Config Config_type;
        typedef typename Config::String_type String_type;
        typedef typename Config::Object_type Object;
        typedef typename Config::Array_type Array;
        typedef typename String_type::const_pointer Const_str_ptr;  // eg const char*

        Value_impl();  // creates null value
        ~Value_impl() {}
        Value_impl( Const_str_ptr      value ); 
        Value_impl( const String_type& value );
        Value_impl( const Object&      value );
        Value_impl( const Array&       value );
        Value_impl( bool               value ); 
        Value_impl( int                value );
        Value_impl( unsigned           value );
        Value_impl( boost::int64_t     value );
        Value_impl( boost::uint64_t    value );
        Value_impl( double             value );
        Value_impl( const Value_impl& other );

        Value_impl& operator=( const Value_impl& lhs );

        bool operator==( const Value_impl& lhs ) const;

        Value_type type() const;
        bool is_null() const;

        operator Object&()                      { return get_obj(); }
        operator Array&()                       { return get_array(); }
        operator const Object&()        const   { return get_obj(); }
        operator const Array&()         const   { return get_array(); }
        operator const String_type&()   const   { return get_str(); }
        operator bool()                 const   { return get_bool(); }
        operator int()                  const   { return int(get_int64()); }
        operator unsigned()             const   { return unsigned(get_int64()); }
        operator boost::int64_t()       const   { return get_int64(); }
        operator boost::uint64_t()      const   { return boost::int64_t(get_int64()); }
        operator double()               const   { return get_double(); }

        template< typename T > T get_value() const;  // example usage: int    i = value.get_value< int >();
                                                     // or             double d = value.get_value< double >();

        // internal
        Object &            get_obj()    /*const*/;
        Array &             get_array()  /*const*/;

    private:
        static const Value_impl null;

        void                check_type( Value_type vtype ) const;
        const Object&       get_obj()    const;
        const Array&        get_array()  const;
        const String_type&  get_str()    const;
        bool                get_bool()   const;
        int64_t             get_int64() const;
        double              get_double() const;

        typedef std::variant< String_type
                              , Object
                              , Array
                              , bool
                              , boost::int64_t
                              , double >                                Variant;
        class Impl
        {
        public:
            Impl()
                : type_(null_type)
            {}

            template<class T>
            Impl(Value_type type, T v)
                : type_(type)
                , v_(Variant(v))
            {
            }

            Value_type  type_;
            Variant     v_;
        };

        std::shared_ptr<Impl> impl_;
    };

    // vector objects
    /*
    template< class Config >
    struct Pair_impl
    {
        typedef typename Config::String_type String_type;
        typedef typename Config::Value_type Value_type;

        Pair_impl( const String_type& name, const Value_type& value );

        bool operator==( const Pair_impl& lhs ) const;

        String_type name_;
        Value_type value_;
    };

    template< class String >
    struct Config_vector
    {
        typedef String String_type;
        typedef Value_impl< Config_vector > Value_type;
        typedef Pair_impl < Config_vector > Pair_type;
        typedef std::vector< Value_type > Array_type;
        typedef std::vector< Pair_type > Object_type;

        static Value_type& add( Object_type& obj, const String_type& name, const Value_type& value )
        {
            obj.push_back( Pair_type( name , value ) );

            return obj.back().value_;
        }
                
        static String_type get_name( const Pair_type& pair )
        {
            return pair.name_;
        }
                
        static Value_type get_value( const Pair_type& pair )
        {
            return pair.value_;
        }
    };

    // typedefs for ASCII

    typedef Config_vector< std::string > Config;

    typedef Config::Value_type  Value;
    typedef Config::Pair_type   Pair;
    typedef Config::Object_type Object;
    typedef Config::Array_type  Array;

    // typedefs for Unicode

#ifndef BOOST_NO_STD_WSTRING

    typedef Config_vector< std::wstring > wConfig;

    typedef wConfig::Value_type  wValue;
    typedef wConfig::Pair_type   wPair;
    typedef wConfig::Object_type wObject;
    typedef wConfig::Array_type  wArray;
#endif
    */
    // map objects

    template< class String >
    struct Config_map
    {
        typedef String                              String_type;
        typedef Value_impl< Config_map >            Value_type;
        typedef std::vector< Value_type >           Array_type;
        typedef std::map< String_type, Value_type > Object_type;
        typedef typename Object_type::value_type    Pair_type;

        static Value_type& add( Object_type& obj, const String_type& name, const Value_type& value )
        {
            return obj[ name ] = value;
        }
                
        static String_type get_name( const Pair_type& pair )
        {
            return pair.first;
        }
                
        static Value_type get_value( const Pair_type& pair )
        {
            return pair.second;
        }
    };

    // typedefs for ASCII
    /*
    typedef Config_map< std::string > mConfig;

    typedef mConfig::Value_type  mValue;
    typedef mConfig::Object_type mObject;
    typedef mConfig::Array_type  mArray;
    */
    // typedefs for Unicode

#ifndef BOOST_NO_STD_WSTRING

    typedef Config_map< std::wstring > wmConfig;

    typedef wmConfig::Value_type  wmValue;
    typedef wmConfig::Object_type wmObject;
    typedef wmConfig::Array_type  wmArray;

#endif

    ///////////////////////////////////////////////////////////////////////////////////////////////
    //
    // implementation

    template< class Config >
    const Value_impl< Config > Value_impl< Config >::null;

    template< class Config >
    Value_impl< Config >::Value_impl()
        : impl_(new Impl)
    {}

    template< class Config >
    Value_impl< Config >::Value_impl( const Const_str_ptr value )
        : impl_(new Impl(str_type, value))
    {}

    template< class Config >
    Value_impl< Config >::Value_impl( const String_type& value )
        : impl_(new Impl(str_type, value))
    {}

    template< class Config >
    Value_impl< Config >::Value_impl( const Object& value )
        : impl_(new Impl(obj_type, value))
    {}

    template< class Config >
    Value_impl< Config >::Value_impl( const Array& value )
        : impl_(new Impl(array_type, value))
    {}

    template< class Config >
    Value_impl< Config >::Value_impl( bool value )
        : impl_(new Impl(bool_type, value))
    {}

    template< class Config >
    Value_impl< Config >::Value_impl( int value )
        : impl_(new Impl(int64_type, static_cast< boost::int64_t >(value)))
    {}

    template< class Config >
    Value_impl< Config >::Value_impl( unsigned value )
        : impl_(new Impl(int64_type, static_cast< boost::int64_t >(value)))
    {}

    template< class Config >
    Value_impl< Config >::Value_impl( boost::int64_t value )
        : impl_(new Impl(int64_type, value))
    {}

    template< class Config >
    Value_impl< Config >::Value_impl( boost::uint64_t value )
        : impl_(new Impl(int64_type, static_cast< boost::int64_t >(value)))
    {}

    template< class Config >
    Value_impl< Config >::Value_impl( double value )
        : impl_(new Impl(real_type, value))
    {}

    template< class Config >
    Value_impl< Config >::Value_impl(const Value_impl< Config > & other)
        : impl_(other.impl_)
    {}

    template< class Config >
    Value_impl< Config > & Value_impl< Config >::operator=(const Value_impl & other)
    {
        impl_ = other.impl_;
        return *this;
    }

    template< class Config >
    bool Value_impl< Config >::operator==( const Value_impl & lhs ) const
    {
        if( this == &lhs )
        {
            return true;
        }
        if (!impl_ && !lhs.impl_)
        {
            return true;
        }
        if (impl_ && lhs.impl_)
        {
            if (impl_->type_ == impl_->lhs.type_
                && v_ == lhs.v_)
            {
                return true;
            }
        }
        return false;
    }

    template< class Config >
    Value_type Value_impl< Config >::type() const
    {
        return impl_->type_;
    }

    template< class Config >
    bool Value_impl< Config >::is_null() const
    {
        return impl_->type_ == null_type;
    }

    template< class Config >
    void Value_impl< Config >::check_type( const Value_type vtype ) const
    {
        if ( impl_->type_ != vtype ) 
        {
            std::ostringstream os;
            os << "value type is " << impl_->type_ << " while requested type " << vtype;
            throw std::runtime_error( os.str() );
        }
    }

    template< class Config >
    typename Value_impl< Config >::Object& Value_impl< Config >::get_obj()
    {
        check_type( obj_type );
        return std::get< Object >( impl_->v_ );
    }

    template< class Config >
    const typename Value_impl< Config >::Object& Value_impl< Config >::get_obj() const
    {
        check_type( obj_type );
        return std::get< Object >( impl_->v_ );
    }
     
    template< class Config >
    typename Value_impl< Config >::Array& Value_impl< Config >::get_array()
    {
        check_type(  array_type );

        return std::get< Array >( impl_->v_ );
    }

    template< class Config >
    const typename Value_impl< Config >::Array& Value_impl< Config >::get_array() const
    {
        check_type(  array_type );
        return std::get< Array >( impl_->v_ );
    }
     
    template< class Config >
    const typename Config::String_type& Value_impl< Config >::get_str() const
    {
        check_type( str_type );
        return std::get< String_type >( impl_->v_ );
    }

    template< class Config >
    bool Value_impl< Config >::get_bool() const
    {
        check_type(  bool_type );
        return std::get< bool >( impl_->v_ );
    }
     
    template< class Config >
    boost::int64_t Value_impl< Config >::get_int64() const
    {
        check_type( int64_type );
        return std::get< boost::int64_t >( impl_->v_ );
    }

    template< class Config >
    double Value_impl< Config >::get_double() const
    {
        check_type( real_type );
        return std::get< double >( impl_->v_ );
    }

//     template< class Config >
//     Pair_impl< Config >::Pair_impl( const String_type& name, const Value_type& value )
//         :   name_( name )
//         ,   value_( value )
//     {}
// 
//     template< class Config >
//     bool Pair_impl< Config >::operator==( const Pair_impl< Config >& lhs ) const
//     {
//         if( this == &lhs ) return true;
//         return ( name_ == lhs.name_ ) && ( value_ == lhs.value_ );
//     }

    // converts a C string, ie. 8 bit char array, to a string object
    //
    template < class String_type >
    String_type to_str( const char* c_str )
    {
        String_type result;
        for (const char* p = c_str; *p != 0; ++p)
        {
            result += *p;
        }
        return result;
    }

    //
    /*
    namespace internal_
    {
        template< typename T >
        struct Type_to_type
        {
        };

        template< class Value > 
        int get_value( const Value& value, Type_to_type< int > )
        {
            return value;
        }
       
        template< class Value > 
        boost::int64_t get_value( const Value& value, Type_to_type< boost::int64_t > )
        {
            return value;
        }
       
        template< class Value > 
        boost::uint64_t get_value( const Value& value, Type_to_type< boost::uint64_t > )
        {
            return value;
        }
       
        template< class Value > 
        double get_value( const Value& value, Type_to_type< double > )
        {
            return value;
        }
       
        template< class Value > 
        typename Value::String_type get_value( const Value& value, Type_to_type< typename Value::String_type > )
        {
            return value;
        }
       
        template< class Value > 
        typename Value::Array get_value( const Value& value, Type_to_type< typename Value::Array > )
        {
            return value;
        }
       
        template< class Value > 
        typename Value::Object get_value( const Value& value, Type_to_type< typename Value::Object > )
        {
            return value;
        }
       
        template< class Value > 
        bool get_value( const Value& value, Type_to_type< bool > )
        {
            return value;
        }
    }
    */
    template< class Config >
    template< typename T > 
    T Value_impl< Config >::get_value() const
    {
        return static_cast<T>(*this);//internal_::get_value( *this, internal_::Type_to_type< T >() );
    }
}

#endif
