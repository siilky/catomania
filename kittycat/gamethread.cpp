
//#pragma warning(disable : 4512)     // assignment operator could not be generated

#include <QTimer>
#include <QMetaMethod>

#include "boost/signal.hpp"
#include <boost/bind.hpp>

#include "common.h"
#include "utils.h"
#include "log.h"
#include "MarketItemSetup.h"
#include "gamethread.h"
#include "netdata/connection.h"
#include "game/data/items.h"
#include "game/game.h"
#include "game/gtime.h"
#include "game/ai/task.h"
#include "game/ai/tlogin.h"
#include "game/ai/tinfo.h"
#include "game/ai/tshop.h"
#include "game/ai/tmove.h"
#include "marketbuilder.h"

#include "wtypes.h"
#include "VMProtectSDK.h"
#include "config.h"


GameThread::GameThread()
    : tCloseDelay_(new TRandomDelay())
    , tOpenDelay_(new TRandomDelay())
    , tOpenMarket_(new TOpenMarket(game_))
    //, tMoveMarket_(new TMove(game_))
    , closeDelayFixed_(0)
    , closeDelayRandom_(0)
    , openDelayFixed_(0)
    , openDelayRandom_(0)
    , doWandering_(false)
    , doOnlineRegistration_(false)
    , lowConverterValue_(0)
    , highConverterValue_(0)
    , marketOpened_(false)
    , isOpening_(false)
    , isClosing_(false)
    , isWaitingForConversion_(false)
{
    game_->sBaseEvent.connect(boost::bind(&GameThread::onBaseEvent, this, boost::placeholders::_1));
#if (GAME_OWNER_TYPE == 1) && !defined(GAME_USING_CLIENT)
    game_->enableGac();
#endif
}

void GameThread::setElements(const elements::ItemListCollection & items)
{
    elements_ = items;
    game_->setElements(items);
}

std::shared_ptr<ItemPropsFactory> GameThread::getPropFactory() const
{
    return game_->getPropFactory();
}

void GameThread::setOptions( unsigned closeDelayFixed, unsigned closeDelayRandom
                           , unsigned openDelayFixed, unsigned openDelayRandom
                           , bool doOnlineRegistration
                           , unsigned lowConverterValue, unsigned highConverterValue)
{
    closeDelayFixed_      = closeDelayFixed * 1000;     // ms
    closeDelayRandom_     = closeDelayRandom * 1000;
    openDelayFixed_       = openDelayFixed * 1000;
    openDelayRandom_      = openDelayRandom * 1000;
    doOnlineRegistration_ = doOnlineRegistration;
    lowConverterValue_    = lowConverterValue;
    highConverterValue_   = highConverterValue;
}

bool GameThread::openMarket(bool delayed)
{
//     store initial point
//     marketPos_ = game_->getPlayer().getPosition();
//     Log("Current point [%.2f %.2f %.2f]"
//         , marketPos_.x()
//         , marketPos_.y()
//         , marketPos_.z());
//         
    rebuildMarketList();
    if (tOpenMarket_->items.size() == 0)
    {
        marketIsEmpty();
        return false;
    }

    if (delayed)
    {
        Log("OpenDelay set");
        tOpenDelay_->fixedPart  = openDelayFixed_;
        tOpenDelay_->randomPart = openDelayRandom_;
    }
    else
    {
        Log("OpenDelay reset");
        tOpenDelay_->fixedPart  = 0;
        tOpenDelay_->randomPart = 0;
        tOpenDelay_->reset();
    }

    if ( ! isOpening_)
    {
        Log("OpenMarket_ set");
        ctOpenMarket_.set();
    }
    Log("CloseMarket_ reset");
    ctCloseMarket_.reset();

    return true;
}

void GameThread::closeMarket(bool delayed)
{
    if (delayed)
    {
        Log("CloseDelay set");
        tCloseDelay_->fixedPart  = closeDelayFixed_;
        tCloseDelay_->randomPart = closeDelayRandom_;
    }
    else
    {
        Log("CloseDelay reset");
        tCloseDelay_->fixedPart  = 0;
        tCloseDelay_->randomPart = 0;
        tCloseDelay_->reset();
    }

    if ( ! isClosing_)
    {
        Log("CloseMarket_ set");
        ctCloseMarket_.set();
    }
    Log("OpenMarket_ reset");
    ctOpenMarket_.reset();
}

