
#include "stdafx.h"

#pragma warning(disable : 4512)     // assignment operator could not be generated
#pragma warning(disable : 4127)     // conditional expression is constant

// game prereqs
#include <iomanip>
#include <fstream>

#include "boost/signal.hpp"
#include <boost/bind.hpp>

#include "netio/connectiontcp.h"
#include "netdata/connectionx.h"
#include "game/game.h"
#include "game/ai/task.h"
#include "game/gamebase.h"
#include "netdata/connection.h"


namespace qlib
{

ConnectionGeneratorTcp::ConnectionGeneratorTcp( const std::wstring  & login
                                              , const std::wstring  & password
                                              , const std::wstring  & server
                                              , const QNetworkProxy & proxy
                                              , bool                  forced /*= false*/)
    : server_(server)
    , proxy_(proxy)
    , forced_(forced)
    , connection_(0)
{
    login_ = wstrToStr(login);
    password_ = wstrToStr(password);
}

ConnectionGeneratorTcp::~ConnectionGeneratorTcp()
{
}

bool ConnectionGeneratorTcp::isValid() const
{
    return !login_.empty() && !password_.empty() && !server_.empty();
}

std::shared_ptr<Connection> ConnectionGeneratorTcp::get()
{
    // we should create connection in thread calling "get"
    if (!conn_)
    {
        connection_ = new ConnectionTcp();
        conn_ = std::shared_ptr<Connection>(connection_);
    }
    return conn_;
}

bool ConnectionGeneratorTcp::open()
{
    return connection_->open(login_, password_, server_, forced_, proxy_);
}

void ConnectionGeneratorTcp::close()
{
    conn_.reset();
}

//

ConnectionGeneratorInprocess::ConnectionGeneratorInprocess(unsigned processId)
    : processId_(processId)
    , connection_(0)
{
}

ConnectionGeneratorInprocess::~ConnectionGeneratorInprocess()
{
}

bool ConnectionGeneratorInprocess::isValid() const
{
    return processId_ != 0;
}

std::shared_ptr<Connection> ConnectionGeneratorInprocess::get()
{
    if (!conn_)
    {
        connection_ = new ConnectionX();
        conn_ = std::shared_ptr<Connection>(connection_);
    }
    return conn_;
}

bool ConnectionGeneratorInprocess::open()
{
    return connection_->open(processId_);
}

void ConnectionGeneratorInprocess::close()
{
    conn_.reset();
}

//

long (__stdcall *exceptionHandler)(EXCEPTION_POINTERS* pep) = nullptr;

GameBase::GameBase()
    : game_(new Game())
    , sleepTime_(1000 /*sec*/ / 10 /*ticks per second*/)
    , gameLock_(QMutex::Recursive)
{
    moveToThread(this);

    qRegisterMetaType<GameEvent>("GameEvent");
    game_->sBaseEvent.connect(boost::bind(&GameBase::onBaseEvent, this, boost::placeholders::_1));
}

GameBase::~GameBase()
{
    stopThread(true);
}

void GameBase::start(QSharedPointer<ConnectionGenerator> connectionGen)
{
    if (isRunning())
    {
        return;
    }

    connectionGen_ = connectionGen;

    exit_ = false;
    QThread::start();
}

void GameBase::stop(bool wait /*= true*/)
{
    stopThread(wait);
}

bool GameBase::selectChar(const QString & charName)
{
    assert(QThread::currentThread() == thread());

    QMutexLocker lock(&gameLock_);

    Account::Account & account = game_->getAccount();
    if (!account.selectChar(charName.toStdWString()))
    {
        error(tr("Failed to select character %1").arg(charName));
        return false;
    }
    return true;
}

//

static int exceptionFilter(EXCEPTION_POINTERS *ep)
{
    if (exceptionHandler)
    {
        exceptionHandler(ep);
    }

    return EXCEPTION_EXECUTE_HANDLER;
}

static QString exString("Exception in thread");

void GameBase::run()
{
    // catch for SEH
    __try
    {
        runIntl();
    }
    __except (exceptionFilter(GetExceptionInformation()))
    {
        error(exString);
    }

    connectionGen_->close();
    game_->unbind();
    disconnected();

    Log("GameBase End");
}

void GameBase::runIntl()
{
    if (!connectionGen_->isValid())
    {
        error(tr("Incorrect account credentials"));
        disconnected();
        return;
    }

    do {
        if ( ! beforeConnection())
        {
            break;
        }

        game_->init();
        game_->timer()->sync(0);
        game_->timer()->serverSync(0, 0);

        // TBD: shared_ptr тут несколько излишен
        std::shared_ptr<Connection> connection(connectionGen_->get());

        game_->bind(connection);

        if ( ! connectionGen_->open())
        {
            error(QString::fromStdWString(connection->getError().message));
            break;
        }

        // wait until channel gets connected or failed
        // TBD: ожидание соединения должно быть выражено более явно

        // remove any posted events outside of thread lifetime
        QCoreApplication::removePostedEvents(this);

        QEventLoop eventLoop;
        bool running = false;
        do
        {
            running = processEventsTick(eventLoop);
            if ( ! running || connection->isConnected())
            {
                break;
            }

            const ErrorState & e = connection->getError();
            if (e.code == ERR_NO_ERROR)     // check if there's actual error or we're waiting to be connected
            {
                continue;
            }
            else
            {
                Log("Connection failed: %ws", e.message.c_str());
                error(QString::fromStdWString(e.message));
                break;
            }
        } while (running);
        if (!running)
        {
            Log("Exit from event loop on connection");
            break;
        }

        if ( ! connection->isConnected())
        {
            Log("Connection dropped before connected");
            break;
        }

        connected();

        std::shared_ptr<ITask> task(constructTask());
        if ( ! execute(connection, task, eventLoop))
        {
            task->stop();
        }
        else
        {
            completed();
        }
    } while (false);
}

bool GameBase::beforeConnection()
{
    return true;
}

bool GameBase::execute( std::shared_ptr<Connection> connection
                      , std::shared_ptr<ITask>      taskRoot
                      , QEventLoop                  & eventLoop)
{
    bool        connectionFail = false;
    DWORD       waitResult     = WAIT_TIMEOUT;

    ITask::State    status = ITask::TASK_NONE;
    bool running = false;
    do
    {
        gameLock_.lock();

        try
        {
            if ( ! connection->receive())
            {
                gameLock_.unlock();
                Log("Connection closed: %ls ", wstrToTStr(connection->getError().message).c_str());
                error(QString::fromStdWString(connection->getError().message));
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

        running = processEventsTick(eventLoop);

    } while (running && status == ITask::TASK_RUNNING);

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

void GameBase::stopThread(bool doWait)
{
    if (isRunning())
    {
        exit_ = true;
        quit();
        if (doWait && ! wait(10000))
        {
            terminate();
        }
    }
}

bool GameBase::processEventsTick(QEventLoop & eventLoop)
{
    QElapsedTimer timer;
    timer.start();

    eventLoop.processEvents(QEventLoop::AllEvents, sleepTime_);

    if (exit_)
    {
        return false;
    }

    unsigned long elapsed = unsigned long(timer.elapsed());
    if (elapsed < sleepTime_)
    {
        msleep(sleepTime_ - elapsed);
    }

    return true;
}

void GameBase::onBaseEvent(BaseEvent *e)
{
    if (e == 0) return;

    event(GameEvent(e->clone()));
}



}
