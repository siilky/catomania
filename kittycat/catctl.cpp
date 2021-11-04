
#pragma warning(disable : 4512)     // assignment operator could not be generated

#include "boost/signal.hpp"

#include "config.h"
#include "persistence2.h"
#include "game/game.h"              // uses boost signal, should go before QT includes

#include "stdafx.h"                 // but we dont use PCH

#include "main.h"
#include "catctl.h"
#include "event.h"
#include "gamethread.h"
#include "serverlist.h"
#include "marketbuilder.h"
#include "processctl.h"
#include "util.h"

#include "wtypes.h"
#include "VMProtectSDK.h"
#include "game/data/gshop.h"
#include "mailru/mailruauth.h"
#include "auth_errors.h"
#include "AutoPriceMonitor.h"

#include "qlib/netio/connectionproxy.h"
#include "qlib/vmp.h"
#include "arc/arcauth.h"

extern elements::ItemListCollection  g_elements_;

//

class ConnectionGeneratorProxy : public qlib::ConnectionGenerator
{
public:
    ConnectionGeneratorProxy( QSharedPointer<ProcessCtl> processCtl
                            , const QString         &executable
                            , quint16               localPort
                            , const QString         &login
                            , const QString         &password
                            , const std::wstring    &server
                            , const QNetworkProxy   &proxy = QNetworkProxy()
                            , bool                  forced = false
                            , bool                  safeMode = false)
        : executable_(executable)
        , processCtl_(processCtl)
        , login_(login)
        , password_(password)
        , server_(server)
        , proxy_(proxy)
        , forced_(forced)
        , safeMode_(safeMode)
        , localPort_(localPort)
        , connection_(0)
    {
    }

    virtual ~ConnectionGeneratorProxy()
    {
        close();
        processCtl_->stop();
    }

    virtual bool isValid() const
    {
        return processCtl_ != 0 && !executable_.isEmpty()
            && !login_.isEmpty() && !password_.isEmpty() && !server_.empty();
    }

    virtual std::shared_ptr<Connection> get() override
    {
        // we should create connection in thread calling "get"
        if (!conn_)
        {
            connection_ = new qlib::ConnectionProxy(safeMode_);
            conn_ = std::shared_ptr<Connection>(connection_);
        }
        return conn_;
    }

    virtual bool open()
    {
        uint id = qHash(QCoreApplication::applicationPid());
        id = (id & 0x00FFFFFF) ^ (id & 0xFF000000 >> 8);
        QHostAddress localIp(0x7F000000 | id);

        if (connection_->open(login_.toStdString(), password_.toStdString(), forced_, server_, proxy_, localPort_, localIp))
        {
            processCtl_->setServerSubstitution(localIp.toString(), localPort_);
            if (processCtl_->start(executable_, login_, password_.fill('0')))
            {
                return true;
            }
            connection_->close();
        }
        return false;
    }

    virtual void close()
    {
        conn_.reset();
        processCtl_->stop();
    }

private:
    QString                     executable_;
    QSharedPointer<ProcessCtl>  processCtl_;

    QString         login_;
    QString         password_;
    std::wstring    server_;
    QNetworkProxy   proxy_;
    bool            forced_;
    bool            safeMode_;

    quint16                 localPort_;
    qlib::ConnectionProxy   *connection_;
    std::shared_ptr < Connection >    conn_;
};

//

CatCtl::Account::Account(const JsonValue & config)
{
    std::wstring                n;
    std::vector<std::wstring>   cs;

    config.get(L"Name", n);
    config.get(L"Characters", cs, json::NoConversion<std::wstring>());

    name = QString::fromStdWString(n);
    for(unsigned i = 0; i < cs.size(); i++)
    {
        characters << QString::fromStdWString(cs[i]);
    }
}

CatCtl::Account::Account()
{
}

CatCtl::Account::operator JsonValue() const
{
    std::vector<std::wstring>   cs;
    for(int i = 0; i < characters.size(); i++)
    {
        cs.push_back(characters[i].toStdWString());
    }

    JsonValue config;

    config.set(L"Name", name.toStdWString());
    config.set(L"Characters", cs, json::NoConversion<std::wstring>());

    return config;
}

//

CatCtl::CatCtl(const JsonValue & config, QObject *parent /*= NULL*/ )
    : QObject(parent)
    , config_(config)
    , options_(config)
    , currentServer_(-1)
    , gameThread_(new GameThread())
    , marketOpened_(false)
    , marketShouldBeOpened_(false)
    , marketReopenPending_(false)
    , suppressReconnect_(false)
    , remainingToReconnect_(-1)
#if defined GAME_USING_CLIENT
    , processCtl_(new ProcessCtl())