void GameThread::reopenMarket()
{
    Log("Reopen delays set");
    tOpenDelay_->fixedPart   = openDelayFixed_;
    tOpenDelay_->randomPart  = openDelayRandom_;
    tCloseDelay_->fixedPart  = closeDelayFixed_;
    tCloseDelay_->randomPart = closeDelayRandom_;

    if ( ! isOpening_)
    {
        Log("OpenMarket_ set");
        ctOpenMarket_.set();
    }
    if (marketOpened_ && ! isClosing_)
    {
        Log("CloseMarket_ set");
        ctCloseMarket_.set();
    }
}

void GameThread::getIpInfo(DWORD & lastLoginTs, DWORD & lastIp, DWORD & currentIp)
{
    QMutexLocker lock(&gameLock_);
    game_->getAccount().getIpInfo(lastLoginTs, lastIp, currentIp);
}

int GameThread::myLevel()
{
    QMutexLocker lock(&gameLock_);
    const Player::PlayerInfo & info = game_->getPlayer().getBasicInfo();
    return info.basic.lvl;
}

Account::CharList GameThread::getCharList()
{
    QMutexLocker lock(&gameLock_);
    return game_->getAccount().getCharList();
}

bool GameThread::getCurrentChar(::Account::CharInfo & info)
{
    QMutexLocker lock(&gameLock_);
    return game_->getAccount().getCurrentChar(info);
}

unsigned GameThread::getMoney(unsigned *maxMoney /*= 0*/)
{
    QMutexLocker lock(&gameLock_);
    return game_->getInventory().getMoney(maxMoney);
}

unsigned GameThread::getBanknotes()
{
    QMutexLocker lock(&gameLock_);
    return game_->getInventory().getBanknotes();
}

unsigned GameThread::getGold()
{
    QMutexLocker lock(&gameLock_);
    return game_->getInventory().getSilver();
}

std::vector<InventoryItem> GameThread::getBag()
{
    QMutexLocker lock(&gameLock_);

    const Inventory & inv = game_->getInventory();
    return inv.getBag();;
}

InventoryItem GameThread::getCatshop()
{
    QMutexLocker lock(&gameLock_);
    const std::vector<InventoryItem> & equip = game_->getInventory().getEquip();
    if (equip.size() > CatShop)
    {
        return equip[CatShop];
    }
    return InventoryItem();
}

void GameThread::getCatshopLimits(const InventoryItem & catshop, unsigned & maxSlots, unsigned & maxChars)
{
    maxSlots = 0;
    maxChars = 0;

    if (catshop.isEmpty())
    {
        return;
    }

    elements::Item item;
    if (elements_.getItem(catshop.id, item))
    {
        maxChars = item["NameLength"];

        int sellSlots = item["SellSlots"];
        int buySlots = item["BuySlots"];
        if (sellSlots > 0 && buySlots > 0)
        {
            maxSlots = std::min(sellSlots, buySlots);
        }
    }
}

std::vector<MarketItem> GameThread::getMarket()
{
    QMutexLocker lock(&gameLock_);

    const Inventory & inv = game_->getInventory();
    std::vector<MarketItem> items = inv.getMarket();

    // removing empty slots to prevent reopening market if all required items are bought
    for (std::vector<MarketItem>::iterator it = items.begin(); it != items.end();)
    {
        if (it->count == 0/* && it->slot == -1*/)
        {
            it = items.erase(it);
        }
        else
        {
            ++it;
        }
    }

    return items;
}

CoordEx GameThread::getPlayerPos()
{
    QMutexLocker lock(&gameLock_);
    return game_->getPlayer().getPosition();
}

void GameThread::setMarketSetup(const std::vector<MarketItemSetup> & setup)
{
    marketSetup_ = setup;
}

void GameThread::setMarketTitle(const std::wstring & shopTitle)
{
    tOpenMarket_->name = shopTitle;
}

