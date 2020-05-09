#ifndef fragment_h
#define fragment_h

#include "netdata/sw_version.h"
#include "utils.h"


enum FragmentException
{
    ePackedIntUnderflow,
    eFragmentError,
    eFragmentNotParsed,
    eFragmentFactoryDuplicate,
    eContentError,
};

//------------------------------------------------------------------------------
class FragmentBase
{
public:
    FragmentBase()
        : isOk_(false), isParsed_(false), id_(0)
    {}
    FragmentBase(unsigned id)
        : isOk_(true), isParsed_(true), id_(id)
    {}
    FragmentBase(const barray & data, barray::const_iterator & curPos)
        : isOk_(true), isParsed_(false), id_(0)
        , payload_(curPos, data.end())
    {}

    virtual ~FragmentBase()
    {}

    // Parse payload data and store in data members
    virtual bool parse()
    {
        isParsed_ = true;
        return true;
    }

    // Assemble data members to payload data
    virtual barray assemble()
    {
        return payload_;
    }
    virtual barray assemble(unsigned int /*id*/)
    {
        return payload_;
    }

    unsigned int getId() const
    {
        return id_;
    }

    bool isOk() const
    {
        return isOk_;
    }

    bool isParsed() const
    {
        return isParsed_;
    }

    barray & bytes()
    {
        return payload_;
    }

    const barray & bytes() const
    {
        return payload_;
    }

protected:
    void checkOk() const
    {
        if ( ! isOk_)
        {
            throw eFragmentError;
        }
    }

    void checkParsed() const
    {
        if ( ! isParsed_)
        {
            throw eFragmentNotParsed;
        }
    }

    virtual void print(tostream & stream) const
    {
        checkOk();
        checkParsed();

        stream << std::hex << std::uppercase << _T(" Fragment [") << id_ << _T("] L=") << payload_.size() << std::endl;
        dumpBytes(stream, payload_.begin(), payload_.end());
    }

    friend tostream & operator<<(tostream & stream, const FragmentBase *fragment);

    barray          payload_;
    unsigned int    id_;            // Id of fragment as parsed from input stream
    bool            isOk_;
    bool            isParsed_;
};

//------------------------------------------------------------------------------

typedef FragmentBase * (*fragment_ctor_t)(const barray & dataStream, barray::const_iterator & curPos);

template< class Parent >
FragmentBase * fragment_static_ctor(const barray & dataStream, barray::const_iterator & curPos)
{
    return new Parent(dataStream, curPos);
}

typedef struct 
{
    unsigned int        type;
    fragment_ctor_t     ctor;
} fragmentCollection_t;

#define COLLECTION_ELEMENT_GI(name)             { FragmentGi##name::ID, fragment_static_ctor< FragmentGi##name >  }
#define COLLECTION_ELEMENT_GI_2(name)           { FragmentGi##name::ID2,fragment_static_ctor< FragmentGi##name >  }
#define COLLECTION_ELEMENT_GI_3(name)           { FragmentGi##name::ID3,fragment_static_ctor< FragmentGi##name >  }
#define COLLECTION_ELEMENT_GI_4(name)           { FragmentGi##name::ID4,fragment_static_ctor< FragmentGi##name >  }
#define COLLECTION_ELEMENT_GI_ASSIGN(id, name)  { id,                   fragment_static_ctor< FragmentGi##name >  }
#define COLLECTION_ELEMENT(name)                { Fragment##name::ID,   fragment_static_ctor< Fragment##name >  }
#define COLLECTION_ELEMENT_ASSIGN(id, name)     { FRAGMENT_##id,        fragment_static_ctor< Fragment##name >  }
#define COLLECTION_END                          { 0, NULL }


#endif // fragment_h