#endif
{
    reconnectTimer_.setInterval(1000);
    QObject::connect(&reconnectTimer_, &QTimer::timeout, this, &CatCtl::onReconnectTimer);

    config_.get(L"Accounts", accounts_, json::DirectConversion<Account>());
    config_.get(L"ShopItems", marketSetup_, json::DirectConversion<MarketItemSetup>());
    config_.get(L"ShopTitle", marketTitle_);

    config_.get(L"SelectedAccount", accountIndex_, -1);
    config_.get(L"SelectedChar",    charIndex_, -1);
    config_.get(L"CurrentServer",   currentServer_, -1);

    double x, y, z;
    config_.get(L"LastPosX", x, 0.);
    config_.get(L"LastPosY", y, 0.);
    config_.get(L"LastPosZ", z, 0.);
    if (x != 0. && y != 0. && z != 0.)
    {
        lastSavedPos_ = Coord3D(float(x), float(y), float(z));
    }

    loadServers(servers_);
    if (servers_.size() != 0 && currentServer_ < 0)
    {
        currentServer_ = 0;
    }

#if defined(ARC_TOKEN_AUTH)
    std::wstring hwId = options_.arcHwid;
    if (hwId.empty())
    {
        options_.arcHwid = Vmp::getHwId().toStdWString();
        options_.writeTo(config_);
    }
#endif

#if GAME_OWNER_TYPE==2
    loadPwcatsIndex();
#endif

    gameThread_->setOptions( options_.closeDelayFixed, options_.closeDelayRandom
                        , options_.openDelayFixed, options_.openDelayRandom
                        , options_.onlineReg
                        , options_.lowConverter ? options_.lowConverterValue : 0
                        , options_.highConverter ? options_.highConverterValue : 0
                        );

    updateProxy();

    QObject::connect(gameThread_.data(), &GameThread::connected,        this, &CatCtl::connected);
    QObject::connect(gameThread_.data(), &GameThread::disconnected,     this, &CatCtl::disconnected);
    QObject::connect(gameThread_.data(), &GameThread::disconnected,     this, &CatCtl::onDisconnected);
    QObject::connect(gameThread_.data(), &GameThread::completed,        this, &CatCtl::completed);
    QObject::connect(gameThread_.data(), &GameThread::error,            this, &CatCtl::error);
    QObject::connect(gameThread_.data(), &GameThread::marketIsEmpty,    this, &CatCtl::onMarketEmpty);
    QObject::connect(gameThread_.data(), &GameThread::marketMove,       this, &CatCtl::onMarketMove);
    QObject::connect(gameThread_.data(), &GameThread::marketError,      this, &CatCtl::onMarketError);
    QObject::connect(gameThread_.data(), &GameThread::loggedIn,         this, &CatCtl::onLoggedIn);
    QObject::connect(gameThread_.data(), &GameThread::lockInfo,         this, &CatCtl::lockInfo);
    QObject::connect(gameThread_.data(), &GameThread::event,            this, &CatCtl::onEvent);
    QObject::connect(gameThread_.data(), &GameThread::event,            this, &CatCtl::gameEvent);

#if defined GAME_USING_CLIENT
    QObject::connect(processCtl_.data(), &ProcessCtl::error,            this, &CatCtl::error);
    QObject::connect(processCtl_.data(), &ProcessCtl::started,          this, &CatCtl::clientStarted);
    QObject::connect(processCtl_.data(), &ProcessCtl::stopped,          this, &CatCtl::onClientStopped);
    QObject::connect(processCtl_.data(), &ProcessCtl::windowClosed,     this, &CatCtl::clientWindowClosed);
#endif

    QObject::connect(&loadExternalConfigTimer_, &QTimer::timeout,       this, &CatCtl::loadExternalConfig);

    gameThread_->setElements(g_elements_);

    // rebuild props on items

    std::shared_ptr<ItemPropsFactory> pf = gameThread_->getPropFactory();

    for (size_t i = 0; i < marketSetup_.size(); i++)
    {
        if ( ! marketSetup_[i].attributes.empty())
        {
            marketSetup_[i].item.props = std::shared_ptr<ItemProps>(
                pf->create(marketSetup_[i].item.id, marketSetup_[i].attributes));
        }
    }

    // set after props are rebuilt
    gameThread_->setMarketSetup(marketSetup_);
    gameThread_->setMarketTitle(marketTitle_);

    if (options_.autoLogin)
    {
        if (options_.reconnectDelayOnStartup)
        {
            remainingToReconnect_ = options_.reconnectDelayFixed 
                + int(double(qrand()) * options_.reconnectDelayRandom / RAND_MAX);
        }
        else
        {
            remainingToReconnect_ = 1;
        }

        reconnectTimer_.start();
    }

    VMProtectBeginVirtualization(__FUNCTION__);
    if (Vmp::hasOption(Vmp::OptionHistory))
    {
        tradeHistory_ = new HistoryDb();
        // DEBUG
        //assert(tradeHistory_->open("DBx01"));
    }
    else
    {
        tradeHistory_ = 0;
    }
    if (Vmp::hasOption(Vmp::OptionAutoprice))
    {
        autoPriceMonitor_ = AutoPriceMonitor::instance();
    }
    else
    {
        autoPriceMonitor_ = 0;
    }

    VMProtectEnd();
}

CatCtl::~CatCtl()
{
}

#if defined GAME_USING_CLIENT
void CatCtl::setProcessSafeMode(bool isOn)
{
    processCtl_->setSafeMode(isOn);
}
#endif

//

QNetworkProxy CatCtl::getProxy() const
{
    return proxy_;
}

QStringList CatCtl::getServers(int & current) const
{
    QStringList servers;

    size_t size = servers_.size();
    for (size_t i = 0; i < size; i++)
    {
        servers << QString::fromStdWString(servers_[i].name);
    }

    current = currentServer_;

    return servers;
}

int CatCtl::getCurrentServerAsPwcatsIndex() const
{
    if (currentServer_ < 0
        || (size_t)currentServer_ >= servers_.size())
    {
        return -1;
    }

#if GAME_OWNER_TYPE==1
    // link**.pwonline.ru
    std::wstring server = servers_[currentServer_].host;
    if (server.size() < 16)
    {
        return -1;
    }

    wchar_t number[3] = {0};

    if (iswdigit(server[4]))
    {
        number[0] = server[4];
        server[4] = L'*';
    }
    if (iswdigit(server[5]))
    {
        number[1] = server[5];
        server[5] = '*';
    }

    wchar_t *endptr;
    unsigned long value = wcstoul(number, &endptr, 10);

    return endptr == number ? -1 : value;
#elif GAME_OWNER_TYPE==2
    std::wstring server = servers_[currentServer_].name;
    auto it = pwcatsIndex_.find(QString::fromStdWString(server));
    if (it != pwcatsIndex_.end())
    {
        return it.value();
    }
    else
    {
        return -1;
    }
#elif GAME_OWNER_TYPE==3
    return 100;
#elif GAME_OWNER_TYPE==4
    return 300;
#elif GAME_OWNER_TYPE==6
    return 350;
#else
    return -1;
#endif
}

void CatCtl::setCurrentServer(int current)
{
    if (current >= 0 && current < (int)servers_.size())
    {
        currentServer_ = current;
    }
    else
    {
        currentServer_ = -1;
    }

    config_.set(L"CurrentServer", currentServer_);
}

void CatCtl::getAuth(QString & email, QString & password) const
{
    std::wstring e, p;
    config_.get(L"Email", e);
    config_.get(L"Password", p);
    email = QString::fromStdWString(e);
    password = QString::fromStdWString(p);
}

void CatCtl::setAuth(const QString & email, const QString & password)
{
    config_.set(L"Email", email.toStdWString());
    config_.set(L"Password", password.toStdWString());
}

QList<CatCtl::Account> CatCtl::getAccounts(CatCtl::AccountSelection & current) const
{
    current.accountIndex = accountIndex_;
    current.characterIndex = charIndex_;

    return QVector<Account>::fromStdVector(accounts_).toList();
}

void CatCtl::setAccounts(const QList<Account> & accounts)
{
    accounts_ = accounts.toVector().toStdVector();
    config_.set(L"Accounts", accounts_, json::DirectConversion<Account>());
    config_.save();
}