QString GameThread::resolvePlayerName(DWORD playerId)
{
    QMutexLocker lock(&gameLock_);

    std::wstring name;
    bool success, isDelayed;
    std::tie(success, isDelayed, name) = game_->getWorld().resolvePlayer(playerId);
    if (success && !isDelayed)
    {
        return QString::fromStdWString(name);
    }
    return QString();
}

DWORD GameThread::resolvePlayerId(const QString & playerName)
{
    QMutexLocker lock(&gameLock_);

    bool success;
    DWORD id;
    std::tie(success, id) = game_->getWorld().resolvePlayer(playerName.toStdWString());
    return success ? id : 0;
}

unsigned GameThread::lockTime()
{
    QMutexLocker lock(&gameLock_);
    return game_->getPlayer().lockTime();
}

std::vector<CommiShop> GameThread::getCommi()
{
    QMutexLocker lock(&gameLock_);
    return game_->getCommi().shopList();
}

time_t GameThread::commiLastUpdated()
{
    QMutexLocker lock(&gameLock_);
    return game_->getCommi().lastUpdated();
}

//

bool GameThread::selectChar(const QString & charName)
{
    // тут мы вступаем в зону боли
    if (invokeInThread(__func__, charName)) return true;

    return GameBase::selectChar(charName);
}

void GameThread::revive()
{
    if (invokeInThread(__func__)) return;

    QMutexLocker lock(&gameLock_);

    clientdata::FragmentGiPlayerRevive *f = new clientdata::FragmentGiPlayerRevive();
#if PW_SERVER_VERSION >= 1500
    f->unk = 0;
#endif
    clientdata::FragmentGameinfoSet fgi(f);

    std::shared_ptr<Connection> conn = game_->getConnection();
    if (conn)
    {
        conn->send(&fgi);
    }
}

void GameThread::acceptTrade(unsigned tradeId)
{
    if (invokeInThread(__func__, tradeId)) return;

    QMutexLocker lock(&gameLock_);
    game_->getTrade().accept(tradeId);
}

void GameThread::rejectTrade(unsigned tradeId)
{
    if (invokeInThread(__func__, tradeId)) return;

    QMutexLocker lock(&gameLock_);
    game_->getTrade().reject(tradeId);
}

void GameThread::discardTrade()
{
    if (invokeInThread(__func__)) return;

    QMutexLocker lock(&gameLock_);
    game_->getTrade().discard();
}

void GameThread::addTradeItem(unsigned inventorySlot, unsigned count)
{
    if (invokeInThread(__func__, inventorySlot, count)) return;

    QMutexLocker lock(&gameLock_);

    const std::vector<InventoryItem> & bag = game_->getInventory().getBag();
    if (inventorySlot >= bag.size()
        || bag.at(inventorySlot).id == Inventory::INVALID_ID)
    {
        Log("Failed to add trade item: inventory mismatch");
        return;
    }

    int itemCount = std::min(bag.at(inventorySlot).count, count);
    game_->getTrade().addItem(bag.at(inventorySlot).id, inventorySlot, itemCount, 0);
}

void GameThread::addTradeMoney(unsigned money)
{
    if (invokeInThread(__func__, money)) return;

    QMutexLocker lock(&gameLock_);
    game_->getTrade().addItem(0, 0, 0, money);
}

void GameThread::removeTradeItem(unsigned inventorySlot, unsigned count)
{
    if (invokeInThread(__func__, inventorySlot, count)) return;

    const std::vector<InventoryItem> & bag = game_->getInventory().getBag();
    if (inventorySlot >= bag.size()
        || bag.at(inventorySlot).id == Inventory::INVALID_ID)
    {
        Log("Failed to remove trade item: inventory mismatch");
        return;
    }

    int itemCount = std::min(bag.at(inventorySlot).count, count);
    game_->getTrade().removeItem(bag.at(inventorySlot).id, inventorySlot, itemCount, 0);
}

void GameThread::removeTradeMoney(unsigned money)
{
    if (invokeInThread(__func__, money)) return;

    QMutexLocker lock(&gameLock_);
    game_->getTrade().removeItem(0, 0, 0, money);
}

void GameThread::confirmTrade()
{
    if (invokeInThread(__func__)) return;

    QMutexLocker lock(&gameLock_);
    game_->getTrade().confirm();
}

void GameThread::submitTrade()
{
    if (invokeInThread(__func__)) return;

    QMutexLocker lock(&gameLock_);
    game_->getTrade().submit();
}

