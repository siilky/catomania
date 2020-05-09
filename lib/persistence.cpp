// $Id: persistence.cpp 889 2013-03-02 18:16:17Z jerry $
//

#include "stdafx.h"

#include "common.h"
#include "persistence.h"

#define BOOST_UTF8_BEGIN_NAMESPACE
#define BOOST_UTF8_END_NAMESPACE
#define BOOST_UTF8_DECL
#include <boost/detail/utf8_codecvt_facet.hpp>
#include "boost/version.hpp"
#if (BOOST_VERSION / 100) >= 1050
    #include <boost/detail/utf8_codecvt_facet.ipp>
#else
    #include <../libs/serialization/src/utf8_codecvt_facet.cpp>
#endif

// --- Config ---

ConfigFile::~ConfigFile()
{
    fflush();
}

bool ConfigFile::open(const tstring & filename )
{
    filename_ = filename;

    std::wifstream file;
    std::locale utf8(std::locale(), new utf8_codecvt_facet());
    file.imbue(utf8);

    file.open(filename_.c_str(), std::ios_base::binary);
    if (!file)
    {
        return false;
    }

    // take care about unicode header
    wchar_t c = file.peek();
    if (c == 0xFEFF)
    {
        file.get();
    }

    bool result = json::read(file, jsonRoot_);
    return result;
}

void ConfigFile::fflush()
{
    if (filename_.empty()       // wasn't opened
        /*dirty is broken?*/ || !isDirty_)           // no changes
    {
        return;
    }

    std::wofstream file;

    std::locale utf8(std::locale(), new utf8_codecvt_facet());
    file.imbue(utf8);

    file.open(filename_.c_str(), std::ios_base::binary | std::ios_base::trunc);
    if (file)
    {
        json::write_formatted(jsonRoot_, file);
    }

    isDirty_ = false;
}

// --- Persistence ---

std::wstring ConfigNode::GetString( const WCHAR *key, const std::wstring & defaultVal ) const
{
    return getValue(key, defaultVal);
}

void ConfigNode::WriteString( const WCHAR *key, std::wstring val ) const
{
    WriteValue(key, val);
}

void ConfigNode::WriteString( const WCHAR *key, WCHAR *val ) const
{
    WriteString(key, std::wstring(val));
}

int ConfigNode::GetInt( const WCHAR *key, int defaultVal /*= 0*/ ) const
{
    return getValue(key, defaultVal);
}

void ConfigNode::WriteInt( const WCHAR *key, int val ) const
{
    WriteValue(key, val);
}

unsigned ConfigNode::GetUInt( const WCHAR *key, unsigned defaultVal /*= 0*/ ) const
{
    return static_cast<unsigned>(getValue(key, static_cast<boost::uint64_t>(defaultVal)));
}

void ConfigNode::WriteUInt( const WCHAR *key, UINT val ) const
{
    WriteValue(key, static_cast<boost::uint64_t>(val));
}

bool ConfigNode::GetBool( const WCHAR *key, bool defaultVal ) const
{
    return getValue(key, defaultVal);
}

void ConfigNode::WriteBool( const WCHAR *key, bool val ) const
{
    WriteValue(key, val);
}

double ConfigNode::GetDouble( const WCHAR *key, double defaultVal ) const
{
    return getValue(key, defaultVal);
}

void ConfigNode::WriteDouble( const WCHAR *key, double val) const
{
    WriteValue(key, val);
}
