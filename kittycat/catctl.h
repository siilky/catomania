#ifndef catctl_h
#define catctl_h

#pragma warning(disable : 4503)     // 'boost::detail::variant::visitation_impl' : decorated name length exceeded, name was truncated

// moc guard
#pragma warning(push, 1)
#pragma warning(disable : 4251)     // class 'QSharedDataPointer<T>' needs to have dll-interface to be used by clients of class 'QNetworkProxyQuery'
#pragma warning(disable : 6011)     // Dereferencing null pointer
#pragma warning(disable : 6385)     // Invalid data
#pragma warning(disable : 6386)     // Buffer overrun

#include <QtCore>
#include <QtNetwork>

#pragma warning(pop)

#include "MarketItemSetup.h"
#include "options.h"
#include "serverlist.h"
#include "game/data/items.h"
#include "qlib/game/gamebase.h"
#include "PortManager.h"
#include "historydb.h"

namespace PwMailRu
{
    class MailruAuth;
}
namespace ARC
{
    class ArcAuth;
}

class GameThread;
class ProcessCtl;
class AutoPriceMonitor;


class CatCtl : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(CatCtl)
public:
    enum Event
    {
        CharselectRequest,          // game is requesting to select character
        CharselectAuto,             // going to select character automatically
        CharselectFailed,           // failed to select character
        CharselectNoChars,          // No characters on the account

        MarketReopening,
        MarketMoving,

        ImmobilizerWarned,

        Unknown
    };

    // TODO два класса для списка аккаунтов и текущего выделеня это слишком сложно. Нужно упростить.

    class Account
    {
    public:
        Account();
        Account(const JsonValue & config);
        operator JsonValue() const;

        QString     name;
        QStringList characters;
    };

    class AccountSelection
    {
    public:
        AccountSelection()
            : accountIndex(-1), characterIndex(-1)
        {}
        AccountSelection(int acc, int ch)
            : accountIndex(acc), characterIndex(ch)
        {}

        bool operator==(const AccountSelection & r) const
        {
            return accountIndex == r.accountIndex
                && characterIndex == r.characterIndex;
        }

        int accountIndex;
        int characterIndex;
    };


    CatCtl(const JsonValue & config, QObject *parent = NULL);
    ~CatCtl();

#if defined GAME_USING_CLIENT
    void setProcessSafeMode(bool isOn);
#endif

    JsonValue & config()
    {
        return config_;
    }

    QNetworkProxy   getProxy() const;

    QStringList     getServers(int & current) const;
    int             getCurrentServerAsPwcatsIndex() const;
    void            setCurrentServer(int currentServer);

    void            getAuth(QString & email, QString & password) const;
    void            setAuth(const QString & email, const QString & password);
    QList<Account>  getAccounts(AccountSelection & current) const;
    void            setAccounts(const QList<Account> & accounts);

    bool connect(int accountIndex, int charIndex);
    bool selectChar(int accountIndex, int charIndex);
    void disconnect(bool wait = true);
    // arc
    void setPin(QString pin);

    void showClientWindow();
    void hideClientWindow();

    void getIpInfo(DWORD & lastLoginTs, DWORD & lastIp, DWORD & currentIp) const;

    int myLevel();
    DWORD getMyId();
    QString getMyName();

    QString                 getMarketName() const;
    bool                    setMarketName(const QString & title);

    QList<MarketItemSetup>  getShopItems();
    QList<MarketItemSetup>  getCurrentShopItems();
    bool                    setShopItems(const QList<MarketItemSetup> & items);

    InventoryItem           getCatshop();
    void                    getCatshopLimits(const InventoryItem & catshop, unsigned & maxSlots, unsigned & maxChars);     // returns 0 if no catshop
    QList<InventoryItem>    getInventoryItems();

    unsigned                getMoney();
    unsigned                getBanknotes();
    unsigned                getGold();
    qint64                  getProfit();

    QString     getItemName(unsigned itemId);
    std::string getItemIcon(unsigned itemId);
    QString     resolvePlayerName(DWORD playerId);

    const Options & getOptions() const;
    void            setOptions(const Options & opts);

    bool    openMarket();
    void    closeMarket();
    bool    isMarketOpened() const;

    const HistoryDb * tradeHistory() const;
    void        resetProfit();
    unsigned    getStockBuyPrice(unsigned itemId) const;

    // actions
    void        revive();

    // lock
    unsigned    lockTime() const;

    // 
    void sendPrivateMessage(const QString & name, const QString & text);
    void sendPublicMessage(const QString & text);
    void sendGroupMessage(const QString & text);
    void sendGuildMessage(const QString & text);

    void updateCommi(bool trackProgress);
    time_t commiLastUpdated() const;
    std::vector<CommiShop> getCommi();

    QString getPcName() const;

