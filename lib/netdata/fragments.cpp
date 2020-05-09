
#include "stdafx.h"

#include "types.h"
#include "compactuint.h"
#include "netdata/fragment.h"
#include "netdata/fragments.h"

//------------------------------------------------------------------------------
bool identifyFragment(const barray & dataStream, barray::const_iterator & curPos, unsigned int *id)
{
    barray::const_iterator  pos = curPos;
    return getCUI_safe(dataStream, pos, id);
}

bool identifyFragmentGameinfo(const barray & dataStream, barray::const_iterator & curPos, unsigned int *id)
{
    unsigned int            length;
    barray::const_iterator  pos = curPos;

    if ( ! getCUI_safe(dataStream, pos, &length))
    {
        return false;
    }

    size_t bytesLeft = dataStream.end() - pos;
    if (bytesLeft < 2)
    {
        return false;
    }

    *id = getWORD_r(pos);
    return true;
}

//------------------------------------------------------------------------------
Fragment::Fragment(const barray & dataStream, barray::const_iterator & curPos)
{
    barray::const_iterator  pos = curPos;
    unsigned int            payloadLength;

    if ( ! getCUI_safe(dataStream, pos, &id_)
        || ! getCUI_safe(dataStream, pos, &payloadLength))
    {
        return;
    }

    size_t bytesLeft = dataStream.end() - pos;
    if (bytesLeft < payloadLength)
    {
        return;
    }

    payload_.assign(pos, pos + payloadLength);
    curPos = pos + payloadLength;
    isOk_ = true;
}

barray Fragment::assemble(unsigned int id)
{
    barray result;
    barray tmp(5);
    barray::iterator pos;

    // pack ID
    pos = tmp.begin();
    putCUI(pos, id);
    result.insert(result.end(), tmp.begin(), pos);

    // pack the length
    pos = tmp.begin();
    putCUI(pos, payload_.size());
    result.insert(result.end(), tmp.begin(), pos);

    result.insert(result.end(), payload_.begin(), payload_.end());

    isOk_ = true;
    isParsed_ = true;
    id_ = id;


    return result;
}

void Fragment::print(tostream & stream) const
{
    checkOk();
    checkParsed();

    stream << hex << std::uppercase << _T(" [") << id_ << _T("] L=") << payload_.size() << endl;
    dumpBytes(stream, payload_.begin(), payload_.end());
}

//------------------------------------------------------------------------------
FragmentGameinfo::FragmentGameinfo(const barray & dataStream, barray::const_iterator & curPos)
{
    barray::const_iterator  pos = curPos;
    unsigned int            payloadLength;

    if ( ! getCUI_safe(dataStream, pos, &payloadLength))
    {
        return;
    }

    size_t bytesLeft = dataStream.end() - pos;
    if (bytesLeft < payloadLength)
    {
        return;
    }

    id_ = getWORD_r(pos);
    payloadLength -= 2;
    payload_.assign(pos, pos + payloadLength);
    curPos = pos + payloadLength;
    isOk_ = true;
}

barray FragmentGameinfo::assemble(unsigned short id)
{
    unsigned int payloadLength = payload_.size() + 2 /*id*/;

    barray result(7 /*5 + 2*/);
    barray::iterator pos = result.begin();

    putCUI(pos, payloadLength);
    putWORD_r(pos, id);
    result.resize(pos - result.begin());
    result.insert(result.end(), payload_.begin(), payload_.end());

    isOk_ = true;
    isParsed_ = true;
    id_ = id;

    return result;
}

void FragmentGameinfo::print(tostream & stream) const
{
    checkOk();
    checkParsed();

    stream << hex << std::uppercase << _T("  [") << id_ << _T("] L=") << payload_.size() << endl;
    dumpBytes(stream, payload_.begin(), payload_.end());
}

//------------------------------------------------------------------------------
FragmentFactory::FragmentFactory(identifyFragment_fn identifier, const fragmentCollection_t *collection, fragment_ctor_t default_ctor)
    : default_ctor_(default_ctor), identifier_(identifier)
{
    // move collection to map structure, check for duplicates

    while (collection->ctor != NULL)
    {
        if (collection_.find(collection->type) != collection_.end())
        {
            throw eFragmentFactoryDuplicate;
        }

        collection_[collection->type] = collection->ctor;
        ++collection;
    }
}