bool CatCtl::connect(int accountIndex, int charIndex)
{
    if (gameThread_->isRunning())
    {
        Log("Already running");
        return false;
    }

    suppressReconnect_ = false;
    marketOpened_ = false;
    remainingToReconnect_ = -1;
    reconnectTimer_.stop();

    if (VMProtectGetSerialNumberState() != 0)
    {
        error(tr("License error. Please check your license is active"));
        return false;
    }

    if ((accountIndex < 0 || size_t(accountIndex) >= accounts_.size())
        || (charIndex >= 0 && charIndex >= accounts_[accountIndex].characters.size()))
    {
        error(tr("Invalid account info or no accounts configured"));
        return false;
    }

    if (servers_.size() == 0 
        || currentServer_ < 0
        || size_t(currentServer_) >= servers_.size())
    {
        error(tr("Invalid server info or no servers are loaded"));
        return false;
    }

    accountIndex_ = accountIndex;
    charIndex_ = charIndex;
    config_.set(L"SelectedAccount", accountIndex_);
    config_.set(L"SelectedChar", charIndex_);

    QString email, password;
    getAuth(email, password);

    if (email.isEmpty() || password.isEmpty())
    {
        error(tr("Email or password is empty. Please set valid email and password in account setup"));
        return false;
    }

#if defined MAILRU_TOKEN_AUTH
    lastAuthError_.clear();

    if (mailruAuth_)
    {
        delete mailruAuth_;
    }

    mailruAuth_ = PwMailRu::getAuth(email, this);
    if (!mailruAuth_)
    {
        error(tr("Invalid email"));
        return false;
    }

    if (options_.useProxy)
    {
        mailruAuth_->setProxy(proxy_);
    }

    QObject::connect(mailruAuth_.data(), &PwMailRu::MailruAuth::progress, this, &CatCtl::authProgress);
    QObject::connect(mailruAuth_.data(), &PwMailRu::MailruAuth::finished, this, &CatCtl::onAuthFinished);
    QObject::connect(mailruAuth_.data(), &PwMailRu::MailruAuth::error,    this, &CatCtl::onAuthError);

    mailruAuth_->start(email, password);
#elif defined ARC_TOKEN_AUTH
    lastAuthError_.clear();

    if (arcAuth_)
    {
        delete arcAuth_;
    }

    std::wstring hwId = options_.arcHwid;
    assert(!hwId.empty());
    arcAuth_ = new ARC::ArcAuth(QString::fromStdWString(hwId), this);

    if (options_.useProxy)
    {
        arcAuth_->setProxy(proxy_);
    }

    QObject::connect(arcAuth_.data(), &ARC::ArcAuth::progress,  this, &CatCtl::authProgress);
    QObject::connect(arcAuth_.data(), &ARC::ArcAuth::finished,  this, &CatCtl::onAuthFinished);
    QObject::connect(arcAuth_.data(), &ARC::ArcAuth::error,     this, &CatCtl::onAuthError);
    QObject::connect(arcAuth_.data(), &ARC::ArcAuth::needPin,   this, &CatCtl::authGetPin);

    arcAuth_->start(email, password);

#else
    const Server & s = servers_.at(currentServer_);
    bool forcedLogin;
    config_.get(L"ForcedLogin", forcedLogin);

    tropophobiaWarned_ = false;

#if defined GAME_USING_CLIENT
    quint16 port = localPort_.value();
    if (port == 0)
    {
        error(tr("No local ports available"));
        return false;
    }

    gameThread_->start(QSharedPointer<qlib::ConnectionGenerator>(
        new ConnectionGeneratorProxy(
            processCtl_,
            g_elementExePath_,
            port,
            email,
            password,
            s.host + L':' + s.port,
            proxy_,
            forcedLogin)
        ));
#else
    gameThread_->start(QSharedPointer<qlib::ConnectionGenerator>(new qlib::ConnectionGeneratorTcp(
                        email.toStdWString()
                        , password.toStdWString()
                        , s.host + L':' + s.port
                        , proxy_
                        , forcedLogin)));
#endif
#endif

    return true;
}

bool CatCtl::selectChar(int accountIndex, int charIndex)
{
    if (VMProtectGetSerialNumberState() != 0)
    {
        error(tr("License error. Please check your license is active"));
        return false;
    }

    if ((accountIndex < 0 || size_t(accountIndex) >= accounts_.size())
        || (charIndex < 0 || charIndex >= accounts_[accountIndex].characters.size()))
    {
    #if defined(GAME_USING_CLIENT)
        // allow user to create character in client
        accountIndex_ = -1;
        charIndex_ = -1;
        return processCtl_->selectChar("");
    #else
        error(tr("Invalid account info or no accounts configured"));
        return false;
    #endif
    }

    accountIndex_ = accountIndex;
    charIndex_ = charIndex;
    config_.set(L"SelectedAccount", accountIndex_);
    config_.set(L"SelectedChar", charIndex_);

    const QString & charName = accounts_[accountIndex_].characters[charIndex_];
#if defined(GAME_USING_CLIENT)
    return processCtl_->selectChar(charName);
#else
    return gameThread_->selectChar(charName);
#endif
}

void CatCtl::disconnect(bool wait)
{
#if defined(MAILRU_TOKEN_AUTH)
    delete mailruAuth_;
#elif defined(ARC_TOKEN_AUTH)
    delete arcAuth_;
#endif

    suppressReconnect_ = true;
    reconnectTimer_.stop();
    gameThread_->stop(wait);
}

void CatCtl::setPin(QString pin)
{
#if defined ARC_TOKEN_AUTH
    if (arcAuth_)
    {
        arcAuth_->setPin(pin);
    }
#endif
}

void CatCtl::showClientWindow()
{
#if defined(GAME_USING_CLIENT)
    processCtl_->showWindow();
#endif
}

void CatCtl::hideClientWindow()
{
#if defined(GAME_USING_CLIENT)
    processCtl_->hideWindow();
#endif
}

void CatCtl::getIpInfo(DWORD & lastLoginTs, DWORD & lastIp, DWORD & currentIp) const
{
    return gameThread_->getIpInfo(lastLoginTs, lastIp, currentIp);
}

int CatCtl::myLevel()
{
    return gameThread_->myLevel();
}

DWORD CatCtl::getMyId()
{
    ::Account::CharInfo info;
    if (gameThread_->getCurrentChar(info))
    {
        assert(info.id != ::Account::INVALID_ID);
        return info.id;
    }
    else
    {
        assert(0);
        return ::Account::INVALID_ID;
    }
}

QString CatCtl::getMyName()
{
    ::Account::CharInfo info;
    if (gameThread_->getCurrentChar(info))
    {
        assert(!info.name.empty());
        return QString::fromStdWString(info.name);
    }
    else
    {
        assert(0);
        return QString();
    }
}

QString CatCtl::getMarketName() const
{
    return QString::fromStdWString(marketTitle_);
}

bool CatCtl::setMarketName(const QString & title)
{
    if (VMProtectGetSerialNumberState() != 0)
    {
        error(tr("License error. Please check your license is active"));
        return false;
    }

    marketTitle_ = title.toStdWString();
    gameThread_->setMarketTitle(marketTitle_);
    config_.set(L"ShopTitle", marketTitle_);
    config_.save();

    return true;
}

