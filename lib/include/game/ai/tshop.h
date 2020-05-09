#ifndef tshop_h
#define tshop_h

#include "netdata\fragments_giserver.h"

class TMarketSkill : public ITask
{
public:
    TMarketSkill(std::shared_ptr<Game> game);

protected:
    virtual State onStarted();
    virtual void onStopped();

private:
    void onMarketSkill(const serverdata::FragmentGiPersonalMarketAvailable *);
    void onGameError(const serverdata::FragmentGiErrorMsg *);

    std::shared_ptr<Game> game_;
    Connection::Cookie      cMarketSkill_, cErrorMsg_;
};


class TOpenMarket : public ITask
{
public:
    TOpenMarket(std::shared_ptr<Game> game);

    typedef clientdata::FragmentGiOpenMarket::Item      MarketItem;

    std::wstring            name;
    std::vector<MarketItem> items;

protected:
    virtual State onStarted();
    virtual void onStopped();

private:
    void onOpenMarket(const serverdata::FragmentGiSelfOpenMarket *f);
    void onCloseMarket(const serverdata::FragmentGiPlayerCancelMarket *f);

    std::shared_ptr<Game> game_;
    Connection::Cookie      cOpenMarket_, cCloseMarket_;
};


class TCloseMarket : public ITask
{
public:
    TCloseMarket(std::shared_ptr<Game> game);

protected:
    virtual State onStarted();
    virtual void onStopped();

private:
    void onCloseMarket(const serverdata::FragmentGiPlayerCancelMarket *f);

    std::shared_ptr<Game> game_;
    Connection::Cookie      cCloseMarket_;
};


class TOpenPlayerMarket : public ITask
{
public:
    TOpenPlayerMarket(std::shared_ptr<Game> game);

    DWORD   characterID;
    DWORD   chk;
    std::vector<struct serverdata::PlayerMarketInfoImpl::ItemData>  items;

protected:
    virtual State onStarted();
    virtual void onStopped();

private:
    void onMarketInfo(const serverdata::FragmentGiPlayerMarketInfo *f);

    std::shared_ptr<Game> game_;
    Connection::Cookie      cMarketInfo_;
};


#endif