FragmentBase * FragmentFactory::create(const barray & dataStream, barray::const_iterator & curPos)
{
    unsigned int type;
    
    if ( ! identifier_(dataStream, curPos, &type))
    {
        return NULL;
    }

    collection_t::const_iterator it = collection_.find(type);

    if (it != collection_.end())
    {
        return it->second(dataStream, curPos);
    }
    else
    {
        if (default_ctor_ != NULL)
        {
            return default_ctor_(dataStream, curPos);
        }
        else
        {
            return NULL;
        }
    }
}

//------------------------------------------------------------------------------
FragmentSet::FragmentSet(const barray & dataStream, barray::const_iterator & curPos, FragmentFactory *factory)
    : Fragment(dataStream, curPos)
{
    if (!isOk_)
    {
        return;
    }

    barray::const_iterator cur = payload_.begin();
    while (cur < payload_.end())
    {
        FragmentBase *newFragment = factory->create(payload_, cur);
        fragmentArray_.push_back(newFragment);
        if (newFragment == NULL || ! newFragment->isOk())
        {
            return;
        }
    }

    if (cur == payload_.end())
    {
        isOk_ = true;
    }
}

FragmentSet::~FragmentSet()
{
    for (fragmentArray_t::const_iterator it = fragmentArray_.begin(); it != fragmentArray_.end(); ++it)
    {
        delete *it;
    }
}

bool FragmentSet::parse()
{
    checkOk();

    isParsed_ = true;        // empty payload is ok

    for (fragmentArray_t::const_iterator it = fragmentArray_.begin();
        it != fragmentArray_.end() && isParsed_;
        ++it)
    {
        if ( ! (*it)->isParsed())
        {
            isParsed_ = (*it)->parse();
        }
    }

    return isParsed_;
}

barray FragmentSet::assemble(const unsigned int id)
{
    payload_.clear();

    // pack the fragments
    for (fragmentArray_t::const_iterator it = fragmentArray_.begin();
        it != fragmentArray_.end();
        ++it)
    {
        barray intr = (*it)->assemble();
        payload_.insert(payload_.end(), intr.begin(), intr.end());
    }

    // pack the length
    barray tmp(5);
    barray::iterator pos = tmp.begin();
    putCUI(pos, payload_.size());
    payload_.insert(payload_.begin(), tmp.begin(), pos);

    // pack ID
    pos = tmp.begin();
    putCUI(pos, id);
    payload_.insert(payload_.begin(), tmp.begin(), pos);

    isOk_ = true;
    isParsed_ = true;
    id_ = id;

    return payload_;
}

void FragmentSet::print(tostream &stream) const
{
    checkOk();
    checkParsed();

    stream << hex << std::uppercase << _T(" Set [") << id_ << _T("] L=") << payload_.size() << _T(" (") << dec << fragmentArray_.size() << (')') << endl;

    for (fragmentArray_t::const_iterator it = fragmentArray_.begin(); it != fragmentArray_.end(); ++it)
    {
        stream << (*it);
    }
}

//------------------------------------------------------------------------------

void errdump(FILE *fh, const barray & data)
{
    barray::const_iterator pos = data.begin();

    unsigned inLine = 0;
    while(pos != data.end())
    {
        fprintf(fh, "%02X ", *pos);
        ++pos;
        ++inLine;
        if (inLine == 16)
        {
            fprintf(fh, "\n");
            inLine = 0;
        }
    }
    fprintf(fh, "\n");
}

void errdump(const barray & data)
{
    FILE *fh = nullptr;
    fopen_s(&fh, "errors.txt", "a+");
    if (fh != NULL)
    {
        fprintf(fh, "[%08X] Parsing error\n", (unsigned) time(0));
        errdump(fh, data);

        fclose(fh);
    }
}

void errdump(const FragmentBase *fragment)
{
    FILE *fh = nullptr;
    fopen_s(&fh, "errors.txt", "a+");
    if (fh != NULL)
    {
        fprintf(fh, "[%08X] at %s\n", (unsigned) time(0), typeid(*fragment).name());
        errdump(fh, fragment->bytes());

        fclose(fh);
    }
}