QList<MarketItemSetup> CatCtl::getShopItems()
{
    QList<MarketItemSetup> result;

    // накладываем данные шопконфига на инвентарь. Предметы, которых нет в инвентаре, удаляются из конфига
    // В незалогиненном состоянии, когда нет инвентаря, возвращаем чистый конфиг.
    // Если слот итема действительный, то используется этот слот, иначе подбирается первый 
    // подходящий итем. Слоты должны проставляться действительные.
    // 

    if (VMProtectGetSerialNumberState() != 0)
    {
        error(tr("License error. Please check your license is active"));
        return result;
    }

    // build inventory list

    std::vector<MarketItemSetup> marketSetup = marketSetup_;
    std::vector<InventoryItem> bag = gameThread_->getBag();

    if (bag.empty())    // probably not logged in
    {
        // apply names as we won't get it from inventory
        for (size_t i = 0; i < marketSetup.size(); ++i)
        {
            MarketItemSetup & itemSetup = marketSetup[i];

            if (itemSetup.item.name.empty())
            {
                itemSetup.item.name = getItemName(itemSetup.item.id).toStdWString();
            }
            if (itemSetup.item.icon.empty())
            {
                itemSetup.item.icon = getItemIcon(itemSetup.item.id);
            }
        }

        return QVector<MarketItemSetup>::fromStdVector(marketSetup).toList();
    }

    // на первом проходе строим выходной список предметов по инвентарю.
    // на втором применяем настройки с шопконфига.

    for (size_t iBag = 0; iBag < bag.size(); ++iBag)
    {
        if (bag[iBag].id == Inventory::INVALID_ID)
        {
            continue;   // empty bag slot
        }

        MarketItemSetup item(bag[iBag]);
        item.slot = iBag;
        result.push_back(item);
    }
    // 2
    // совместим конфиг с результатом по слотам
    for (size_t iConfig = 0; iConfig < marketSetup.size(); iConfig++)
    {
        // lookup slot in result
        for (int iResult = 0; iResult < result.size(); ++iResult)
        {
            if (result[iResult].slot == marketSetup[iConfig].slot
                && result[iResult].isEqualItem(marketSetup[iConfig]))
            {
                result[iResult].setFrom(marketSetup[iConfig]);
                marketSetup[iConfig].item.id = Inventory::INVALID_ID;   // do not reuse config
                break;
            }
        }
    }
    // что не подобрано по слоту, поищем первые попавшиеся
    for (size_t iConfig = 0; iConfig < marketSetup.size(); iConfig++)
    {
        // lookup slot in result
        for (int iResult = 0; iResult < result.size(); ++iResult)
        {
            if ( ! result[iResult].doSell
                && ! result[iResult].doBuy      // make sure item not set before
                && result[iResult].isEqualItem(marketSetup[iConfig]))
            {
                result[iResult].setFrom(marketSetup[iConfig]);
                marketSetup[iConfig].item.id = Inventory::INVALID_ID;   // do not reuse config
                break;
            }
        }
    }

    return result;
}

QList<MarketItemSetup> CatCtl::getCurrentShopItems()
{
    QList<MarketItemSetup> result;

    if (VMProtectGetSerialNumberState() != 0)
    {
        error(tr("License error. Please check your license is active"));
        return result;
    }

    // fill in selling
    std::vector<MarketItem>  items = gameThread_->getMarket();
    for (size_t i = 0; i < items.size(); ++i)
    {
        if (items[i].id != Inventory::INVALID_ID
            && items[i].count != 0)
        {
            MarketItemSetup item(items[i]);

            if (items[i].slot >= 0)
            {
                item.doSell     = true;
                item.sellCount  = items[i].count;
                item.sellPrice  = items[i].price;
            }
            else
            {
                item.doBuy      = true;
                item.buyCount   = items[i].count;
                item.buyPrice   = items[i].price;
            }

            result.push_back(item);
        }
    }

    return result;
}

bool CatCtl::setShopItems(const QList<MarketItemSetup> & items)
{
    // При установке фильтруется содержимое shopItems содержимым инвентаря.
    // Отсутсвующие предметы не сохраняются.
    // В незалогиненном состоянии, когда нет инвентаря, сохраняются все предметы.
    // Если слот итема приходит действительный, то сохраняется этот слот, иначе подбирается первый 
    // подходящий итем.
    // 

    if (VMProtectGetSerialNumberState() != 0)
    {
        error(tr("License error. Please check your license is active"));
        return false;
    }

    std::vector<MarketItemSetup>   oldSetup;
    oldSetup.swap(marketSetup_);

    mergeMarketSetup(items, oldSetup);

    gameThread_->setMarketSetup(marketSetup_);
    config_.set(L"ShopItems", marketSetup_, json::DirectConversion<MarketItemSetup>());
    config_.save();

    if (marketOpened_)
    {
        checkMarketToReopen();
    }

    emit marketSetupUpdated();

    return true;
}

InventoryItem CatCtl::getCatshop()
{
    return gameThread_->getCatshop();
}

void CatCtl::getCatshopLimits(const InventoryItem & catshop, unsigned & maxSlots, unsigned & maxChars)
{
    return gameThread_->getCatshopLimits(catshop, maxSlots, maxChars);
}

QList<InventoryItem> CatCtl::getInventoryItems()
{
    return QVector<InventoryItem>::fromStdVector(gameThread_->getBag()).toList();
}

unsigned CatCtl::getMoney()
{
    return gameThread_->getMoney();
}

unsigned CatCtl::getBanknotes()
{
    return gameThread_->getBanknotes();
}

unsigned CatCtl::getGold()
{
    return gameThread_->getGold();
}

qint64 CatCtl::getProfit()
{
    qint64 r = 0;
    if (tradeHistory_)
    {
        const HistoryDb::TradeHistory & h = tradeHistory_->get();
        for (const HistoryDb::ItemHistory & item : h)
        {
            int stock;
            qint64 value;
            qint64 profit;
            getIndicators(item, stock, value, profit);
            r += profit;
        }
    }
    return r;
}

QString CatCtl::getItemName(unsigned itemId)
{
    QString result;

    elements::Item eItem;
    if (g_elements_.getItem(itemId, eItem))
    {
        result = QString::fromStdWString(eItem["Name"]);
    }
    if (result.isEmpty())
    {
        result = spacedNumber(itemId);
    }
    return result;
}

std::string CatCtl::getItemIcon(unsigned itemId)
{
    std::string result;

    elements::Item eItem;
    if (g_elements_.getItem(itemId, eItem))
    {
        result = eItem["file_icon"];
    }
    return result;
}

QString CatCtl::resolvePlayerName(DWORD playerId)
{
    return gameThread_->resolvePlayerName(playerId);
}

const Options & CatCtl::getOptions() const
{
    return options_;
}

void CatCtl::setOptions(const Options & options)
{
    if (VMProtectGetSerialNumberState() != 0)
    {
        error(tr("License error. Please check your license is active"));
        return;
    }

    bool loadExternalWasEnabled(options_.useReopenExternal);

    options >> options_;
#if defined(ARC_TOKEN_AUTH)
    if (!options.arcHwid.isTristate()
        && options.arcHwid.value().empty())
    {
        options_.arcHwid = Vmp::getHwId().toStdWString();
    }
#endif

    options_.validate();
    options_.writeTo(config_);

    gameThread_->setOptions( options_.closeDelayFixed, options_.closeDelayRandom
                           , options_.openDelayFixed, options_.openDelayRandom
                           , options_.onlineReg
                           , options_.lowConverter ? options_.lowConverterValue : 0
                           , options_.highConverter ? options_.highConverterValue : 0);

    updateProxy();

    // start or stop previously started timer
    if (marketOpened_)
    {
        if (loadExternalWasEnabled && ! options_.useReopenExternal)
        {
            loadExternalConfigTimer_.stop();
        }
        else if (!loadExternalWasEnabled && options_.useReopenExternal)
        {
            if (options_.useReopenExternal && options_.reopenExternalInterval > 0)
            {
                loadExternalConfig();
                loadExternalConfigTimer_.start(1000 * options_.reopenExternalInterval);
            }
        }
    }
}