void GameThread::setLockTime(unsigned seconds)
{
    if (invokeInThread(__func__, seconds)) return;

    QMutexLocker lock(&gameLock_);
    return game_->getPlayer().setLockTime(seconds);
}

void GameThread::buyCatshop(unsigned itemId, unsigned count, unsigned shopIndex)
{
    if (invokeInThread(__func__, itemId, count, shopIndex)) return;

    QMutexLocker lock(&gameLock_);

    clientdata::FragmentGiPurchaseShopItem *f = new clientdata::FragmentGiPurchaseShopItem();
    clientdata::FragmentGameinfoSet fgi(f);

    f->count     = count;
    f->shopIndex = shopIndex;
    f->itemId    = itemId;
    f->u1        = 0;

    std::shared_ptr<Connection> conn = game_->getConnection();
    if (conn)
    {
        conn->send(&fgi);
    }
}

void GameThread::equipCatShop(unsigned inventorySlot)
{
    if (invokeInThread(__func__, inventorySlot)) return;

    QMutexLocker lock(&gameLock_);
    game_->getInventory().equipItem(inventorySlot, CatShop);
}

void GameThread::sendPrivateMessage(const QString & playerName, const QString & text)
{
    if (invokeInThread(__func__, playerName, text)) return;

    DWORD id = resolvePlayerId(playerName);

    QMutexLocker lock(&gameLock_);
    game_->getChat().sendPrivateMessage(playerName.toStdWString(), id, text.toStdWString());
}

void GameThread::sendPublicMessage(const QString & text)
{
    if (invokeInThread(__func__, text)) return;

    QMutexLocker lock(&gameLock_);
    game_->getChat().sendChatMessage(Chat::ChatTypePublic, text.toStdWString());
}

void GameThread::sendGroupMessage(const QString & text)
{
    if (invokeInThread(__func__, text)) return;

    QMutexLocker lock(&gameLock_);
    game_->getChat().sendChatMessage(Chat::ChatTypeGroup, text.toStdWString());
}

void GameThread::sendGuildMessage(const QString & text)
{
    if (invokeInThread(__func__, text)) return;

    QMutexLocker lock(&gameLock_);
    game_->getChat().sendGuildMessage(text.toStdWString());
}

void GameThread::updateCommi(bool trackProgress)
{
    if (invokeInThread(__func__, trackProgress)) return;

    QMutexLocker lock(&gameLock_);
    game_->getCommi().update(trackProgress);
}

void GameThread::swapItems(unsigned slot1, unsigned slot2)
{
    if (invokeInThread(__func__, slot1, slot2)) return;

    QMutexLocker lock(&gameLock_);
    game_->getInventory().swapItems(slot1, slot2);
}

void GameThread::moveItems(unsigned srcSlot, unsigned dstSlot, unsigned count)
{
    if (invokeInThread(__func__, srcSlot, dstSlot, count)) return;

    QMutexLocker lock(&gameLock_);
    game_->getInventory().moveItems(srcSlot, dstSlot, count);
}

//

bool GameThread::beforeConnection()
{
    if (VMProtectGetSerialNumberState() != 0)
    {
        return false;
    }

    marketOpened_ = false;
    isOpening_ = false;
    isClosing_ = false;
    isWaitingForConversion_ = false;

    return true;
}