public slots:
    // trade
    void acceptTrade(DWORD tradeId);
    void rejectTrade(DWORD tradeId);
    void discardTrade();
    void addTradeItem(unsigned inventorySlot, unsigned count);
    void addTradeMoney(int money);
    void removeTradeItem(unsigned inventorySlot, unsigned count);
    void removeTradeMoney(int money);
    void confirmTrade();
    void submitTrade();

    void setLockTime(unsigned seconds);

    void buyCatshop(unsigned itemShopId);
    void equipCatShop(unsigned inventorySlot);

    void swapItems(unsigned slot1, unsigned slot2);
    void moveItems(unsigned srcSlot, unsigned dstSlot, unsigned count);

signals:
    void authProgress(int value, int maxValue);
    void authGetPin();
    void connected();
    void loggedIn();
    void disconnected();
    void completed();

    void message(const QString & msg);
    void error(const QString & message);

    void event(CatCtl::Event gameEvent);
    void gameEvent(const qlib::GameEvent & event);  // GameBase mirror

    void reconnectEvent(int remainingSeconds);      // 0 - reconnect required, <0 - reconnect canceled
    void lockInfo(bool status, unsigned long long remainingSeconds);

    void clientStarted();
    void clientStopped();
    void clientWindowClosed();

    void marketSetupUpdated();
    void profitUpdated();

private slots:
    void onDisconnected();
    void onReconnectTimer();

#if defined(MAILRU_TOKEN_AUTH) || defined(ARC_TOKEN_AUTH)
    void onAuthError(const QString & str);
    void onAuthFinished(bool success);
#endif

    void onClientStopped();

    void onLoggedIn();
    void onMarketMove();
    void onMarketEmpty();
    void onMarketError();

    void onEvent(const qlib::GameEvent & event);

    void checkMarketToReopen();
    void loadExternalConfig();

private:
    void setupReconnect();
    void requestCharSelect();
    void updateProxy();
    void saveMarketSetup();

    void mergeMarketSetup(const QList<MarketItemSetup> & items, std::vector<MarketItemSetup> & oldSetup);

    JsonValue   config_;
    Options     options_;

    std::vector<Account>    accounts_;
    int                     accountIndex_, charIndex_;    // selected indexes
    std::vector<Server>     servers_;
    int                     currentServer_;

    bool        marketOpened_;
    bool        marketShouldBeOpened_;
    bool        marketReopenPending_;
    Coord3D     lastSavedPos_;
    bool        tropophobiaWarned_;

    std::vector<MarketItemSetup>    marketSetup_;
    std::wstring                    marketTitle_;

#if defined MAILRU_TOKEN_AUTH
    QPointer<PwMailRu::MailruAuth>  mailruAuth_;
    QString                         lastAuthError_;
#elif defined ARC_TOKEN_AUTH
    QPointer<ARC::ArcAuth>          arcAuth_;
    QString                         lastAuthError_;
#endif

    QNetworkProxy                   proxy_;
    QScopedPointer<GameThread>      gameThread_;

    QTimer  reconnectTimer_;
    int     remainingToReconnect_;
    bool    suppressReconnect_;

    QTimer  loadExternalConfigTimer_;

    HistoryDb   *tradeHistory_;
    AutoPriceMonitor *autoPriceMonitor_;

#if GAME_OWNER_TYPE==2
    void loadPwcatsIndex();
    QMap<QString, unsigned>     pwcatsIndex_;
#endif

#if defined GAME_USING_CLIENT
    QSharedPointer<ProcessCtl>  processCtl_;
    UniquePort                  localPort_;
#endif
};

#endif