bool CatCtl::openMarket()
{
    if (VMProtectGetSerialNumberState() != 0)
    {
        error(tr("License error. Please check your license is active"));
        return false;
    }

    marketShouldBeOpened_ = true;
    marketReopenPending_ = false;

    return gameThread_->openMarket();
}

void CatCtl::closeMarket()
{
    if (VMProtectGetSerialNumberState() != 0)
    {
        error(tr("License error. Please check your license is active"));
    }

    marketShouldBeOpened_ = false;
    marketReopenPending_ = false;

    gameThread_->closeMarket();
}

bool CatCtl::isMarketOpened() const
{
    return gameThread_->isMarketOpened();
}

const HistoryDb * CatCtl::tradeHistory() const
{
    return tradeHistory_;
}

void CatCtl::resetProfit()
{
    if (tradeHistory_)
    {
        tradeHistory_->resetProfit();
        emit profitUpdated();
    }
}

unsigned CatCtl::getStockBuyPrice(unsigned itemId) const
{
    if (!tradeHistory_)
    {
        return 0;
    }

    // нужно найти первую актуальную закупку
    const HistoryDb::ItemHistory & history = tradeHistory_->get(itemId);
    for (const HistoryItem & item : history)
    {
        if (item.operation == HistoryItem::OperationBuy
            && item.isActual)
        {
            return item.pricePerItem;
        }
    }
    return 0;
}

void CatCtl::revive()
{
    gameThread_->revive();
}

unsigned CatCtl::lockTime() const
{
    return gameThread_->lockTime();
}

void CatCtl::sendPrivateMessage(const QString & name, const QString & text)
{
    gameThread_->sendPrivateMessage(name, text);
}

void CatCtl::sendPublicMessage(const QString & text)
{
    gameThread_->sendPublicMessage(text);
}

void CatCtl::sendGroupMessage(const QString & text)
{
    gameThread_->sendGroupMessage(text);
}

void CatCtl::sendGuildMessage(const QString & text)
{
    gameThread_->sendGuildMessage(text);
}

void CatCtl::updateCommi(bool trackProgress)
{
    gameThread_->updateCommi(trackProgress);
}

time_t CatCtl::commiLastUpdated() const
{
    return gameThread_->commiLastUpdated();
}

std::vector<CommiShop> CatCtl::getCommi()
{
    return gameThread_->getCommi();
}

QString CatCtl::getPcName() const
{
    QString name;
    #if defined(ARC_TOKEN_AUTH)
    name = arcAuth_ ? arcAuth_->pcName() : QString();
    #endif
    return name;
}

//

void CatCtl::acceptTrade(DWORD tradeId)
{
    gameThread_->acceptTrade(tradeId);
}

void CatCtl::rejectTrade(DWORD tradeId)
{
    gameThread_->rejectTrade(tradeId);
}

void CatCtl::discardTrade()
{
    gameThread_->discardTrade();
}

void CatCtl::addTradeItem(unsigned inventorySlot, unsigned count)
{
    gameThread_->addTradeItem(inventorySlot, count);
}

void CatCtl::addTradeMoney(int money)
{
    gameThread_->addTradeMoney(money);
}

void CatCtl::removeTradeItem(unsigned inventorySlot, unsigned count)
{
    gameThread_->removeTradeItem(inventorySlot, count);
}

void CatCtl::removeTradeMoney(int money)
{
    gameThread_->removeTradeMoney(money);
}

void CatCtl::confirmTrade()
{
    gameThread_->confirmTrade();
}

void CatCtl::submitTrade()
{
    gameThread_->submitTrade();
}

void CatCtl::setLockTime(unsigned seconds)
{
    gameThread_->setLockTime(seconds);
}

void CatCtl::buyCatshop(unsigned itemShopId)
{
    unsigned index;
    const GShop::Item & item = GShop::GShop::instance()->item(itemShopId, &index);
    if ( ! item.isNull())
    {
        gameThread_->buyCatshop(item.itemId, item.itemAmount, index);
    }
}

void CatCtl::equipCatShop(unsigned inventorySlot)
{
    gameThread_->equipCatShop(inventorySlot);
}

void CatCtl::swapItems(unsigned slot1, unsigned slot2)
{
    gameThread_->swapItems(slot1, slot2);
}

void CatCtl::moveItems(unsigned srcSlot, unsigned dstSlot, unsigned count)
{
    gameThread_->moveItems(srcSlot, dstSlot, count);
}

//

void CatCtl::onDisconnected()
{
    setupReconnect();
    if (autoPriceMonitor_)
    {
        autoPriceMonitor_->removeMonitoring(this);
    }
    loadExternalConfigTimer_.stop();
    marketOpened_ = false;
}

void CatCtl::onReconnectTimer()
{
    if (suppressReconnect_)
    {
        remainingToReconnect_ = -1;
    }

    emit reconnectEvent(remainingToReconnect_);

    if (remainingToReconnect_ < 0)
    {
        reconnectTimer_.stop();
    }

    remainingToReconnect_ --;
}

#if defined MAILRU_TOKEN_AUTH
void CatCtl::onAuthError(const QString & str)
{
    lastAuthError_ = str;
}

void CatCtl::onAuthFinished(bool success)
{
    mailruAuth_->deleteLater();

    if (!success)
    {
        int code = mailruAuth_->statusCode();
        if (code != PwMailRu::None)
        {
            QString message = mailruAuthErrorToString(code);
            error(message);
        }
        else
        {
            error(lastAuthError_);
        }
        disconnected();

        if (code == PwMailRu::StatusCode::None)
        {
            // might be a network error
            setupReconnect();
        }
        return;
    }

    QString token = mailruAuth_->token();
    if (token.isEmpty())
    {
        error(tr("Failed to get authorization token"));
        disconnected();
        return;
    }

    QList<PwMailRu::GameAccount> accounts = mailruAuth_->accounts();
    if (accounts.isEmpty())
    {
        error(tr("No accounts found on this email"));
        disconnected();
        return;
    }

    int accId = 0;

    QListIterator<PwMailRu::GameAccount> it(accounts);
    while (it.hasNext())
    {
        const PwMailRu::GameAccount & account = it.next();
        if (accounts_[accountIndex_].name == account.name)
        {
            accId = account.id;
            break;
        }
    }
    if (accId == 0)
    {
        error(tr("Account %1 not found on this email").arg(accounts_[accountIndex_].name));
        disconnected();
        return;
    }

    tropophobiaWarned_ = false;

    bool forcedLogin;
    config_.get(L"ForcedLogin", forcedLogin);
    const Server & s = servers_.at(currentServer_);

#if defined GAME_USING_CLIENT
    quint16 port = localPort_.value();
    if (port == 0)
    {
        error(tr("No local ports available"));
        disconnected();
        return;
    }

    gameThread_->start(QSharedPointer<qlib::ConnectionGenerator>(new ConnectionGeneratorProxy(
                    processCtl_
                    , g_elementExePath_
                    , port
                    , QString::number(accId)
                    , token
                    , s.host + L':' + s.port
                    , proxy_
                    , forcedLogin)));
#else
    gameThread_->start(QSharedPointer<qlib::ConnectionGenerator>(new qlib::ConnectionGeneratorTcp(
                    QString::number(accId).toStdWString()
                    , token.toStdWString()
                    , s.host + L':' + s.port
                    , proxy_
                    , forcedLogin)));
#endif
}