std::shared_ptr<ITask> GameThread::constructTask()
{
    return
#if defined(GAME_USING_CLIENT)
    task<TPerformIngameLogin>(game_)
#else
    task<TPerformLogin>(game_)
    & task<TGetFriendlist>(game_)
    & task<TGetInventory>(game_)
#endif
    &
    (
        (
            task<TDelay>(5000)
            & task<TEvent>(boost::bind(&GameThread::onLoggedIn, this))
        #if PW_SERVER_VERSION < 1760
            // is registration has gone or has been changed?
            & task<TEvent>(boost::bind(&GameThread::onlineRegistration, this))
                .startIf(FCondition(boost::bind(&GameThread::isOnlineRegAvailable, this)))
        #endif
        )
        &&
        (
            task<TTimer>(game_, 1000, boost::bind(&GameThread::onLockTimer, this))
            & task<TDelay>(1000)     // time hacks
            & task<TOnce>()
                .failWhen(FCondition(boost::bind(&GameThread::chkTimeCondition, this)))
        )
    )
    & task<TLoop>(
        task<TSucceed>(
            task<TEvent>(boost::bind(&GameThread::setNoState, this))
            & 
            (
                (   // open market
                    task<TEvent>(boost::bind(&GameThread::setOpeinigState, this))
                    & task<TEvent>(boost::bind(&GameThread::rebuildMarketList, this))
                    & TaskPtrProxy(tOpenDelay_)
                    & task<TTimeout> (
                        task<TMarketSkill>(game_)
                        , 15000
                    )
                    & task<TTimeout> (
                        TaskPtrProxy(tOpenMarket_)
                        , 15000
                    )
                ).startWhen(ctOpenMarket_)
                    .failWhen(GCondition<GotGameErrCond>(game_))
                | task<TFail>(task<TEvent>(boost::bind(&GameThread::marketError, this)))    // transparent fail
            )
            & task<TEvent>(boost::bind(&GameThread::setNoState, this))
            &
            ( // close market
                // check if needs to be closed to registration
                task<TLoop>(
                    task<TDelay>(60 * 60 * 1000)
                    &
                    (
                        task<TTimeout>(
                            task<TCloseMarket>(game_)
                            , 15000)
                        & task<TEvent>(boost::bind(&GameThread::setReopenMarket, this))   // reopen after closing
                    ).startIf(FCondition(boost::bind(&GameThread::isOnlineRegAvailable, this)))
                )
                ^
                (   // or close by trigger
                    task<TEvent>(boost::bind(&GameThread::setClosingState, this))
                    & TaskPtrProxy(tCloseDelay_)
                    & task<TTimeout>(
                        task<TCloseMarket>(game_)
                        , 15000
                    )
                ).startWhen(ctCloseMarket_)
                // wait if closed externally
                ^ task<TWait>()       
                    .stopWhen(ctMarketClosedExtern_)
            )
            & task<TEvent>(boost::bind(&GameThread::setNoState, this))
            &
            ( // after-close actions
                (
                    task<TDelay>(1000)
                    & task<TEvent>(boost::bind(&GameThread::onlineRegistration, this))
                    & task<TDelay>(2000)
                ).startIf(FCondition(boost::bind(&GameThread::isOnlineRegAvailable, this)))
            )
//                 ^ (
//                     task<TDelay>(60 * 10 * 1000)
//                     & task<TEvent>(boost::bind(&GameThread::setMoveMarket, this))
//                 ).startWhen(BCondition(GameThread::doWandering_))
//                 (   // when moveMarket timer fires, open-close loop goes to initial point (before opening trigger)
//                     // and we need to actually close market here but only call to set opening
//                     task<TTimeout>(
//                         task<TCloseMarket>(game_)
//                         , 15000
//                     )
//                     & task<TDelay>(1000)
//                     & TaskPtrProxy(tMoveMarket_)
//                     & task<TDelay>(1000)
//                     & task<TEvent>(boost::bind(&GameThread::setOpenMarket, this))
//                 ).startWhen(ctMoveMarket_)
//             )
        )
    );
}

void GameThread::setNoState()
{
    Log("No state");
    isOpening_ = false;
    isClosing_ = false;
    game_->clearLastGameError();    // clear possible convertation error
}

void GameThread::setOpeinigState()
{
    Log("Opening state");
    isOpening_ = true;
    isClosing_ = false;
}

void GameThread::setClosingState()
{
    Log("Closing state");
    isOpening_ = false;
    isClosing_ = true;
}

void GameThread::setMoveMarket()
{
//     Log("moving");
// 
//     if (marketPos_.isNull())
//     {
//         Log("Cannot move - no initial point");
//         return;
//     }
// 
//     tMoveMarket_->destination = marketPos_.moved( 2.f * rand() / RAND_MAX - 1.0f
//                                                 , 2.f * rand() / RAND_MAX - 1.0f
//                                                 , 0);
//     Log("Moving to [%.2f %.2f %.2f]"
//         , tMoveMarket_->destination.x()
//         , tMoveMarket_->destination.y()
//         , tMoveMarket_->destination.z());
// 
//     ctDelayedMarket_.reset();
//     ctDelayedMarketCancel_.set();   // canceling delayed as we're reopening by move
//     ctMoveMarket_.set();
// 
//     marketMove();
}

