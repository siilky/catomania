#ifndef fragments_h
#define fragments_h

#include <map>
#include <iomanip>

using std::endl;
using std::dec;
using std::hex;
using std::setw;
using std::string;
using std::wstring;

#include "netdata/fragment.h"


typedef bool (*identifyFragment_fn)(const barray & dataStream, barray::const_iterator & curPos, unsigned int *id);

// return false if there's not enough data to read
bool identifyFragment(const barray & dataStream, barray::const_iterator & curPos, unsigned int *id);

// return false if there's not enough data to read
bool identifyFragmentGameinfo(const barray & dataStream, barray::const_iterator & curPos, unsigned int *id);

//  FragmentBase:       Interface base object
//  Fragment:           <id>.cui <length>.cui <data>.length
//  FragmentGameinfo:   <length>.cui <id>.w <data>.length
//

// Read fragment in form <ID><Length>
// If fragment was successfully read then IsOk property sets to true and curPos 
//  will be advanced to the amount of bytes read, else IsOk() will be set to 
//  false if there's not enough data of format error.
// The parse() operation actually interprets fragment data to internal structure.
// If fragment was constructed then it guaranteed to have payload of length 
// declared in header 
//

class Fragment : public FragmentBase
{
public:
    Fragment()
    {}
    Fragment(unsigned id)
        : FragmentBase(id)
    {}
    Fragment(const barray & data, barray::const_iterator & curPos);

    virtual bool parse()
    {
        isParsed_ = true;
        return isParsed_;
    }

protected:
    barray assemble(unsigned int id);

private:
    // hide default user assemble function (visible in child)
    virtual barray assemble()
    {
        return barray();
    }

    virtual void print(tostream & stream) const;
};

// Specialization template to instantiate certain fragment objects
template<class Base, class Impl>
class FragmentFormatSpec : public Base
                         , public Impl
{
public:
    FragmentFormatSpec()
        : Base(Impl::ID)
    {}
    FragmentFormatSpec(const barray & data, barray::const_iterator & curPos)
        : Base(data, curPos)
    {}

    virtual bool parse()
    {
        checkOk();

        // format() can alter isParsed_
        SerializerIn in(payload_);
        Impl::format(in);

        isParsed_ = !in.isOk();
        return isParsed_;
    }

    virtual barray assemble()
    {
        payload_.clear();

        SerializerLength calcSize;
        Impl::format(calcSize);
        payload_.resize(calcSize.size());

        SerializerOut out(payload_);
        Impl::format(out);

        return Base::assemble((unsigned short)id_);
    }

private:
    virtual void print(tostream & stream) const
    {
#if defined(FRAGMENT_PRINTABLE)
        checkParsed();

        stream << _T("  [") << hex << Impl::ID << _T("] ") << typeid(Impl).name() << endl;

        Impl::print(stream);
        stream << endl;

        dumpBytes(stream, payload_.begin(), payload_.end());
        stream << _T(" ---") << endl;
#else
        (void)stream;
#endif
    }
};

template<class Impl>
class FragmentSpec : public FragmentFormatSpec<Fragment, Impl>
{
public:
    FragmentSpec()
    {}
    FragmentSpec(const barray & data, barray::const_iterator & curPos)
        : FragmentFormatSpec(data, curPos)
    {}
};

template<class Impl>
class FragmentBaseSpec : public FragmentFormatSpec<FragmentBase, Impl>
{
public:
    FragmentBaseSpec()
    {}
    FragmentBaseSpec(const barray & data, barray::const_iterator & curPos)
        : FragmentFormatSpec(data, curPos)
    {}
};

//------------------------------------------------------------------------------
class FragmentGameinfo : public FragmentBase
{
public:
    FragmentGameinfo()
    {};
    FragmentGameinfo(unsigned id)
        : FragmentBase(id)
    {}
    FragmentGameinfo(const barray & dataStream, barray::const_iterator & curPos);

    virtual bool parse()
    {
        isParsed_ = true;
        return isParsed_;
    }

protected:
    // assemble packet payload as content of FGI_* with given id
    barray assemble(unsigned short id);

private:
    // hide default user assemble function (visible in child)
    virtual barray assemble()
    {
        return barray();
    }

    virtual void print(tostream & stream) const;
};

template<class Impl>
class FragmentGiSpec : public FragmentFormatSpec<FragmentGameinfo, Impl>
{
public:
    FragmentGiSpec()
    {}
    FragmentGiSpec(const barray & data, barray::const_iterator & curPos)
        : FragmentFormatSpec(data, curPos)
    {}
};

//------------------------------------------------------------------------------
class FragmentFactory
{
public:
    FragmentFactory(identifyFragment_fn identifier, const fragmentCollection_t *collection, fragment_ctor_t default_ctor = NULL);

    // return NULL if there's not enough data to recognize fragment structure
    virtual FragmentBase *create(const barray & dataStream, barray::const_iterator & curPos);

private:
    typedef std::map<unsigned int, fragment_ctor_t>   collection_t;

    collection_t        collection_;
    fragment_ctor_t     default_ctor_;
    identifyFragment_fn identifier_;
};


//------------------------------------------------------------------------------
// A set of Fragments.
// IsOk() and curPos behavior are similar to Fragment's
//
class FragmentSet : public Fragment
{
public:
    FragmentSet()
    {}
    FragmentSet(FragmentBase *fragment)
        : fragmentArray_(1, fragment)
    {}
    FragmentSet(const barray & dataStream, barray::const_iterator & curPos)
        : Fragment(dataStream, curPos)
    {}
    FragmentSet(const barray & dataStream, barray::const_iterator & curPos, FragmentFactory *factory);
    virtual ~FragmentSet();

    virtual bool parse();

    size_t getNumFragments() const
    {
        return fragmentArray_.size();
    }

    FragmentBase * getFragment(size_t index) const
    {
        return fragmentArray_[index];
    }

    void putFragment(FragmentBase *fragment)
    {
        fragmentArray_.push_back(fragment);
    }

    barray assemble(unsigned int id);

protected:
    virtual void print(tostream & stream) const;

    typedef std::vector<FragmentBase *>    fragmentArray_t;

    fragmentArray_t     fragmentArray_;

private:
    // hide default assemble function
    virtual barray assemble()
    {
        return barray();
    }
};

//------------------------------------------------------------------------------

inline tostream & operator<<(tostream & stream, const FragmentBase *fragment)
{
    fragment->print(stream);
    return stream;
}

inline tostream & operator<<(tostream & stream, FragmentBase *fragment)
{
    return stream << static_cast<const FragmentBase *>(fragment);
}


void errdump(const FragmentBase *fragment);
void errdump(const barray & data);
void errdump(FILE *fh, const barray & data);

#endif