#elif defined ARC_TOKEN_AUTH
void CatCtl::onAuthError(const QString & str)
{
    lastAuthError_ = str;
}

void CatCtl::onAuthFinished(bool success)
{
    arcAuth_->deleteLater();

    if (!success)
    {
        int code = arcAuth_->statusCode();
        if (code != ARC::ArcAuth::None)
        {
            if (code == ARC::ArcAuth::PinRequired)
            {
                authGetPin();
                return;
            }
            else
            {
                QString message = arcAuthErrorToString(code);
                error(message);
            }
        }
        else
        {
            auto e = arcAuth_->getError();
            if (e.code != ERR_NO_ERROR)
            {
                error(QString::fromStdWString(e.message));
            }
            else if (!lastAuthError_.isEmpty())
            {
                error(lastAuthError_);
            }

            setupReconnect();
        }

        disconnected();
        return;
    }

    QString token = arcAuth_->token();
    if (token.isEmpty())
    {
        error(tr("Failed to get authorization token"));
        disconnected();
        return;
    }

    bool forcedLogin;
    config_.get(L"ForcedLogin", forcedLogin);
    const Server & s = servers_.at(currentServer_);

    std::wstring email;
    config_.get(L"Email", email);

#if defined GAME_USING_CLIENT
    quint16 port = localPort_.value();
    if (port == 0)
    {
        error(tr("No local ports available"));
        disconnected();
        return;
    }

    gameThread_->start(QSharedPointer<qlib::ConnectionGenerator>(new ConnectionGeneratorProxy(
        processCtl_
        , g_elementExePath_
        , port
        , QString::fromStdWString(email)
        , token
        , s.host + L':' + s.port
        , proxy_
        , forcedLogin
        , g_isSafeProcessMode)
        ));
#else
    gameThread_->start(QSharedPointer<qlib::ConnectionGenerator>(new qlib::ConnectionGeneratorTcp(
        email
        , token.toStdWString()
        , s.host + L':' + s.port
        , proxy_
        , forcedLogin)));
#endif
}

#endif

void CatCtl::onClientStopped()
{
    clientStopped();
    gameThread_->stop(false);
}

void CatCtl::onLoggedIn()
{
    if (options_.tropophobia && tropophobiaWarned_)
    {
        event(ImmobilizerWarned);
        disconnect(false);
    }
#if defined GAME_USING_CLIENT
    processCtl_->flushMemory();
#endif

    if (Vmp::hasOption(Vmp::OptionHistory))
    {
        QDir db(qApp->applicationDirPath());
        if (!db.exists("db")
            && !db.mkdir("db"))
        {
            error(tr("Failed to create db folder, trade history will not be recorded"));
        }
        else
        {
            unsigned playerId = getMyId();
            QString filename = QString("%01/db/%02%03")
                .arg(qApp->applicationDirPath())
                .arg(currentServer_, 2, 10, QChar('0'))
                .arg(playerId, 8, 16, QChar('0'));

            if (!tradeHistory_->open(filename))
            {
                error(tr("Failed to create/open history file (%1), trade history will not be recorded")
                      .arg(filename));
            }
        }
    }

    emit loggedIn();
}

void CatCtl::onMarketMove()
{
//     std::vector<GameThread::ShortMarketItem> marketItems = buildMarketList(marketConfig_, gameThread_.data());
//     gameThread_->setMarketSetup(marketItems, marketTitle_);
// 
//     event(MarketMoving);
}

void CatCtl::onMarketError()
{
    error(tr("Market is not opened"));
}

void CatCtl::onMarketEmpty()
{
    error(tr("Market is not opened: no items to trade"));
}

void CatCtl::onEvent(const qlib::GameEvent & event)
{
    switch (event->type())
    {
        case BaseEvent::CharSelectRequest:
            requestCharSelect();
            break;

        case BaseEvent::PlayerKilled:
        {
            marketReopenPending_ = false;
            break;
        }

        case BaseEvent::MarketItemSold:
        {
            const ItemSoldEvent *e = static_cast<ItemSoldEvent*>(event.data());

            if (tradeHistory_)
            {
                if (!tradeHistory_->append(e->itemId(),
                                           HistoryItem(QDateTime::currentDateTime().toTime_t()
                                                       , HistoryItem::OperationSell
                                                       , e->count()
                                                       , e->price())
                                           ))
                {
                    error(tr("Failed to record sell operation"));
                }
            }

            if (marketOpened_)
            {
                QTimer::singleShot(5000, this, SLOT(checkMarketToReopen()));
            }
            break;
        }

        case BaseEvent::MarketItemPurchased:
        {
            const ItemPurchasedEvent *e = static_cast<ItemPurchasedEvent*>(event.data());


            if (tradeHistory_)
            {
                if (!tradeHistory_->append(e->itemId(),
                                           HistoryItem(QDateTime::currentDateTime().toTime_t()
                                                       , HistoryItem::OperationBuy
                                                       , e->count()
                                                       , e->price())
                                           ))
                {
                    error(tr("Failed to record buy operation"));
                }
            }

            if (marketOpened_)
            {
                QTimer::singleShot(5000, this, SLOT(checkMarketToReopen()));
            }
            break;
        }

        case BaseEvent::PlayerMarketState:
        {
            const PlayerMarketStateEvent *e = static_cast<const PlayerMarketStateEvent*>(event.data());
            marketOpened_ = e->isOpened();
            if (marketOpened_)
            {
                marketReopenPending_ = false;

                lastSavedPos_ = gameThread_->getPlayerPos();
                config_.set(L"LastPosX", lastSavedPos_.x());
                config_.set(L"LastPosY", lastSavedPos_.y());
                config_.set(L"LastPosZ", lastSavedPos_.z());

                if (options_.saveExternal)
                {
                    saveMarketSetup();
                }
                if (options_.useReopenExternal && options_.reopenExternalInterval > 0)
                {
                    loadExternalConfigTimer_.start(1000 * options_.reopenExternalInterval);
                }
            }
            else
            {
                loadExternalConfigTimer_.stop();
            }

            if (autoPriceMonitor_)
            {
                if (marketOpened_)
                {
                    autoPriceMonitor_->addMonitoring(this);
                }
                else
                {
                    autoPriceMonitor_->removeMonitoring(this);
                }
            }

            break;
        }

        case BaseEvent::PlayerPositionNotify:
        {
            if (options_.tropophobia &&
                !lastSavedPos_.isNull())
            {
                const PlayerPositionNotifyEvent *e = static_cast<const PlayerPositionNotifyEvent*>(event.data());
                if (!e->position().equalsTo(lastSavedPos_, 10.))
                {
                    tropophobiaWarned_ = true;
                }
            }
            break;
        }
    }
}