void GameThread::setReopenMarket()
{
    Log("OpenMarket set");
    tOpenDelay_->fixedPart  = 0;
    tOpenDelay_->randomPart = 0;
    tOpenDelay_->reset();

    if ( ! isOpening_)
    {
        Log("OpenMarket_ set");
        ctOpenMarket_.set();
    }
}

bool GameThread::chkTimeCondition()
{
    Log("chkTimeCondition");

    VMProtectSerialNumberData data;

    bool result = true;

    if (VMProtectGetSerialNumberData(&data, sizeof(data))
        && data.nState == 0)
    {
        if (data.dtExpire.wYear != 0)
        {
            FILETIME currentSt;
            GetSystemTimeAsFileTime(&currentSt);
            _LARGE_INTEGER systemT;
            systemT.HighPart = currentSt.dwHighDateTime;
            systemT.LowPart = currentSt.dwLowDateTime;

            Timestamp g = game_->timer()->serverTime();
            ULARGE_INTEGER gameT;
            gameT.QuadPart = g * 10000;     // to 10ns

            long long diff = std::llabs(systemT.QuadPart - gameT.QuadPart);
            Log("GT: %llu  LT: %llu  D: %llu", gameT.QuadPart / 10000, systemT.QuadPart / 10000, diff / 10000);

            if (diff > (6LL/*h*/ * 60/*m*/ * 60/*s*/ * 1000 * 10000))
            {
                Log("Err time screwed up");
                // true
            }
            else
            {
                FILETIME gt;
                gt.dwLowDateTime = gameT.LowPart;
                gt.dwHighDateTime = gameT.HighPart;

                SYSTEMTIME gameSt;
                if (FileTimeToSystemTime(&gt, &gameSt))
                {
                    if (gameSt.wYear > data.dtExpire.wYear
                        || (gameSt.wYear == data.dtExpire.wYear && gameSt.wMonth > data.dtExpire.bMonth)
                        || (gameSt.wYear == data.dtExpire.wYear && gameSt.wMonth == data.dtExpire.bMonth && gameSt.wDay > data.dtExpire.bDay))
                    {
                        // expired
                    }
                    else
                    {
                        result = false;
                    }
                }
            }
        }
        else
        {
            result = false;
        }
    }

    return result;
}

void GameThread::onlineRegistration()
{
    Log("Performing online registration");
    game_->getPlayer().onlineRegistration();
}

bool GameThread::isOnlineRegAvailable()
{
    Log("OnlineCheck");
    if ( ! doOnlineRegistration_ )
    {
        return false;
    }

    Timestamp g = game_->timer()->serverLocalTime();
    SYSTEMTIME gameSt;
    if (GTime::tsToSystem(g, &gameSt))
    {
        std::vector<bool> days = game_->getPlayer().onlineRegistrationDays();
        if (gameSt.wDay < days.size()
            && ! days[gameSt.wDay - 1])
        {
            Log("Online registration available");
            return true;
        }
        else
        {
            Log("Online already registered");
        }
    }
    return false;
}

//

bool GameThread::onLockTimer()
{
    Player & p = game_->getPlayer();
    Timestamp remaining;
    Player::LockState lockState = p.lockState(&remaining);
    if (lockState == Player::LockStateUnknown)
    {
        return true;    // continue timer, no event
    }

    lockInfo(lockState != Player::LockStateOff, remaining);
    return lockState != Player::LockStateOff;
}

