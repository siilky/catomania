#ifndef gamebase_h
#define gamebase_h

#include <QThread>
#include <QMutex>
#include <QEventLoop>
#include <QSharedPointer>
#include <QNetworkProxy>

#include "event.h"
#include <memory>

class Game;
class ITask;
class Connection;
class ConnectionX;
class ConnectionTcp;

namespace qlib
{

class ConnectionGenerator
{
public:
    virtual ~ConnectionGenerator()
    {
        close();
    };

    virtual bool isValid() const = 0;
    virtual std::shared_ptr<Connection> get() = 0;
    virtual bool open() = 0;
    virtual void close() {};
};


class ConnectionGeneratorTcp : public ConnectionGenerator
{
public:
    ConnectionGeneratorTcp( const std::wstring  & login
                          , const std::wstring  & password
                          , const std::wstring  & server
                          , const QNetworkProxy & proxy = QNetworkProxy()
                          , bool                  forced = false);
    virtual ~ConnectionGeneratorTcp();

    virtual bool isValid() const;
    virtual std::shared_ptr<Connection> get();
    virtual bool open();
    virtual void close();

private:
    std::string     login_;
    std::string     password_;
    std::wstring    server_;
    QNetworkProxy   proxy_;
    bool            forced_;

    ConnectionTcp   *connection_;       // weak
    std::shared_ptr<Connection>   conn_;
};


class ConnectionGeneratorInprocess : public ConnectionGenerator
{
public:
    ConnectionGeneratorInprocess(unsigned processId);
    virtual ~ConnectionGeneratorInprocess();

    virtual bool isValid() const;
    virtual std::shared_ptr<Connection> get();
    virtual bool open();
    virtual void close();

private:
    unsigned        processId_;
    ConnectionX     *connection_;       // weak
    std::shared_ptr<Connection>   conn_;
};


//

extern long (__stdcall *exceptionHandler)(EXCEPTION_POINTERS* pep);

typedef QSharedPointer<BaseEvent>   GameEvent;
Q_DECLARE_METATYPE(GameEvent);


class GameBase : public QThread
{
    Q_OBJECT
    Q_DISABLE_COPY(GameBase)
public:
    GameBase();   // cannot have parent while running on self thread
    virtual ~GameBase();

    void start(QSharedPointer<ConnectionGenerator> connectionGen_);
    void stop(bool wait = true);

public Q_SLOTS:
    bool selectChar(const QString & charName);

Q_SIGNALS:
    void connected();
    void disconnected();
    void completed();

    void event(const GameEvent & e);    // асинхронный сигнал с евентами.
                                        // Если нужна синхронная реакция на евент - можно использовать прямое соединение к game_->sBaseEvent
    void error(const QString & error);

protected:
    // do any work before connection is initiated.
    // return false if cannot continue
    virtual bool beforeConnection();

    virtual std::shared_ptr<ITask> constructTask() = 0;

    //

    std::shared_ptr<Game> game_;
    QMutex                  gameLock_;

private:
    virtual void run();
    void runIntl();

    void stopThread(bool doWait);
    bool processEventsTick(QEventLoop & eventLoop);

    bool execute( std::shared_ptr<Connection> connection
                , std::shared_ptr<ITask>      taskRoot
                , QEventLoop                    & eventLoop);

    void onBaseEvent(BaseEvent *e);

    QSharedPointer<ConnectionGenerator> connectionGen_;

    unsigned        sleepTime_;
    volatile bool   exit_;
};

}

#endif