void CatCtl::checkMarketToReopen()
{
    if (marketReopenPending_)
    {
        Log("Not reopening, already pending");
        return;
    }

    unsigned money = gameThread_->getMoney();
    bool hasCatshop = ! gameThread_->getCatshop().isEmpty();
    unsigned buyLimit = money;
    if (hasCatshop
        && options_.highConverter
        && options_.highConverterValue > 1)
    {
        // лимит покупки ограничен сверху конвертером монеток в банкноты, при этом если установлено на 100, то лимит ~90 т.к. на 101 меняется бумажка и остается 91
        buyLimit = std::min(money, (options_.highConverterValue - 1) * 10000000);
    }
    std::vector<ShortMarketItem> newItems = buildMarketList( gameThread_->getBag()
                                                           , marketSetup_
                                                           , (hasCatshop ? InventoryMoneyLimitCatshop : InventoryMoneyLimit) - money
                                                           , buyLimit);
    if (newItems.size() == 0)
    {
        Log("Closing market: no more items to trade");
        gameThread_->closeMarket(true);
        return;
    }

    if ( ! options_.useReopen || ! marketShouldBeOpened_)
    {
        Log("No reopen by options");
        return;
    }

    bool doReopen = false;

    std::vector<MarketItem> currentItems = gameThread_->getMarket();
    if (newItems.size() != currentItems.size())
    {
        Log("Scheduling market reopening by item count change");
        doReopen = true;
    }
    else if (options_.useReopenPercentage || options_.useReopenValue)
    {
        for (size_t i = 0; i < newItems.size(); i++)
        {
            if (newItems[i].itemId != currentItems[i].id
                || newItems[i].price != currentItems[i].price
                || newItems[i].slot != currentItems[i].slot)
            {
                Log("Scheduling market reopening by id/price/slot change");
                doReopen = true;
                break;
            }

            // при уменьшении планируемых предметов переставляемся всегда
            if ((int)currentItems[i].count < newItems[i].count)
            {
                if (newItems[i].count > int(options_.reopenModeSelector))
                {
                    // big mode
                    Log("Item %i (%i) big mode: current %i, to market %i", newItems[i].itemId, i, currentItems[i].count, newItems[i].count);

                    if (options_.useReopenPercentage)
                    {

                        int difference = newItems[i].count - int(currentItems[i].count);
                        if (difference > int(double(newItems[i].count) * options_.reopenPercentage / 100))
                        {
                            Log("Scheduling market reopening by percent change (big mode)");
                            doReopen = true;
                            break;
                        }
                    }
                }
                else if (options_.useReopenValue
                        && currentItems[i].count <= (unsigned)options_.reopenValue)
                {
                    // small mode
                    Log("Scheduling market reopening by count change (small mode)");
                    doReopen = true;
                    break;
                }
            }
            else if ((int)currentItems[i].count > newItems[i].count)
            {
                Log("Scheduling market reopening by count decrease");
                doReopen = true;
                break;
            }
        }
    }

    if (doReopen)
    {
        marketReopenPending_ = true;

        event(MarketReopening);
        gameThread_->reopenMarket();
    }
}

void CatCtl::loadExternalConfig()
{
    assert(marketOpened_);

    auto myName = getMyName();
    if (myName.isEmpty())
    {
        qDebug() << "Failed to load external config: no name";
        return;
    }

    QString filename = sanitizeFilename(
        QString("%1-%2.upd")
        .arg(currentServer_)
        .arg(myName));

    QFile file(filename);
    if (file.exists())
    {
        if (file.open(QIODevice::ReadOnly) 
            && file.size() < 1024 * 1024)       // reasonably should be less than 1M
        {
            QByteArray data = file.readAll();

            JsonValue json;
            if (json.read(QString::fromUtf8(data).toStdWString()))
            {
                std::vector<MarketItemSetup> marketSetup;
                json.get(L"ShopItems", marketSetup, json::DirectConversion<MarketItemSetup>());
                if (marketSetup.size() > 0)
                {
                    message(tr("Updating market setup"));

                    std::vector<MarketItemSetup>   oldSetup;
                    oldSetup.swap(marketSetup_);    // clear and store

                    mergeMarketSetup(QList<MarketItemSetup>::fromVector(QVector<MarketItemSetup>::fromStdVector(marketSetup)), oldSetup);
                    // merge if anything left untouched
                    marketSetup_.insert(marketSetup_.end(), oldSetup.begin(), oldSetup.end());

                    gameThread_->setMarketSetup(marketSetup_);
                    config_.set(L"ShopItems", marketSetup_, json::DirectConversion<MarketItemSetup>());
                    config_.save();

                    std::wstring marketName;
                    json.get(L"ShopTitle", marketName);
                    setMarketName(QString::fromStdWString(marketName));

                    emit marketSetupUpdated();

                    if (marketOpened_)
                    {
                        checkMarketToReopen();
                    }
                }
            }

            file.close();
        }
        file.remove();
    }
}

