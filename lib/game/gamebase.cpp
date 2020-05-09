
#include "stdafx.h"

#include "boost/signals.hpp"

#include "game/gamebase.h"
#include "netdata/connectiontcp.h"


GameBase::GameBase()
    : game_(new Game())
    , sleepTime_(1000 /*sec*/ / 20 /*ticks per second*/)
{
    game_->sRequestCharSel.connect(std::bind(boost::ref(sRequestCharSel), _1));
    game_->sEvent.connect(std::bind(boost::ref(sGameEvent), _1));
}

GameBase::~GameBase()
{
    stop(true);
}

bool GameBase::charSelect(std::wstring charName)
{
    if (!game_)
    {
        return false;
    }

    AutoLock lock(gameLock_);

    Account::Account & account = game_->getAccount();
    return account.setCurrentChar(charName);
}

void GameBase::start( std::wstring login
                    , std::wstring password
                    , std::wstring server
                    , bool forced)
{
    login_ = wstrToStr(login);
    password_ = wstrToStr(password);
    server_ = server;
    forced_ = forced;

    run();
}

void GameBase::stop(bool wait /*= true*/)
{
    stopThread(wait);

    login_.clear();
    password_.clear();
    server_.clear();
}

void GameBase::stop() // Thread inherited
{
    stop(true);
}

void GameBase::operator()()
{
    if (login_.empty() || password_.empty() || server_.empty())
    {
        sError(L"Incorrect account credentials");
        sDisconnected();
        return;
    }

    try { do {

        if ( ! beforeConnection())
        {
            sError(L"Invalid game data setup");
            break;
        }

        game_->init();

        ConnectionTcp *conn = new ConnectionTcp();
        std::shared_ptr<Connection>   connection(conn);

        if ( ! conn->open(login_, password_, server_, forced_))
        {
            sError(connection->getError().message);
            break;
        }


        // wait until channel gets connected or failed

        while (WaitForSingleObject(stopEvent_, sleepTime_) == WAIT_TIMEOUT
                && ! connection->isConnected())
        {
            const ErrorState & e = connection->getError();
            if (e.code == ERR_NO_ERROR)     // check if there's actual error or we're waiting to be connected
            {
                continue;
            }
            else
            {
                sError(e.message);
                break;
            }
        }

        if ( ! connection->isConnected())
        {
            break;
        }

        sConnected();

        game_->bind(connection);
        game_->timer()->sync(0);
        game_->timer()->serverSync(0, 0);

        std::shared_ptr<ITask> taskRoot = constructTask();
        ITask::State status;

        if ( ! execute(connection, taskRoot, status))
        {
            taskRoot->stop();
        }
        else
        {
            sCompleted();
        }
    } while (0);
    }
    catch (coreExceptionCode & e)
    {
        WCHAR tmp[32], tmp2[8];
        wcscpy_s(tmp, L"Core exception: ");
        _itow_s((int)e, tmp2, 10);
        wcscat_s(tmp, tmp2);

        sError(tmp);
    }
    catch (...)
    {
        sError(L"Unknown exception");
        game_->unbind();
        throw;
    }

    game_->unbind();

    sDisconnected();

    Log("Thread End");
}

bool GameBase::execute( std::shared_ptr<Connection> connection
                      , std::shared_ptr<ITask>      taskRoot
                      , ITask::State                 & status)
{
    bool        connectionFail = false;
    DWORD       waitResult     = WAIT_TIMEOUT;

    status = ITask::TASK_NONE;

    do
    {
        gameLock_.lock();

        try
        {
            if ( ! connection->receive())
            {
                gameLock_.unlock();
                Log("Connection closed: %ls ", wstrToTStr(connection->getError().message).c_str());
                sError(connection->getError().message);
                connectionFail = true;
                break;
            }

            game_->timer()->tick();
            Timestamp now = game_->timer()->time();

            game_->tick(now);
            status = taskRoot->tick(now);
        }
        catch (...)
        {
            gameLock_.unlock();
            throw;
        }

        gameLock_.unlock();

        waitResult = WaitForSingleObject(stopEvent_, sleepTime_);

    } while (status == ITask::TASK_RUNNING && waitResult == WAIT_TIMEOUT);

    if (connectionFail)
    {
        Log("Task terminated due to connection error");
        return false;
    }
    else if (waitResult != WAIT_TIMEOUT)
    {
        Log("Task stopped by request");
        return false;
    }
    else
    {
        Log("Task stopped by status");
    }

    Log("Task result: %i", status);
    return true;
}
