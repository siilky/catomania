#ifndef connection_h
#define connection_h

#include "netio/netchannel.h"
//#include "netdata/fragments_server.h"
#include "fragment_manager.h"

namespace serverdata
{
    class AccInfoImpl;
}

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

#if PW_SERVER_VERSION >= 1700
    void setGiIdEncodeVal(unsigned v)
    {
        GIEncodeValue = v;
    }
    void setGiIdEncodeVal2(unsigned v2)
    {
        GIEncodeValue2 = v2;
    }
#endif

    // send to server
    bool send(FragmentBase *fragment);
    bool send(const barray & data);

    template<class FragmentType>
    Cookie bindServerHandler(std::function< void (const FragmentType *) > handler)
    {
        if (serverProcessor_)
        {
            return std::make_pair( CookieServer
                                 , serverProcessor_->registerHandler( FragmentType::ID
                                                                    , std::bind(&fragmentHandler_i<FragmentType>, std::placeholders::_1, handler)));
        }
        else
        {
            return {};
        }
    }

    template<class FragmentType>
    Cookie bindClientHandler(std::function<void (const FragmentType *)> handler)
    {
        if (clientProcessor_)
        {
            return std::make_pair( CookieClient
                                 , clientProcessor_->registerHandler( FragmentType::ID
                                                                    , std::bind(&fragmentHandler_i<FragmentType>, std::placeholders::_1, handler)));
        }
        else
        {
            return {};
        }
    }

#if defined(FRAGMENT_PRINTABLE)
    void bindServerDefaultHandler(std::function< void(const FragmentBase *) > handler)
    {
        if (serverProcessor_)
        {
            serverProcessor_->registerDefaultHandler(handler);
        }
    }
    void bindClientDefaultHandler(std::function< void(const FragmentBase *) > handler)
    {
        if (clientProcessor_)
        {
            clientProcessor_->registerDefaultHandler(handler);
        }
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

#if PW_SERVER_VERSION >= 1700
    unsigned GIEncodeValue = 0;
    unsigned GIEncodeValue2 = 0;
#endif
};


// read it as 'FragmentedIO_PW'
class Connection : public ConnectionBase
{
public:
    Connection(bool safeMode = false);
    virtual ~Connection();

    template<class FragmentType>
    Cookie bindServerHandler(std::function< void(const FragmentType *) > handler)
    {
        if (serverGiProcessor_
            && std::is_base_of<FragmentGameinfo, FragmentType>::value)
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
        if (clientGiProcessor_
            && std::is_base_of<FragmentGameinfo, FragmentType>::value)
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
    void setAccountId(const serverdata::AccInfoImpl * f);
    Cookie  AccountInfoHandler_;

    FragmentProcessor   *serverGiProcessor_ = nullptr;
    FragmentProcessor   *clientGiProcessor_ = nullptr;

    std::vector<FragmentProcessor::Cookie> sHooks_, cHooks_;
};


#endif
