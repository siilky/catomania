#ifndef gamethread_h
#define gamethread_h

#pragma warning(disable : 4512)     // assignment operator could not be generated

#include "common.h"
#include "error.h"

#include "qlib/game/gamebase.h"
#include "qlib/eventq.h"
#include "game/ai/condition.h"
#include "game/data/elements.h"
#include "game/data/itemprops.h"
#include "game/data/items.h"
#include "game/account.h"
#include "game/inventory.h"

class MarketItemSetup;
class ITask;
class TMove;
class TOpenMarket;
class TRandomDelay;
class TRandomDelay;

// class runs in GameBase thread
// это последний рубеж обороны от потока GameBase , все обращения должны проходить через GameBase::lock_
// + все сигналы из game:: надо заворачивать в Qt сигналы

class GameThread
    : public qlib::GameBase
{
    Q_OBJECT
public:
    explicit GameThread();  // cannot have parent while GameBase is running on self thread

    void setElements(const elements::ItemListCollection & items);

    std::shared_ptr<ItemPropsFactory> getPropFactory() const;

    void setOptions( unsigned  closeDelayFixed, unsigned  closeDelayRandom
                   , unsigned  openDelayFixed, unsigned  openDelayRandom
                   , bool doOnlineRegistration
                   , unsigned lowConverterValue, unsigned highConverterValue);

    bool openMarket(bool delayed = false);
    void closeMarket(bool delayed = false);
    void reopenMarket();
    bool isMarketOpened() const
    {
        return marketOpened_;
    }

    void                getIpInfo(DWORD & lastLoginTs, DWORD & lastIp, DWORD & currentIp);
    int                 myLevel();

    Account::CharList   getCharList();
    bool                getCurrentChar(::Account::CharInfo & info);
    unsigned            getMoney(unsigned *maxMoney = 0);
    unsigned            getBanknotes();
    unsigned            getGold();

    std::vector<InventoryItem>  getBag();       // returns full set with empty slots set id=-1
    InventoryItem               getCatshop();
    void                        getCatshopLimits(const InventoryItem & catshop, unsigned & maxSlots, unsigned & maxChars);     // = 0 if no catshop
    std::vector<MarketItem>     getMarket();
    CoordEx                     getPlayerPos();

    void setMarketSetup(const std::vector<MarketItemSetup> & items);
    void setMarketTitle(const std::wstring & shopTitle);

    QString resolvePlayerName(DWORD playerId);
    DWORD resolvePlayerId(const QString & playerName);

    unsigned lockTime();

    std::vector<CommiShop> getCommi();
    time_t commiLastUpdated();

public Q_SLOTS:
    bool selectChar(const QString & charName);

    void revive();

    void acceptTrade(unsigned tradeId);
    void rejectTrade(unsigned tradeId);
    void discardTrade();
    void addTradeItem(unsigned inventorySlot, unsigned count);
    void addTradeMoney(unsigned money);
    void removeTradeItem(unsigned inventorySlot, unsigned count);
    void removeTradeMoney(unsigned money);
    void confirmTrade();
    void submitTrade();

    void setLockTime(unsigned seconds);

    void buyCatshop(unsigned itemId, unsigned count, unsigned shopIndex);
    void equipCatShop(unsigned inventorySlot);

    void sendPrivateMessage(const QString & playerName, const QString & text);
    void sendPublicMessage(const QString & text);
    void sendGroupMessage(const QString & text);
    void sendGuildMessage(const QString & text);

    void updateCommi(bool trackProgress);

    void swapItems(unsigned slot1, unsigned slot2);
    void moveItems(unsigned slot1, unsigned slot2, unsigned count);

Q_SIGNALS:
    void loggedIn();
    void lockInfo(bool isOn, unsigned long long remaining);

    void marketIsEmpty();
    void marketMove();
    void marketReopen();
    void marketError();

    void playerResolved(unsigned playerId, const QString & name);

private Q_SLOTS:
    void converNotes();

private:
    bool invokeInThread(const char *function)
    {
        if (!isRunning())
        {
            return true;    // silently eat call
        }
        if (thread() != QThread::currentThread())
        {
            bool result = QMetaObject::invokeMethod(this, function, Qt::QueuedConnection);
            Q_ASSERT(result);
            return true;
        }
        return false;
    }

    template<typename A1>
    bool invokeInThread(const char *function, A1 arg1)
    {
        if (!isRunning())
        {
            return true;    // silently eat call
        }
        if (thread() != QThread::currentThread())
        {
            bool result = QMetaObject::invokeMethod(this, function, Qt::QueuedConnection
                                                    , QArgument<A1>(typeid(arg1).name(), arg1)
            );
            Q_ASSERT(result);
            return true;
        }
        return false;
    }

    template<typename A1, typename A2>
    bool invokeInThread(const char *function, A1 arg1, A2 arg2)
    {
        if (!isRunning())
        {
            return true;    // silently eat call 
        }
        if (thread() != QThread::currentThread())
        {
            bool result = QMetaObject::invokeMethod(this, function, Qt::QueuedConnection
                                                    , QArgument<A1>(typeid(arg1).name(), arg1)
                                                    , QArgument<A2>(typeid(arg2).name(), arg2)
            );
            Q_ASSERT(result);
            return true;
        }
        return false;
    }

    template<typename A1, typename A2, typename A3>
    bool invokeInThread(const char *function, A1 arg1, A2 arg2, A3 arg3)
    {
        if (!isRunning())
        {
            return true;    // silently eat call
        }
        if (thread() != QThread::currentThread())
        {
            bool result = QMetaObject::invokeMethod(this, function, Qt::QueuedConnection
                                                    , QArgument<A1>(typeid(arg1).name(), arg1)
                                                    , QArgument<A2>(typeid(arg2).name(), arg2)
                                                    , QArgument<A3>(typeid(arg2).name(), arg3)
            );
            Q_ASSERT(result);
            return true;
        }
        return false;
    }

    virtual bool beforeConnection();
    virtual std::shared_ptr<ITask> constructTask() override;

    void setNoState();
    void setOpeinigState();
    void setClosingState();

    void setMoveMarket();
    void setReopenMarket();
    bool chkTimeCondition();
    void onlineRegistration();
    bool isOnlineRegAvailable();

    bool onLockTimer();

    void onBaseEvent(BaseEvent *event);
    void onLoggedIn();

    void rebuildMarketList();

    //

    elements::ItemListCollection    elements_;


    std::vector<MarketItemSetup>    marketSetup_;
    std::wstring                    marketTitle_;

    unsigned closeDelayFixed_, closeDelayRandom_;
    unsigned openDelayFixed_, openDelayRandom_;
    unsigned lowConverterValue_;
    unsigned highConverterValue_;
    bool    doWandering_;
    bool    doOnlineRegistration_;

    std::shared_ptr<TOpenMarket>  tOpenMarket_;
    std::shared_ptr<TRandomDelay> tOpenDelay_;    // delay before opening
    std::shared_ptr<TRandomDelay> tCloseDelay_;   // delay before closing
    //std::shared_ptr<TMove>        tMoveMarket_;

    Trigger ctOpenMarket_;
    Trigger ctCloseMarket_;
    //Trigger ctMoveMarket_;
    Trigger ctMarketClosedExtern_;

    Coord3D marketPos_;

    bool isOpening_;
    bool isClosing_;
    bool marketOpened_;
    bool isWaitingForConversion_;
};

#endif
