#ifndef gamebase_h
#define gamebase_h

#include "game/game.h"
#include "game/account.h"
#include "game/ai/task.h"
#include "thread.h"

class GameBase : protected Thread
{
public:
    GameBase();
    virtual ~GameBase();

    void start( std::wstring login
              , std::wstring password
              , std::wstring server
              , bool forced);

    void stop(bool wait = true);
    virtual void stop(); // Thread inherited

    bool charSelect(std::wstring charName);

    // signals

    boost::signal<void(const Account::CharList & charlist)> sRequestCharSel;
    boost::signal<void()>                                   sConnected;
    boost::signal<void(Game::EventType)>                    sGameEvent;
    boost::signal<void()>                                   sDisconnected;
    boost::signal<void()>                                   sCompleted;
    boost::signal<void(const std::wstring & /*error*/)>     sError;

protected:
    virtual bool beforeConnection()
    {
        return true;
    }

    virtual std::shared_ptr<ITask> constructTask() = 0;

    void operator()();

    bool execute( std::shared_ptr<Connection> connection
                , std::shared_ptr<ITask>      taskRoot
                , ITask::State                 & status);

    //

    std::string     login_;
    std::string     password_;
    std::wstring    server_;
    bool            forced_;

    std::shared_ptr<Game> game_;
    Syncronized             gameLock_;

    unsigned        sleepTime_;
};

#endif