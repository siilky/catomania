#ifndef fragment_manager_h
#define fragment_manager_h

#include <set>

#include "netdata/fragments.h"
#include "netio/netchannel.h"
#include "thread.h"

class FragmentProcessor : private Syncronized
{
public:

    typedef std::function< void (const FragmentBase *f) > FragmentHandler;
    typedef std::function< void (const barray & data) >   ErrorHandler;

    class Cookie
    {
    public:
        Cookie()
            : cookieCtr(0), fragmentId(0)
        {}
        Cookie(unsigned id, unsigned ctr)
            : fragmentId(id)
            , cookieCtr(ctr)
        {}

        bool operator<(const Cookie & r) const
        {
            return (fragmentId < r.fragmentId)
                || (fragmentId ==  r.fragmentId && cookieCtr < r.cookieCtr);
        }

        void reset()
        {
            cookieCtr = 0;
        }

        bool empty()
        {
            return cookieCtr == 0;
        }

        unsigned    fragmentId;
        unsigned    cookieCtr;
    };

    FragmentProcessor()
        : inProcessing_(false), cookieCounter_(1)
    {}

    // TODO: this is used in FragmentManager(const FragmentManager & r)
    FragmentProcessor(const FragmentProcessor & /*r*/)
        : inProcessing_(false), cookieCounter_(1)
    {}
    virtual ~FragmentProcessor() {}

    // returns registration cookie
    Cookie registerHandler(unsigned int id, FragmentHandler handler);

    void unregisterHandler(Cookie & cookie);                    // resets cookie
    void unregisterHandlers(std::vector<Cookie> & cookies);     // resets cookies (clears vector)

    void registerMirrorHandler(FragmentHandler handler)
    {
        mirrorHandler_ = handler;
    }

    void registerDefaultHandler(FragmentHandler handler)
    {
        defaultHandler_ = handler;
    }

    void registerErrorHandler(ErrorHandler handler)
    {
        errorHandler_ = handler;
    }

    void unregisterMirrorHandler()
    {
        mirrorHandler_ = nullptr;
    }

    void unregisterDefaultHandler()
    {
        defaultHandler_ = nullptr;
    }

    void unregisterErrorHandler()
    {
        errorHandler_ = nullptr;
    }

    virtual void process(FragmentBase *fragment);

    void processError(const barray & data)
    {
        if (errorHandler_ != NULL)
        {
            errorHandler_(data);
        }
    }

private:
    typedef std::multimap< unsigned int /*id*/, std::pair< unsigned /*cookie*/, FragmentHandler > >  FragmentHandlers;


    void eraseHandler(FragmentHandlers & map, Cookie cookie);

    FragmentHandlers    handlers_;

    FragmentHandler     defaultHandler_, mirrorHandler_;
    ErrorHandler        errorHandler_;

    volatile int        inProcessing_;          // processing count - increments as we enter process and decrements as we exit process() (process van be recursive for example parsing of an array)

    FragmentHandlers    addedList_;             // List of handlers added during process()
    std::set<Cookie>    removedList_;           // List of handlers deleted during process()

    unsigned            cookieCounter_;
};


// common handler for fragmentArray / FragmentGameInfo
template<class FragmentSet>
void fragmentCollectionHandler(const FragmentBase *fragment, FragmentProcessor *processor)
{
    const FragmentSet *fs = dynamic_cast<const FragmentSet *>(fragment);
    if (fs == NULL)
    {
        return;
    }

    size_t num_fragments = fs->getNumFragments();
    for (size_t i = 0; i < num_fragments; i++)
    {
        FragmentBase *newF = fs->getFragment(i);
        if (newF != NULL)
        {
            if ( ! newF->isParsed() )
            {
                try
                {
                    newF->parse();
                }
                catch (...)
                {
                    errdump(newF);
                }
            }

            processor->process(newF);
        }
    }
}


#endif
