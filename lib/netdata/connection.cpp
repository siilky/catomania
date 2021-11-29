
#include "stdafx.h"

#include "utils.h"
#include "netdata/connection.h"
#include "netdata/fragments.h"
#include "netdata/fragments_server.h"
#include "netdata/fragments_client.h"
#include "netdata/connectionx.h"
#include "netio/netchannelx.h"


ConnectionBase::ConnectionBase()
{
#ifdef _DEBUG
    logfile_.open("connlog.txt", std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    if (logfile_)
    {
        logfile_.rdbuf()->pubsetbuf(logStrbuf_, sizeof_array(logStrbuf_));
    }
#endif
}

ConnectionBase::~ConnectionBase()
{
    if (client_)
    {
        delete client_;
    }
    if (server_)
    {
        delete server_;
    }

    if (serverProcessor_)
    {
        for (auto cookie : sHooks_)
        {
            serverProcessor_->unregisterHandler(cookie);
        }
    }

    if (clientProcessor_)
    {
        for (auto cookie : cHooks_)
        {
            clientProcessor_->unregisterHandler(cookie);
        }
    }

    delete serverProcessor_;
    delete clientProcessor_;
}

bool ConnectionBase::isConnected()
{
    if (server_ == NULL && client_ == NULL)
    {
        return false;
    }

    return (server_ == NULL || server_->isConnected())
        && (client_ == NULL || client_->isConnected());
}

const ErrorState & ConnectionBase::getError()
{
    if (server_ != NULL)
    {
        const ErrorState & serr = server_->getError();
        if (serr.code != ERR_NO_ERROR)
        {
            return serr;
        }
    }

    if (client_ != NULL)
    {
        const ErrorState & clerr = client_->getError();
        if (clerr.code != ERR_NO_ERROR)
        {
            return clerr;
        }
    }

    return error_;
}

bool ConnectionBase::send(FragmentBase *fragment)
{
    if (server_ == nullptr || ! server_->isConnected())
    {
        return false;
    }

    if (clientProcessor_)
    {
        clientProcessor_->process(fragment);
    }

#if PW_SERVER_VERSION >= 1700
    if (GIEncodeValue != 0)
    {
        fragment->encode(GIEncodeValue ^ GIEncodeValue2);
    }
#endif

    barray stream = fragment->assemble();
    if (fragment->isOk())
    {
    #ifdef _DEBUG
        if (logfile_)
        {
            logfile_ << _T("=>") << fragment << endl;
        }
    #endif
        return server_->write(stream);
    }
    else
    {
        return false;
    }
}

bool ConnectionBase::send(const barray & data)
{
    return server_->write(data);
}

bool ConnectionBase::receive()
{
    // return connection status after channels are processed so last message has chance to be processed

    if (server_ && serverProcessor_ && serverFactory_)
    {
        processChannel(server_, serverProcessor_, serverFactory_);
    }
    if (client_ && clientProcessor_ && clientFactory_)
    {
        processChannel(client_, clientProcessor_, clientFactory_);
    }

    return ! ((server_ != nullptr && !server_->isConnected())
           || (client_ != nullptr && !client_->isConnected()));
}

bool ConnectionBase::processChannel( NetChannel        *channel
                                   , FragmentProcessor *processor
                                   , FragmentFactory   *factory)
{
    barray data;

    if ( ! channel->peek(data))
    {
        Log("Stream read failed");
        return false;
    }

    if (data.size() == 0)
    {
        return true;
    }

    barray::const_iterator  start = data.begin();
    barray::const_iterator  curPos = start;

    try
    {
        while (curPos != data.end())
        {
            barray::const_iterator newPos = curPos;

            std::shared_ptr<FragmentBase> f(factory->create(data, newPos));

            if (f && f->isOk())
            {
                curPos = newPos;

                if ( ! f->isParsed() )
                {
                    try
                    {
                        f->parse();
                    }
                    catch (...)
                    {
                        errdump(f.get());
                        continue;
                    }
                }

#ifdef _DEBUG
                if (logfile_)
                {
                    logfile_ << _T("<=") << f.get() << endl;
                }
#endif

                processor->process(f.get());
            }
            else
            {
                // We couldn't form the full packet, most
                // probably it caused by splitted stream
                Log("Can't form packet now, will continue");
                break;
            }
        }
    }
    catch (FragmentException &)
    {
        Log("Fragment Exception!");
        processor->processError(data);
    }
    catch(...)
    {
        Log("Unknown Exception!");
        processor->processError(data);
        throw;
    }

    channel->read(curPos - start);

    return true;
}

ConnectionBase & ConnectionBase::unbindHandler(Cookie & cookie)
{
    if (cookie.empty())
    {
        return *this;
    }

    switch (cookie.first)
    {
        case CookieServer:
            serverProcessor_->unregisterHandler(cookie.second);
            break;
        case CookieClient:
            clientProcessor_->unregisterHandler(cookie.second);
            break;
        default:
            assert(0);
    }

    return *this;
}

ConnectionBase & ConnectionBase::unbindHandlers(std::vector<Cookie> & cookies)
{
    for (std::vector<Cookie>::iterator it = cookies.begin();
        it != cookies.end(); ++it)
    {
        unbindHandler(*it);
    }

    cookies.clear();
    return *this;
}

//

Connection::Connection(bool safeMode)
{
    if (safeMode)
    {
        return;
    }

    serverProcessor_ = new FragmentProcessor();
    serverFactory_   = &serverdata::fragmentFactory;
    clientProcessor_ = new FragmentProcessor();
    clientFactory_   = &clientdata::fragmentFactory;

    serverGiProcessor_ = new FragmentProcessor();
    clientGiProcessor_ = new FragmentProcessor();

    sHooks_.push_back(serverProcessor_->registerHandler(serverdata::FragmentGameinfoSet::ID
                                                       , std::bind(&fragmentCollectionHandler<serverdata::FragmentGameinfoSet>
                                                                     , _1
                                                                     , serverGiProcessor_)));
    sHooks_.push_back(serverProcessor_->registerHandler(serverdata::FragmentArray::ID
                                                       , std::bind(&fragmentCollectionHandler<serverdata::FragmentArray>
                                                                     , _1
                                                                     , serverProcessor_)));
    sHooks_.push_back(serverProcessor_->registerHandler(serverdata::FragmentArrayPacked::ID
                                                       , std::bind(&fragmentCollectionHandler<serverdata::FragmentArrayPacked>
                                                                     , _1
                                                                     , serverProcessor_)));

    cHooks_.push_back(clientProcessor_->registerHandler(clientdata::FragmentGameinfoSet::ID
                                                       , std::bind(&fragmentCollectionHandler<clientdata::FragmentGameinfoSet>
                                                                     , _1
                                                                     , clientGiProcessor_)));
    cHooks_.push_back(clientProcessor_->registerHandler(clientdata::FragmentArray::ID
                                                       , std::bind(&fragmentCollectionHandler<clientdata::FragmentArray>
                                                                     , _1
                                                                     , clientProcessor_)));
#if PW_SERVER_VERSION >= 1700
    AccountInfoHandler_ = bindServerHandler(std::function<void(const serverdata::AccInfoImpl *)>
                                            (std::bind(&Connection::setAccountId, this, _1))
    );
#endif
}

Connection::~Connection()
{
    for (auto cookie : sHooks_)
    {
        serverProcessor_->unregisterHandler(cookie);
    }

    for (auto cookie : cHooks_)
    {
        clientProcessor_->unregisterHandler(cookie);
    }

    delete serverGiProcessor_;
    delete clientGiProcessor_;
}

Connection & Connection::unbindHandler(Cookie & cookie)
{
    if (cookie.empty())
    {
        return *this;
    }

    switch (cookie.first)
    {
        case CookieServerGi:
            serverGiProcessor_->unregisterHandler(cookie.second);
            break;
        case CookieClientGi:
            clientGiProcessor_->unregisterHandler(cookie.second);
            break;
        default:
            ConnectionBase::unbindHandler(cookie);
            break;
    }
    return *this;
}

void Connection::setAccountId(const serverdata::AccInfoImpl * f)
{
#if PW_SERVER_VERSION >= 1700
    setGiIdEncodeVal(f->accId);
#else
    (void)f;
#endif
}