void GameThread::onBaseEvent(BaseEvent *e)
{
    // синхронный слот !

    if (e == 0) return;

    switch (e->type())
    {
        case BaseEvent::MarketItemSold:
        case BaseEvent::MarketItemPurchased:
        {
            if (!marketOpened_)
            {
                // skip trade operations when not in market
                break;
            }

            // close market immediately if there's no items left in market (and reschedule if possible)
            std::vector<MarketItem> currentItems = getMarket();
            if (currentItems.size() == 0)
            {
                Log("Closing market: no more items in current market");
                tCloseDelay_->fixedPart  = 0;
                tCloseDelay_->randomPart = 0;
                tCloseDelay_->reset();
                if ( ! isClosing_)
                {
                    Log("CloseMarket_ set");
                    ctCloseMarket_.set();
                }
            }
            else
            {
                // do small delay to get updated money / notes
                // this also ensures operation executes in this thread
                QTimer::singleShot(1000, this, SLOT(converNotes()));
            }
            break;
        }

        case BaseEvent::InventoryUpdate:
        {
            InventoryUpdateEvent *ev = static_cast<InventoryUpdateEvent*>(e);
            if ((ev->storageType() & MONEY_INFO) != 0)
            {
                Log("Not waiting for notes conversation");
                isWaitingForConversion_ = false;
            }
            break;
        }

        case BaseEvent::PlayerMarketState:
        {
            PlayerMarketStateEvent *ev = static_cast<PlayerMarketStateEvent*>(e);
            marketOpened_ = ev->isOpened();
            Log("PlayerMarketState: %i", marketOpened_);

            isWaitingForConversion_ = false;    // when just opened - we reset it, when just close - also no conversion would be possible

            if (marketOpened_)
            {
                converNotes();  // convert when just opened
                ctMarketClosedExtern_.reset();
            }
            else
            {
                ctMarketClosedExtern_.set();
            }
            
            break;
        }

        case BaseEvent::PlayerKilled:
        {
            Log("OpenMarket reset");
            ctOpenMarket_.reset();      // in case of delayed is set
            break;
        }
    }
}

void GameThread::onLoggedIn()
{
    Log("onLoggedIn");
    loggedIn();
}

//

void GameThread::rebuildMarketList()
{
    tOpenMarket_->items.clear();

    unsigned maxSlots, unused;
    getCatshopLimits(getCatshop(), maxSlots, unused);
    bool hasCatshop = maxSlots != 0;

    unsigned money = getMoney();
    unsigned buyLimit = money;
    if (hasCatshop
        && highConverterValue_ > 1)
    {
        // лимит покупки ограничен сверху конвертером монеток в банкноты, при этом если установлено на 100, то лимит ~90 т.к. на 101 меняется бумажка и остается 91
        buyLimit = std::min(money, (highConverterValue_ - 1) * 10000000);
    }
    std::vector<ShortMarketItem> items = buildMarketList(getBag()
                                                         , marketSetup_
                                                         , (hasCatshop ? InventoryMoneyLimitCatshop : InventoryMoneyLimit) - money
                                                         , buyLimit
                                                         , std::max<unsigned>(CatshopMaxSlotsDefault, maxSlots));
    for (size_t i = 0; i < items.size(); i++)
    {
        const ShortMarketItem & mi = items.at(i);

        TOpenMarket::MarketItem ti;
        ti.id    = mi.itemId;
        ti.slot  = (WORD)mi.slot;   // fixme
        ti.count = mi.count;
        ti.price = mi.price;
        ti.unk   = 0;

        tOpenMarket_->items.push_back(ti);
    }
}

void GameThread::converNotes()
{
    if (getCatshop().isEmpty() || !marketOpened_)
    {
        Log("No money conversion: catshop missing / market closed");
        return;
    }

    if (isWaitingForConversion_)
    {
        Log("waiting for conversion to finish");
        return;
    }

    unsigned moneyInNotes = getMoney() / 10000000;
    unsigned notes = getBanknotes();

    if (lowConverterValue_ > 0 
        && moneyInNotes < lowConverterValue_
        && notes > 0)
    {
        // convert to money
        int toConvert = std::min(lowConverterValue_ - moneyInNotes, notes);
        Log("Converting %i to money", toConvert);
        game_->getPlayer().convertNotes(true, toConvert);
        isWaitingForConversion_ = true;
    }
    else if (highConverterValue_ > 0
        && moneyInNotes >= highConverterValue_)
    {
        // convert to notes
        int toConvert = 1 + moneyInNotes - highConverterValue_;
        Log("Converting %i to notes", toConvert);
        game_->getPlayer().convertNotes(false, toConvert);
        isWaitingForConversion_ = true;
    }
    else
    {
        Log("Converting: nothing to convert");
    }
}
