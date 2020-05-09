#ifndef tingame_h
#define tingame_h

#include "game/ai/task.h"
#include "netdata/fragments_server.h"
#include "netdata/connection.h"
#include "game/game.h"


class TWaitKeepalive : public ITask
{
    // Wait for server keepalive fragment

public:
    TWaitKeepalive(std::shared_ptr<Game> game);;

protected:
    virtual State onStarted();
    virtual void onStopped();

private:
    void onKeepalive(const serverdata::FragmentKeepalive * /*f*/);

    Connection::Cookie      cKeepalive_;
    std::shared_ptr<Game> game_;
};


class TGetMyId : public TWait
{
public:
    TGetMyId(std::shared_ptr<Game> game);

protected:
    virtual State onStarted();
    virtual void onStopped();

private:
    void onEmotionRe(const serverdata::FragmentGiDoEmotion *f);

    Connection::Cookie      cEmotionRe_;
    WORD                    seed_;

    std::shared_ptr<Game> game_;
};


class TGetMyAttrs : public ITask
{
public:
    TGetMyAttrs(std::shared_ptr<Game> game);;

protected:
    virtual State onStarted();
    virtual void onStopped();

private:
    void onCharAttrs(const serverdata::FragmentGiCharacters *f);

    Connection::Cookie      cCharacters_;
    std::shared_ptr<Game> game_;
};


class TGetMyInfo : public ITask
{
    // get my info
public:
    TGetMyInfo(std::shared_ptr<Game> game);;

protected:
    virtual State onStarted();
    virtual void onStopped();

private:
    void onBaseInfo(const serverdata::FragmentPlayerBaseInfoRe *f);

    Connection::Cookie      cBaseInfo_;
    std::shared_ptr<Game> game_;
};


class TWaitEnterGame : public ITask
{
    // 

public:
    TWaitEnterGame(std::shared_ptr<Game> game);;

protected:
    virtual State onStarted();

    virtual void onStopped();

private:
    void onEnterGame(const clientdata::FragmentEnterWorld * /*f*/);

    Connection::Cookie      cEnterGame_;
    std::shared_ptr<Game> game_;
};


class TWaitPlayerRestart : public ITask
{
    // 

public:
    TWaitPlayerRestart(std::shared_ptr<Game> game);;

protected:
    virtual State onStarted();
    virtual void onStopped();

private:
    void onPlayerLogout(const serverdata::FragmentPlayerLogout * /*f*/);
    void onRoleList(const serverdata::FragmentRoleListRe * /*f*/);

    Connection::Cookie      cPlayerLogout_, cRoleList_;
    std::shared_ptr<Game> game_;
};


class TGetBattleMap : public ITask
{
    // get my info
public:
    TGetBattleMap(std::shared_ptr<Game> game);;

protected:
    virtual State onStarted();
    virtual void onStopped();

private:
    void onBattleGetMapRe(const serverdata::FragmentBattleGetMapRe * /*f*/);

    Connection::Cookie      cTwMap_;
    std::shared_ptr<Game> game_;
};


// Conditions

struct GotAccountInfo
{
    bool operator ()(std::shared_ptr<Game> game) const
    {
        Account::Account & account = game->getAccount();
        const Account::CharList & cl = account.getCharList();
        return ! cl.empty();
    }
};


#endif
