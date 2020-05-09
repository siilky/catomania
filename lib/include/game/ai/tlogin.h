#ifndef tlogin_h
#define tlogin_h

#include "..\account.h"

// Conditions

struct CharSelectedCond
{
    bool operator ()(std::shared_ptr<Game> game) const
    {
        Account::Account & account = game->getAccount();
        Account::CharInfo unused;
        return account.getCurrentChar(unused);
    }
};

struct GotCharAttrs
{
    bool operator ()(std::shared_ptr<Game> game) const
    {
        Player & p = game->getPlayer();
        Player::Availability avail = p.getAvailability();

        return (avail & Player::BASIC_INFO) != 0
            && (avail & Player::POS_INFO) != 0
            && (avail & Player::ATTR_INFO) != 0;
    }
};

struct GotAccountCond
{
    bool operator ()(std::shared_ptr<Game> game) const
    {
        Account::Account & account = game->getAccount();
        DWORD unused;
        return account.getAccountId(unused);
    }
};

struct NoCharsOnAccountCond
{
    bool operator ()(std::shared_ptr<Game> game) const
    {
        Account::Account & account = game->getAccount();
        const Account::CharList charlist = account.getCharList();
        if (charlist.empty())
        {
            return true;
        }
        return false;
    }
};

struct GotLockInfo
{
    bool operator ()(std::shared_ptr<Game> game) const
    {
        Player & p = game->getPlayer();
        return p.lockState() != Player::LockStateUnknown;
    }
};

struct IsLockExpired
{
    bool operator ()(std::shared_ptr<Game> game) const
    {
        Player & p = game->getPlayer();
        return p.lockState() == Player::LockStateOff;
    }
};

// Tasks

class TGetAccountCharlist : public ITask
{
    // Get character list of an account

public:
    TGetAccountCharlist(std::shared_ptr<Game> game);

protected:
    virtual State onStarted();
    virtual void onStopped();
    virtual State onCompleted();

private:
    bool getNextRole(int slot);
    void onRoleList(const serverdata::FragmentRoleListRe *f);

    std::shared_ptr<Game>     game_;
    Connection::Cookie          cAccInfo, cRoleList;
};

class TGetInventory : public ITask
{
    // get my inventory
public:
    TGetInventory(std::shared_ptr<Game> game);;

protected:
    virtual State onStarted();

private:
    struct GotInventoryCond
    {
        bool operator ()(std::shared_ptr<Game> game) const
        {
            const Inventory & inv = game->getInventory();
            return (inv.getAvailability() & ALL_INFO) == ALL_INFO;
        }
    };

    std::shared_ptr<Game> game_;
};


class TPerformLogin : public IComplexTask
{
    // Wait for account ID
    // Get character list
    // wait for charselect
    // perform character login
    // Wait for character attributes to arrive

public:
    TPerformLogin(std::shared_ptr<Game> game);

protected:
    virtual void onStopped();

private:
    std::shared_ptr<Game>     game_;
};

class TPerformIngameLogin : public IComplexTask
{
    // Wait for character attributes to arrive

public:
    TPerformIngameLogin(std::shared_ptr<Game> game);

private:
    std::shared_ptr<Game>     game_;
};

#endif
