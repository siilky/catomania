
#include "stdafx.h"

#include "log.h"
#include "common.h"
#include "netdata/fragment_manager.h"


//------------------------------------------------------------------------------
FragmentProcessor::Cookie FragmentProcessor::registerHandler(unsigned id, FragmentHandler handler)
{
    lock();

    cookieCounter_ ++;

    if (inProcessing_ > 0)
    {
        addedList_.insert(std::make_pair(id, std::make_pair(cookieCounter_, handler)));
    }
    else
    {
        handlers_.insert(std::make_pair(id, std::make_pair(cookieCounter_, handler)));
    }

    Cookie rez(id, cookieCounter_);

    unlock();

    return rez;
}

void FragmentProcessor::unregisterHandler(Cookie & cookie)
{
    std::pair< FragmentHandlers::iterator
             , FragmentHandlers::iterator >     range;

    lock();

    if (inProcessing_ > 0)
    {
        eraseHandler(addedList_, cookie);
        removedList_.insert(cookie);
    }
    else
    {
        eraseHandler(handlers_, cookie);
    }

    cookie.reset();

    unlock();
}

void FragmentProcessor::unregisterHandlers(std::vector<Cookie> & cookies)
{
    lock();

    for (std::vector<Cookie>::iterator it = cookies.begin();
        it != cookies.end(); ++it)
    {
        unregisterHandler(*it);
    }

    cookies.clear();

    unlock();
}

void FragmentProcessor::process(FragmentBase *fragment)
{
    unsigned int    id = fragment->getId();

    lock();
    inProcessing_++;

    try
    {
        if (mirrorHandler_)
        {
            mirrorHandler_(fragment);
        }

        std::pair< FragmentHandlers::iterator, FragmentHandlers::iterator >
            range = handlers_.equal_range(id);

        if (range.first != range.second)
        {
            if (fragment->isParsed())
            {
                while(range.first != range.second)
                {
                    range.first->second.second(fragment);
                    ++(range.first);
                }
            }
            else if (errorHandler_)
            {
                errorHandler_(fragment->bytes());
            }
        }
        else if (defaultHandler_)
        {
            defaultHandler_(fragment);
        }
    }
    catch (...)
    {
        inProcessing_--;
        unlock();
        throw;
    }

    inProcessing_--;

    // process added/deleted handlers

    if (inProcessing_ == 0)
    {
        for (FragmentHandlers::const_iterator it = addedList_.begin();
            it != addedList_.end(); ++it)
        {
            handlers_.insert(*it);
        }
        addedList_.clear();

        for (std::set<Cookie>::iterator it = removedList_.begin();
            it != removedList_.end(); ++it)
        {
            eraseHandler(handlers_, *it);
        }
        removedList_.clear();
    }

    unlock();
}

void FragmentProcessor::eraseHandler(FragmentHandlers & map, Cookie cookie)
{
    std::pair< FragmentHandlers::iterator
             , FragmentHandlers::iterator >     range;

    range = map.equal_range(cookie.fragmentId);
    while (range.first != range.second)
    {
        if (range.first->second.first == cookie.cookieCtr)
        {
            map.erase(range.first);
            return;
        }
        ++(range.first);
    }
}

//------------------------------------------------------------------------------

/*
void FragmentManager::start(NetChannel *channel, FragmentFactory *factory)
{
    channel_ = channel;
    factory_ = factory;

    run();
}

void FragmentManager::operator()()
{
    bool    stopThread = false;
    HANDLE  waitObjects[] = { channel_->waitableRead()
                            , stopEvent_};

    while ( ! stopThread)
    {
        DWORD waitRez = WaitForMultipleObjects(sizeof_array(waitObjects), waitObjects, FALSE, INFINITE);

        switch(waitRez - (waitRez % MAXIMUM_WAIT_OBJECTS))
        {
            case WAIT_OBJECT_0:
            {
                switch(waitRez % MAXIMUM_WAIT_OBJECTS)
                {
                    case 0:     // data is available for reading
                    {
                        barray data;
                        
                        if ( ! channel_->peek(data))
                        {
                            Log("Stream read failed");
                            break;
                        }

                        if (data.size() == 0)
                        {
                            Log("read returned no data");
                            break;
                        }

                        barray::const_iterator  start = data.begin();
                        barray::const_iterator  curPos = start;

                        try
                        {
                            while (curPos != data.end())
                            {
                                barray::const_iterator newPos = curPos;
                                FragmentBase *f = factory_->create(data, newPos);

                                if (f != NULL && f->isOk())
                                {
                                    curPos = newPos;

                                    if ( ! f->isParsed() )
                                    {
                                        try
                                        {
                                            f->parse();
                                        }
                                        catch (std::exception &)
                                        {
                                            errdump(f);
                                        }
                                    }

                                    #if defined(DEBUG)
                                        Sleep(50);
                                    #endif
                                    processor_.process(f);
                                }
                                else
                                {
                                    // We can't form the full packet, most
                                    // probably it caused by stream split
                                    Log("Can't form packet now, will continue");
                                    break;
                                }

                                delete f;
                            }
                        }
                        catch (FragmentException &)
                        {
                            Log("Fragment Exception!");
                            processor_.processError(data);
                        }
                        catch(...)
                        {
                            Log("Unknown Exception!");
                            processor_.processError(data);
                            throw;
                        }

                        channel_->read(curPos - start);
                        break;
                    }

                    case 1:     // stop event
                        stopThread = true;
                        break;

                    default:
                        assert(0);
                }
                break;
            }

            case WAIT_ABANDONED_0:
                stopThread = true;
                break;

            default:
                assert(0);
        }
    }
}

bool FragmentManager::sendFragment(FragmentBase *fragment)
{
    barray stream = fragment->assemble();

    if (fragment->isOk() && oppositeEndpoint_ != NULL)
    {
        oppositeEndpoint_->processor_.process(fragment);
    }

    if ( ! stream.empty())
    {
        return channel_->write(stream);
    }
    return true;
}
*/