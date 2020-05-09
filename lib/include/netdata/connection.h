#ifndef connection_h
#define connection_h

#include "netio/netchannel.h"
#include "fragment_manager.h"


// read it as 'FragmentedIO'
class ConnectionBase
{
public:
    class Cookie : public std::pair<int /*where*/, FragmentProcessor::Cookie>
    {
    public:
        Cookie()
        {}

        Cookie(std::pair<int, FragmentProcessor::Cookie> pair)
            : std::pair<int /*where*/, FragmentProcessor::Cookie>(pair)
        {}

//         Cookie(int pos, const FragmentProcessor::Cookie & cookie)
//             : std::pair<int /*where*/, FragmentProcessor::Cookie>(pos, cookie)
//         {}

        bool empty()
        {
            return second.empty();
        }
    };

    ConnectionBase();
    virtual ~ConnectionBase();

    virtual bool isConnected();
    virtual const ErrorState & getError();

    // receive/process server/client messages
    bool receive();

    // send to server
    bool send(FragmentBase *fragment);
    bool send(const barray & data);

    template<class FragmentType>
    Cookie bindServerHandler(std::function< void (const FragmentType *) > handler)
    {
        return std::make_pair( CookieServer
                             , serverProcessor_->registerHandler( FragmentType::ID
                                                                , std::bind(&fragmentHandler_i<FragmentType>, std::placeholders::_1, handler)));
    }

    template<class FragmentType>
    Cookie bindClientHandler(std::function<void (const FragmentType *)> handler)
    {
        return std::make_pair( CookieClient
                             , clientProcessor_->registerHandler( FragmentType::ID
                                                                , std::bind(&fragmentHandler_i<FragmentType>, std::placeholders::_1, handler)));
    }

#if defined(FRAGMENT_PRINTABLE)
    void bindServerDefaultHandler(std::function< void(const FragmentBase *) > handler)
    {
        serverProcessor_->registerDefaultHandler(handler);
    }
    void bindClientDefaultHandler(std::function< void(const FragmentBase *) > handler)
    {
        clientProcessor_->registerDefaultHandler(handler);
    }
#endif

    virtual ConnectionBase & unbindHandler(Cookie & cookie);

    ConnectionBase & unbindHandlers(std::vector<Cookie> & cookies);

protected:
    enum
    {
        CookieServer       = 1,
        CookieClient,
        CookieBaseLast,
    };

    // callback function wrapper
    template<class FragmentType>
    static void fragmentHandler_i(const FragmentBase *f, std::function<void (const FragmentType *)> handler)
    {
        assert(f != NULL);      // invalid fragment passed
        auto *spec = dynamic_cast<const FragmentType *>(f);
        assert(spec != NULL);   // invalid fragment casting
        handler(spec);
    }

    ErrorState  error_;

    NetChannel  *server_ = nullptr;
    NetChannel  *client_ = nullptr;

    FragmentProcessor   *serverProcessor_ = nullptr;
    FragmentFactory     *serverFactory_ = nullptr;
    FragmentProcessor   *clientProcessor_ = nullptr;
    FragmentFactory     *clientFactory_ = nullptr;

#ifdef _DEBUG
    tofstream   logfile_;
    TCHAR       logStrbuf_[4096];
#endif

private:
    bool processChannel( NetChannel        *channel
                       , FragmentProcessor *processor
                       , FragmentFactory   *factory);

    std::vector<FragmentProcessor::Cookie> sHooks_, cHooks_;
};


// read it as 'FragmentedIO_PW'
class Connection : public ConnectionBase
{
public:
    Connection();
    virtual ~Connection();

    template<class FragmentType>
    Cookie bindServerHandler(std::function< void(const FragmentType *) > handler)
    {
        if (boost::is_base_of<FragmentGameinfo, FragmentType>::value)
        {
            return std::make_pair(CookieServerGi
                                 , serverGiProcessor_->registerHandler(FragmentType::ID
                                                                      , std::bind(&fragmentHandler_i<FragmentType>, std::placeholders::_1, handler)));
        }
        else
        {
            return ConnectionBase::bindServerHandler(handler);
        }
    }

    template<class FragmentType>
    Cookie bindClientHandler(std::function<void(const FragmentType *)> handler)
    {
        if (boost::is_base_of<FragmentGameinfo, FragmentType>::value)
        {
            return std::make_pair(CookieClientGi
                                 , clientGiProcessor_->registerHandler(FragmentType::ID
                                                                      , std::bind(&fragmentHandler_i<FragmentType>, std::placeholders::_1, handler)));
        }
        else
        {
            return ConnectionBase::bindClientHandler(handler);
        }
    }

    virtual Connection & unbindHandler(Cookie & cookie) override;

protected:
    enum
    {
        CookieServerGi = CookieBaseLast + 1,
        CookieClientGi,
        CookieConnectionLast
    };

private:

    FragmentProcessor   *serverGiProcessor_;
    FragmentProcessor   *clientGiProcessor_;

    std::vector<FragmentProcessor::Cookie> sgiHooks_, cgiHooks_;
};


#endif