void CatCtl::mergeMarketSetup(const QList<MarketItemSetup> & items, std::vector<MarketItemSetup> & oldSetup)
{
    std::vector<InventoryItem> bag = gameThread_->getBag();

    for (int iShop = 0; iShop < items.size(); iShop++)
    {
        MarketItemSetup si = items[iShop];

        // сливаем конфиг с текущим - не заменяем поля с третьим состоянием (берем старые)
        for (size_t iOldConfig = 0; iOldConfig < oldSetup.size(); iOldConfig++)
        {
            const MarketItemSetup & oi = oldSetup[iOldConfig];

            if (oi.isEqualItem(si))
            {
                if (si.slot.isTristate())       si.slot = oi.slot;
                if (si.doSell.isTristate())     si.doSell = oi.doSell;
                if (si.sellCount.isTristate())  si.sellCount = oi.sellCount;
                if (si.sellPrice.isTristate())  si.sellPrice = oi.sellPrice;
                if (si.doBuy.isTristate())      si.doBuy = oi.doBuy;
                if (si.buyCount.isTristate())   si.buyCount = oi.buyCount;
                if (si.buyPrice.isTristate())   si.buyPrice = oi.buyPrice;
                if (si.keepMin.isTristate())    si.keepMin = oi.keepMin;
                if (si.keepMax.isTristate())    si.keepMax = oi.keepMax;

                oldSetup.erase(oldSetup.begin() + iOldConfig);    // do not reuse config
                break;
            }
        }

        // для предметов, не найденных в старом конфиге 
        // очищаем tristate состояния так как больше ничего с ними сделать не сможем
        if (si.slot.isTristate())       si.slot = -1;
        if (si.doSell.isTristate())     si.doSell = false;
        if (si.sellCount.isTristate())  si.sellCount = 0;
        if (si.sellPrice.isTristate())  si.sellPrice = 0;
        if (si.doBuy.isTristate())      si.doBuy = false;
        if (si.buyCount.isTristate())   si.buyCount = 0;
        if (si.buyPrice.isTristate())   si.buyPrice = 0;
        if (si.keepMin.isTristate())    si.keepMin = 0;
        if (si.keepMax.isTristate())    si.keepMax = 0;

        si.normalize();

        // skip items without operation
        if (!si.doSell && !si.doBuy)
        {
            continue;
        }

        if (bag.empty())    // probably not logged in
        {
            marketSetup_.push_back(si);
        }
        else
        {
            // подбираем слот инвентаря, соответствующий конфигу
            if (si.slot >= 0
                && si.slot < int(bag.size())
                && bag[si.slot].id == si.item.id
                && bag[si.slot].propsEqualTo(si.item.props))
            {
                marketSetup_.push_back(si);
                bag.at(si.slot).id = Inventory::INVALID_ID;     // do not reuse item
                continue;
            }
            // иначе ищем первый подходящий
            for (size_t iBag = 0; iBag < bag.size(); iBag++)
            {
                if (bag[iBag].id == si.item.id
                    && bag[iBag].propsEqualTo(si.item.props))
                {
                    si.slot = iBag;
                    marketSetup_.push_back(si);
                    bag.erase(bag.begin() + iBag);              // do not reuse config
                    break;
                }
            }
        }
    }
}

//

void CatCtl::setupReconnect()
{
    if (suppressReconnect_)
    {
        return;
    }

    if (options_.autoReconnect 
        && (options_.reconnectDelayFixed >= 0 || options_.reconnectDelayRandom >= 0))
    {
        remainingToReconnect_ = options_.reconnectDelayFixed 
            + int(double(qrand()) * options_.reconnectDelayRandom / RAND_MAX);
        reconnectTimer_.start();
    }
}

void CatCtl::requestCharSelect()
{
    if (accountIndex_ < 0 || (size_t)accountIndex_ >= accounts_.size())
    {
        assert(0);
        event(CharselectFailed);
        return;
    }

    const ::Account::CharList charlist = gameThread_->getCharList();
    if (charlist.size() == 0)
    {
        event(CharselectNoChars);
        selectChar(-1, -1);
        return;
    }

    // update char list and current positions

    Account & currentAcc = accounts_[accountIndex_];

    // remember previously selected char
    std::wstring charName;
    if (charIndex_ >= 0)
    {
        charName = currentAcc.characters[charIndex_].toStdWString();
    }

    // clear current char selection
    int oldSelection = charIndex_;
    charIndex_ = -1;

    // copy char names to new list

    currentAcc.characters.clear();
    for (size_t i = 0; i < charlist.size(); i++)
    {
        if (charlist[i].name.empty())
        {
            // this is like a deleted character or smth
            continue;
        }
        currentAcc.characters << QString::fromStdWString(charlist[i].name);

        // set current selection to the previously selected position 
        if (oldSelection >= 0 && charName == charlist[i].name)
        {
            charIndex_ = i;
        }
    }

    config_.set(L"Accounts", accounts_, json::DirectConversion<Account>());

    if (charlist.size() == 1 && charIndex_ != 0)
    {
        charIndex_ = 0;
    }

    if (charIndex_ >= 0)
    {
        if (selectChar(accountIndex_, charIndex_))
        {
            event(CharselectAuto);
        }
        else
        {
            event(CharselectFailed);
        }
    }
    else
    {
        event(CharselectRequest);
    }
}

void CatCtl::updateProxy()
{
    if (!options_.useProxy)
    {
        proxy_ = QNetworkProxy();
        return;
    }

    // parse host and port

    quint16 port = options_.proxyType == Options::ProxySocks ? 1080 : 80;
    std::wstring address = options_.proxyAddr;

    size_t delim = address.find(':');
    if (delim != std::wstring::npos)
    {
        std::wstring sPort = address.substr(delim + 1);
        quint16 p = quint16(QString::fromStdWString(sPort).toInt());
        if (p != 0)
        {
            port = p;
        }

        address = address.substr(0, delim);
    }

    proxy_.setType(options_.proxyType == Options::ProxySocks ? QNetworkProxy::Socks5Proxy : QNetworkProxy::HttpProxy);
    proxy_.setHostName(QString::fromStdWString(address));
    proxy_.setPort(port);
    proxy_.setUser(std::wstring(options_.proxyUser).empty() ? QString() : QString::fromStdWString(options_.proxyUser));
    proxy_.setPassword(std::wstring(options_.proxyPass).empty() ? QString() : QString::fromStdWString(options_.proxyPass));
}

void CatCtl::saveMarketSetup()
{
    JsonValue json;

    class BagConverter
    {
    public:
        JsonValue convert(const InventoryItem & item) const
        {
            JsonValue config;
            if (item.id == InventoryItem::INVALID_ID)
            {
                return config;
            }

            config.set(L"Id", item.id);
            config.set(L"Count", item.count);
            return config;
        }
    };

    std::vector<InventoryItem> bag = gameThread_->getBag();
    json.set(L"Bag", bag, BagConverter());

    json.set(L"ShopItems", marketSetup_, json::DirectConversion<MarketItemSetup>());
    json.set(L"ShopTitle", marketTitle_);
	json.set(L"Money", gameThread_->getMoney());

    json.set(L"LastUpdate", QDateTime::currentDateTime().toTime_t());

    unsigned myID = getMyId();
    json.set(L"wId", myID);

    auto myName = getMyName();
    assert(!myName.isEmpty());
    QString filename = QString("%1-%2.set")
        .arg(currentServer_)
        .arg(myName);
    json.saveTo(sanitizeFilename(filename).toStdWString(), false);
}

#if GAME_OWNER_TYPE==2
void CatCtl::loadPwcatsIndex()
{
    pwcatsIndex_.clear();

    QFile file("pwcatsindex.txt");
    if (!file.exists()
        || !file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return;
    }

    while (!file.atEnd())
    {
        QString line = file.readLine();
        QStringList parts = line.split(QRegularExpression("[\\t]+"), QString::SkipEmptyParts);
        if (parts.count() == 2)
        {
            bool r;
            unsigned index = parts[1].toUInt(&r);
            if (r)
            {
                pwcatsIndex_[parts[0].trimmed()] = index;
            }
        }
    }
}

#endif
