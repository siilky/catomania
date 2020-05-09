
#include "stdafx.h"

#include "event.h"
#include "game/inventory_types.h"

#include "ui_catview.h"
#include "uicatview.h"

#include "uiabout.h"
#include "uiaccounts.h"
#include "uicharsel.h"
#include "uiconfig.h"
#include "uitrade.h"
#include "uilock.h"
#include "uihistoryview.hpp"
#include "uigetpin.h"

#include "util.h"
#include "clicker.h"
#include "qlib/vmp.h"
#include "license.h"
#include "chat.hpp"
#include "uicommi.h"

Q_DECLARE_METATYPE(CatCtl::AccountSelection)

CatView::CatView(QSharedPointer<CatCtl> ctl, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CatView)
    , lbStatusLock_(new QLabel)
    , lbStatusLockImage_(new QLabel)
    , lbStatusMarket_(new QLabel)
    , lbStatusMoney_(new QLabel)
    , lbStatusSelling_(new QLabel)
    , lbStatusBuying_(new QLabel)
    , lbStatusGold_(new QLabel)
    , ctl_(ctl)
    , isLoggedIn_(false)
    , timerState_(false)
    , lockState_(-1)
    , isMarketOpened_(false)
    , isDead_(false)
    , saveLog_(false)
    , rotateLog_(false)
{
    ui->setupUi(this);
    ui->cbAccounts->view()->setTextElideMode(Qt::ElideNone);

    chat_ = new Chat(ctl);
    ui->tabWidget->addTab(chat_, tr("Chat"));   // owns chat
    
    // Optional button
    VMProtectBeginVirtualization(__FUNCTION__);
    if (Vmp::hasOption(Vmp::OptionHistory))
    {
        lbStatusProfit_ = new QLabel();
    }
    else
    {
        lbStatusProfit_ = 0;
        ui->btnHistory->hide();
    }
    VMProtectEnd();

#if CLIENT_VERSION <= 1420   // determine correct version
    ui->btnCommi->hide();
#endif


    // status bar

    // ugly statusbar painting
    ui->statusBar->setStyleSheet("QStatusBar::item { margin:0 -1 0 -1; border: none; border-right: 1px solid white; border-left: 1px solid lightgray; }");

    copyAction1_ = new QAction(QIcon(":/catView/copying_and_distribution.png"), "", this);
    copyAction2_ = new QAction(QIcon(":/catView/copying_and_distribution.png"), "", this);
    copyAction3_ = new QAction(QIcon(":/catView/copying_and_distribution.png"), "", this);

    QSignalMapper *signalMapper = new QSignalMapper(this);
    signalMapper->setMapping(copyAction1_, lbStatusMoney_);
    signalMapper->setMapping(copyAction2_, lbStatusSelling_);
    signalMapper->setMapping(copyAction3_, lbStatusBuying_);
    connect(copyAction1_, &QAction::triggered, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    connect(copyAction2_, &QAction::triggered, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    connect(copyAction3_, &QAction::triggered, signalMapper, static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
    connect(signalMapper, static_cast<void (QSignalMapper::*)(QWidget*)>(&QSignalMapper::mapped), this, &CatView::copyToCb);
    //qconnect(signalMapper, SIGNAL(mapped(QWidget *)), this, SLOT(copyToCb(QWidget *)));
    lbStatusMoney_->addAction(copyAction1_);
    lbStatusSelling_->addAction(copyAction2_);
    lbStatusBuying_->addAction(copyAction3_);
    lbStatusMoney_->setContextMenuPolicy(Qt::ActionsContextMenu);
    lbStatusSelling_->setContextMenuPolicy(Qt::ActionsContextMenu);
    lbStatusBuying_->setContextMenuPolicy(Qt::ActionsContextMenu);

    QString style("QLabel { min-width: 90px; padding-left: 2px; padding-right: 2px; }");
    lbStatusMoney_->setStyleSheet(style);
    lbStatusSelling_->setStyleSheet(style);
    lbStatusBuying_->setStyleSheet(style);
    lbStatusGold_->setStyleSheet("QLabel { padding-left: 2px; padding-right: 2px; }");
    if (lbStatusProfit_)
    {
        lbStatusProfit_->setStyleSheet(style);
    }

    lbStatusMoney_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    lbStatusSelling_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    lbStatusBuying_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    lbStatusGold_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    if (lbStatusProfit_)
    {
        lbStatusProfit_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    }

    if (lbStatusProfit_)
    {
        ui->statusBar->addPermanentWidget(lbStatusProfit_);
    }
    ui->statusBar->addPermanentWidget(lbStatusGold_);
    ui->statusBar->addPermanentWidget(lbStatusMoney_);
    ui->statusBar->addPermanentWidget(lbStatusSelling_);
    ui->statusBar->addPermanentWidget(lbStatusBuying_);

    // market status

    lbStatusMarket_->setFixedWidth(18);
    lbStatusMarket_->setAlignment(Qt::AlignCenter);
    ui->statusBar->addPermanentWidget(lbStatusMarket_);

    // lock icon and text

    lbStatusLockImage_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lbStatusLock_->setAlignment(Qt::AlignCenter);

    QWidget *lockWidget = new QWidget();
    QLayout *lockLayout = new QHBoxLayout(lockWidget);
    lockLayout->setSpacing(2);
    lockLayout->setContentsMargins(0, 0, 0, 0);
    lockLayout->addWidget(lbStatusLockImage_);
    lockLayout->addWidget(lbStatusLock_);
    lockWidget->setCursor(QCursor(Qt::PointingHandCursor));
    Clicker * clicker = new Clicker(lockWidget);
    lockWidget->installEventFilter(clicker);
    connect(clicker, &Clicker::clicked, this, &CatView::displaySetLockTime);

    ui->statusBar->addPermanentWidget(lockWidget);

    // Question status icon
    lbHelp_ = new QLabel();
    lbHelp_->setCursor(QCursor(Qt::PointingHandCursor));
    lbHelp_->setAlignment(Qt::AlignCenter);
    lbHelp_->setFixedWidth(18);
    connect(lbHelp_, &QLabel::linkActivated, this, &CatView::displayAbout);
    updateLicenseState(SERIAL_STATE_FLAG_INVALID);
    ui->statusBar->addPermanentWidget(lbHelp_);


    updateText();

    //

    connect(&LicenseStorage::instance(), &LicenseStorage::licenseStateChanged, this, &CatView::updateLicenseState);

    connect(ctl_.data(), &CatCtl::message,          this, &CatView::logMessage);
    connect(ctl_.data(), &CatCtl::connected,        this, &CatView::onConnected);
    connect(ctl_.data(), &CatCtl::loggedIn,         this, &CatView::onLoggedIn);
    connect(ctl_.data(), &CatCtl::disconnected,     this, &CatView::onDisconnected);
    connect(ctl_.data(), &CatCtl::completed,        this, &CatView::onCompleted);
    connect(ctl_.data(), &CatCtl::error,            this, &CatView::logError);
    connect(ctl_.data(), &CatCtl::event,            this, &CatView::onEvent);
    connect(ctl_.data(), &CatCtl::gameEvent,        this, &CatView::onGameEvent);
    connect(ctl_.data(), &CatCtl::reconnectEvent,   this, &CatView::onReconnectEvent);
    connect(ctl_.data(), &CatCtl::lockInfo,         this, &CatView::onLockInfo);
    connect(ctl_.data(), &CatCtl::authProgress,     this, &CatView::onAuthProgress);
    connect(ctl_.data(), &CatCtl::authGetPin,       this, &CatView::onAuthGetPin);
    connect(ctl_.data(), &CatCtl::clientStarted,    this, &CatView::onClientStarted);
    connect(ctl_.data(), &CatCtl::clientStopped,    this, &CatView::onClientStopped);
    connect(ctl_.data(), &CatCtl::clientWindowClosed, this, &CatView::onClientWindowClosed);
    connect(ctl_.data(), &CatCtl::profitUpdated,    this, &CatView::updateProfit);

    //

    setShopState(false);
    setConnectedState(false);
    setLoggedInState(false);

    const Options & opts = ctl_->getOptions();
    ui->cbAutoLogin->setChecked(opts.autoLogin);
    ui->cbAutorelogin->setChecked(opts.autoReconnect);
    ui->cbForcedLogin->setChecked(opts.forcedLogin);
    ui->cbAutoMarket->setChecked(opts.autoShop);
    ui->cbTropophobia->setChecked(opts.tropophobia);
    updateOptions();

    ctl_->getServers(currentServer_);
    accounts_ = ctl_->getAccounts(currentAccount_);
    updateAccountsCb();
}

CatView::~CatView()
{
}

void CatView::setClientExePath(const QString & exePath)
{
    if (exePath.isEmpty())
    {
        return;
    }

    clientExePath_ = exePath;
}

QString CatView::currentAccount() const
{
    return getCurrentName();
}

CatView::CatState CatView::state() const
{
    if (timerState_)
    {
        return Waiting;
    }
    else if ( ! isLoggedIn_)
    {
        return Offline;
    }
    else if (isDead_)
    {
        return Dead;
    }
    else if (isMarketOpened_)
    {
        return MarketOpened;
    }
    else if (lockState_ == 1)
    {
        return Locked;
    }
    else
    {
        return Online;
    }
}

void CatView::logMessage(const QString & msg)
{
    QDateTime now = QDateTime::currentDateTime();
    writeLog(now, msg);

    QString colored = msg;
    colored.replace(QRegExp("\\^([0-9A-F]{6})(.+)(?=(/\\^))(/\\^)"), "<font color=\"#\\1\">\\2</font>");
    QString time = now.toString("MM-dd hh:mm:ss");
    QString formatted = QString("<font color=\"lightgray\">[%1]</font> %2").arg(time).arg(colored);

    // keep extra line at the end
    QTextCursor cursor = ui->teLog->textCursor();
    //cursor.movePosition(QTextCursor::PreviousBlock, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::End, QTextCursor::MoveAnchor);
    cursor.insertHtml(formatted);
    cursor.insertBlock();
    ui->teLog->setTextCursor(cursor);
}

void CatView::disableAll()
{
    ui->btnCloseMarket->setEnabled(false);
    ui->btnOpenMarket->setEnabled(false);
    ui->btnCommi->setEnabled(false);
    ui->btnLogin->setEnabled(false);
    ui->btnLogout->setEnabled(false);
    ui->btnAccountsSetup->setEnabled(false);
    ui->btnMarketSetup->setEnabled(false);
    ui->cbAccounts->setEnabled(false);
}

//

void CatView::changeEvent(QEvent *event)
{
    switch (event->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            updateText();
            break;
    }

    QWidget::changeEvent(event);
}

//

void CatView::on_cbAccounts_currentIndexChanged(int index)
{
    if (index >= 0)
    {
        currentAccount_ = ui->cbAccounts->itemData(index).value<CatCtl::AccountSelection>();
    }
    else
    {
        currentAccount_.accountIndex = -1;
    }

    accountUpdate(getCurrentName());
}

void CatView::on_btnAccountsSetup_clicked()
{
    QScopedPointer<AccountsView> acc(new AccountsView(ctl_, this));
    acc->exec();
    accounts_ = ctl_->getAccounts(currentAccount_);
    ctl_->getServers(currentServer_);
    updateAccountsCb();
}

void CatView::on_cbAutoLogin_clicked(bool checked)
{
    Options opts = ctl_->getOptions();
    opts.autoLogin = checked;
    ctl_->setOptions(opts);

    if ( ! checked && ! isLoggedIn_)
    {
        stopReconnect();
    }
}

void CatView::on_cbForcedLogin_clicked(bool checked)
{
    Options opts = ctl_->getOptions();
    opts.forcedLogin = checked;
    ctl_->setOptions(opts);
}

void CatView::on_cbAutorelogin_clicked(bool checked)
{
    Options opts = ctl_->getOptions();
    opts.autoReconnect = checked;
    ctl_->setOptions(opts);

    if ( ! checked && ! isLoggedIn_)
    {
        stopReconnect();
    }
}

void CatView::on_cbTropophobia_clicked(bool checked)
{
    Options opts = ctl_->getOptions();
    opts.tropophobia = checked;
    ctl_->setOptions(opts);
}

// void CatView::on_cbWandering_clicked(bool /*checked*/)
// {
//     Options opts;
// 
//     ctl_->getOptions(&opts);
//     opts.wandering = checked;
//     ctl_->setOptions(&opts);
// }

void CatView::on_cbAutoMarket_clicked(bool checked)
{
    Options opts = ctl_->getOptions();
    opts.autoShop = checked;
    ctl_->setOptions(opts);
}

void CatView::on_btnLogin_clicked()
{
    ui->statusBar->clearMessage();

    if (currentAccount_.accountIndex < 0)
    {
        if (accounts_.count() == 0)
        {
            logMessage(tr("Please add account to log in"));
        }
        else
        {
            logMessage(tr("Please select account/character to log in"));
        }

        return;
    }

    logMessage(tr("Connecting..."));

    bool result = ctl_->connect(currentAccount_.accountIndex, currentAccount_.characterIndex);
    if ( ! result)
    {
        logError(tr("Connection Failed"));
        return;
    }

    ui->btnLogin->setEnabled(false);
}

void CatView::on_btnLogout_clicked()
{
    logOut();
}

void CatView::on_btnShowClientWindow_clicked()
{
    ui->btnShowClientWindow->isChecked() ? ctl_->showClientWindow() : ctl_->hideClientWindow();
}

void CatView::on_btnOpenMarket_clicked()
{
    if (tradeView_)
    {
        tradeView_->activateWindow();
    }
    else
    {
        ctl_->openMarket();
        // core does items validation and updates item set
    }
}

void CatView::on_btnCloseMarket_clicked()
{
    ctl_->closeMarket();
}

void CatView::on_btnMarketSetup_clicked()
{
    configView_ = new ConfigView(QList< QSharedPointer<CatCtl> >() << ctl_, &ctl_->config(), this);
    configView_->setAttribute(Qt::WA_DeleteOnClose, true);

    connect(ctl_.data(),    &CatCtl::marketSetupUpdated
           , configView_,   &ConfigView::updateMarketSetup);
    connect( configView_,   &ConfigView::globalConfig
           , this,          &CatView::globalSettings);
    connect( configView_,   &ConfigView::finished
           , this,          &CatView::updateOptions);

    configView_->show();
}

void CatView::on_btnHistory_clicked()
{
    VMProtectBeginVirtualization(__FUNCTION__);
    if (!Vmp::hasOption(Vmp::OptionHistory))
    {
        return;
    }

    if (!historyView_)
    {
        historyView_ = new HistoryView(ctl_, this);
        historyView_->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
        historyView_->show();
    }
    else
    {
        historyView_->activateWindow();
    }

    VMProtectEnd();
}

void CatView::on_btnCommi_clicked()
{
    if (!commiView_)
    {
        commiView_ = new CommiView(ctl_, this);
        commiView_->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
        commiView_->show();
    }
    else
    {
        commiView_->activateWindow();
    }
}

void CatView::on_teLog_anchorClicked(const QUrl & link)
{
    if (link.scheme() != "action")
    {
        return;
    }

    QStringList args = link.path().split(':', QString::SkipEmptyParts);
    if (args.isEmpty())
    {
        return;
    }

    if (args.at(0) == "revive")
    {
        ctl_->revive();
    }
    else if (args.at(0) == "acceptTrade"
            && args.count() > 1)
    {
        unsigned tradeId = args.at(1).toUInt(0, 16);
        removeAnchors(tr("Accept"), QString("action:acceptTrade:") + args.at(1));
        removeAnchors(tr("Decline"), QString("action:rejectTrade:") + args.at(1));
        ctl_->acceptTrade(tradeId);
    }
    else if (args.at(0) == "rejectTrade"
            && args.count() > 1)
    {
        unsigned tradeId = args.at(1).toUInt(0, 16);
        removeAnchors(tr("Accept"), QString("action:acceptTrade:") + args.at(1));
        removeAnchors(tr("Decline"), QString("action:rejectTrade:") + args.at(1));
        ctl_->rejectTrade(tradeId);
    }
}

void CatView::displaySetLockTime()
{
    if (isLoggedIn_)
    {
        LockView *lv = new LockView(ctl_.data(), this);
        connect(lv, &LockView::logMessage, this, &CatView::logMessage);
        lv->show();
    }
}

void CatView::updateOptions()
{
    const Options & opts = ctl_->getOptions();

    ui->teLog->document()->setMaximumBlockCount(opts.maxLogLines);

    saveLog_    = opts.saveLogToFile;
    rotateLog_  = opts.splitLogFilesPerDay;
    logPath_    = QString::fromStdWString(opts.logPath);

    chat_->setBufferSize(opts.chatBufferSize);
}

void CatView::updateLicenseState(VMProtectSerialStateFlags /*state*/)
{
    Log("Updating help icon");

    QDateTime now = QDateTime::currentDateTime();
    Vmp::SerialNumberData serial;
    if (Vmp::getSerialData(serial)
        && serial.expiry.isValid())
    {
        if (now.date() > serial.expiry)
        {
            lbHelp_->setToolTip(tr("Your license is expired"));
            lbHelp_->setText("<a href=\"urn:about\"><img src=\":/catView/exclamation.png\"/></a>");
            return;
        }
        else if (now.daysTo(QDateTime(serial.expiry)) <= 2)
        {
            lbHelp_->setToolTip(tr("Your license will expire soon"));
            lbHelp_->setText("<a href=\"urn:about\"><img src=\":/gconfig/error.png\"/></a>");
            return;
        }
    }

    lbHelp_->setToolTip(tr("About and Registration"));
    lbHelp_->setText("<a href=\"urn:about\"><img src=\":/catView/Button-Help-icon.png\"/></a>");
}

void CatView::updateProfit()
{
    if (lbStatusProfit_)
    {
        QString text = tr("<img src=\":/catView/table_money_16.png\"/> %1").arg(spacedNumber(ctl_->getProfit()));
        lbStatusProfit_->setText(text);
    }
}

//

void CatView::onAuthProgress(int value, int maxValue)
{
    qDebug() << "progress:" << value;
    ui->pbAuth->setMaximum(maxValue);
    ui->pbAuth->setValue(value);
}

void CatView::onAuthGetPin()
{
    if (!getPin_)
    {
        QString email, passworf;
        ctl_->getAuth(email, passworf);
        getPin_ = new GetPinView(email, ctl_->getPcName(), this);
        getPin_->setAttribute(Qt::WA_DeleteOnClose, true);

        QObject::connect(getPin_.data(), &QDialog::accepted, this, &CatView::pinAccepted);
        QObject::connect(getPin_.data(), &QDialog::rejected, this, &CatView::pinRejected);
    }
    getPin_->show();
}

void CatView::onConnected()
{
    logMessage(tr("Connected"));
    onAuthProgress(0, 1);
    setConnectedState(true);
}

void CatView::onLoggedIn()
{
    setLoggedInState(true);
    updateProfit();
}

void CatView::onDisconnected()
{
    if (tradeView_)
    {
        tradeView_->close();
    }
    if (charselView_)
    {
        charselView_->close();
    }
    if (commiView_)
    {
        commiView_->close();
    }
    logMessage(tr("Disconnected"));
    onAuthProgress(0, 1);
    setLoggedInState(false);
    setConnectedState(false);
}

void CatView::onCompleted()
{
    logMessage(tr("Task completed"));
}

void CatView::onClientStarted()
{
    ui->swLoginClient->setCurrentIndex(PageClientOperation);
    ui->btnShowClientWindow->setChecked(false);
}

void CatView::onClientStopped()
{
    ui->swLoginClient->setCurrentIndex(PageLoginProgress);
}

void CatView::onClientWindowClosed()
{
    ui->btnShowClientWindow->setChecked(false);
}

void CatView::onEvent(CatCtl::Event event)
{
    switch (event)
    {
        case CatCtl::CharselectRequest:
            startCharselect();
            break;

        case CatCtl::CharselectAuto:
        {
            accounts_ = ctl_->getAccounts(currentAccount_);
            logMessage(tr("Autoselecting character %1")
                .arg(getCurrentName().toHtmlEscaped()));
            updateAccountsCb(false);
            break;
        }

        case CatCtl::CharselectFailed:
            logError(tr("Failed to select character"));
            logOut();
            break;

        case CatCtl::CharselectNoChars:
            logMessage(tr("No characters on this account. Please create any character first or make sure you selected your server in accounts"));
            // allow user to create character in client
        #if !defined(GAME_USING_CLIENT)
            logOut();
        #endif
            break;

        case CatCtl::MarketReopening:
            logMessage(tr("Scheduling market reopening"));
            break;

        case CatCtl::MarketMoving:
            logMessage(tr("Moving market"));
            break;

        case CatCtl::ImmobilizerWarned:
            logMessage(tr("Immobilizer alert"));
            ui->cbAutorelogin->setChecked(false);
            on_cbAutorelogin_clicked(false);
            break;

        default:
            logMessage(tr("Unknown event %1").arg(int(event)));
            break;
    }
}

void CatView::onGameEvent(const qlib::GameEvent & ev)
{
    BaseEvent *event = ev.data();

    switch (ev->type())
    {
        case BaseEvent::CharselectCompleted:
        {
        #if CLIENT_VERSION > 1420   // might be greater
            DWORD lastLoginTs, lastIp, currentIp;
            ctl_->getIpInfo(lastLoginTs, lastIp, currentIp);
            logMessage(tr("Last login  %1  from %2.%3.%4.%5")
                .arg(QDateTime::fromTime_t(lastLoginTs).toString(tr("dd.MM.yyyy hh:mm:ss")))
                .arg((lastIp & 0x000000FF) >> 0)
                .arg((lastIp & 0x0000FF00) >> 8)
                .arg((lastIp & 0x00FF0000) >> 16)
                .arg((lastIp & 0xFF000000) >> 24)
                );
            logMessage(tr("IP: %2.%3.%4.%5")
                .arg((currentIp & 0x000000FF) >> 0)
                .arg((currentIp & 0x0000FF00) >> 8)
                .arg((currentIp & 0x00FF0000) >> 16)
                .arg((currentIp & 0xFF000000) >> 24)
                );
        #endif
            logMessage(tr("Entering in game"));
            break;
        }

        case BaseEvent::SetLockTime:
        {
            const SetLockTimeEvent *e = static_cast<const SetLockTimeEvent *>(event);
            if (e->isOk())
            {
                static const unsigned secsInMin = 60;
                static const unsigned secsInHour = 60 * secsInMin;
                static const unsigned secsInDay = 24 * secsInHour;

                unsigned lockTime = e->seconds();
                unsigned days = lockTime / secsInDay;
                unsigned hours = (lockTime - days * secsInDay) / secsInHour;
                unsigned mins = (lockTime - days * secsInDay - hours * secsInHour) / secsInMin;
                unsigned secs = lockTime - days * secsInDay - hours * secsInHour - mins * secsInMin;

                logMessage(tr("Lock is set to %1 day(s) %2:%3:%4.")
                                .arg(days)
                                .arg(hours, 2, 10, QChar('0'))
                                .arg(mins, 2, 10, QChar('0'))
                                .arg(secs, 2, 10, QChar('0'))
                                );
            }
            else
            {
                logError(tr("Failed to set lock time"));
            }
            break;
        }

        case BaseEvent::PlayerMarketState:
        {
            PlayerMarketStateEvent *e = static_cast<PlayerMarketStateEvent*>(event);
            if (e->isOpened())
            {
                logMessage(tr("Market opened"));
                setShopState(true);
                updateCurrentMarketItems();
            }
            else
            {
                logMessage(tr("Market closed"));
                setShopState(false);
            }
            break;
        }

        case BaseEvent::InventoryUpdate:
        {
            InventoryUpdateEvent *e = static_cast<InventoryUpdateEvent*>(event);
            if ((e->storageType() & MONEY_INFO) != 0)
            {
                setMoney(ctl_->getMoney(), ctl_->getBanknotes());
            }
            if ((e->storageType() & GOLD_INFO) != 0)
            {
                setGold(ctl_->getGold());
            }
            
            break;
        }

        case BaseEvent::ObtainItem:
        {
            ObtainItemEvent *e = static_cast<ObtainItemEvent*>(event);
            logMessage(tr("^C0C0C0%1 x %2 obtained/^")
                .arg(e->count())
                .arg(ctl_->getItemName(e->itemId())));
            break;
        }

        case BaseEvent::DropItem:
        {
            DropItemEvent *e = static_cast<DropItemEvent*>(event);
            logMessage(tr("^C0C0C0%1 x %2 lost/^")
                .arg(e->count())
                .arg(ctl_->getItemName(e->itemId())));
            break;
        }

        case BaseEvent::MarketItemPurchased:
        {
            ItemPurchasedEvent *e = static_cast<ItemPurchasedEvent*>(event);
            if (ctl_->isMarketOpened())
            {
                logMessage(tr("%1 x ^FFC0C0%2/^ %3 purchased")
                    .arg(e->count())
                    .arg(spacedNumber(e->price()))
                    .arg(ctl_->getItemName(e->itemId())));
                updateCurrentMarketItems();
            }
            else
            {
                logMessage(tr("%1 %2 purchased")
                           .arg(e->count())
                           .arg(ctl_->getItemName(e->itemId())));
            }

            updateProfit();
            break;
        }

        case BaseEvent::MarketItemSold:
        {
            ItemSoldEvent *e = static_cast<ItemSoldEvent*>(event);
            if (ctl_->isMarketOpened())
            {
                logMessage(tr("%1 x ^C0C0FF%2/^ %3 sold to %4")
                            .arg(e->count())
                            .arg(spacedNumber(e->price()))
                            .arg(ctl_->getItemName(e->itemId()))
                            .arg(getPlayerNameString(e->buyerId())));
                updateCurrentMarketItems();
            }
            else
            {
                logMessage(tr("%1 %2 sold to %3")
                           .arg(e->count())
                           .arg(ctl_->getItemName(e->itemId()))
                           .arg(getPlayerNameString(e->buyerId())));
            }

            updateProfit();
            break;
        }

        case BaseEvent::PlayerDamaged:
        {
            PlayerDamagedEvent *e = static_cast<PlayerDamagedEvent*>(event);
            logMessage(tr("^C0C0C0%1 deals you %2 of damage/^")
                .arg(getPlayerNameString(e->attacker()).toHtmlEscaped())
                .arg(e->damage()));
            break;
        }

        case BaseEvent::PlayerKilled:
        {
            PlayerKilledEvent *e = static_cast<PlayerKilledEvent*>(event);
            DWORD attacker = e->attacker();
            if (attacker != (DWORD)-1)
            {
                logMessage(tr("^FFC0C0You are killed by %1/^")
                    .arg(getPlayerNameString(attacker).toHtmlEscaped()));
            }
            else
            {
                logMessage(tr("^FFC0C0You are dead/^"));
            }
            isDead_ = true;

            ui->btnOpenMarket->setEnabled(false);
            emit stateUpdated();
            break;
        }

        case BaseEvent::PlayerRevived:
        {
            PlayerRevivedEvent *e = static_cast<PlayerRevivedEvent*>(event);
            if (e->stage() == 1)
            {
                logMessage(tr("Resurrecting..."));
            }
            else if (e->stage() == 2)
            {
                logMessage(tr("Resurrected"));
                ui->btnOpenMarket->setEnabled(true);
                isDead_ = false;
                emit stateUpdated();
            }
            else
            {
                assert(0);
            }

            removeAnchors(tr("Revive"));

            break;
        }

        case BaseEvent::PlayerExp:
        {
            PlayerExpEvent *e = static_cast<PlayerExpEvent*>(event);
            if (e->exp() != 0)
            {
                logMessage(tr("^80FFFFReceived %1 exp/^").arg(e->exp()));
            }
            if (e->soul() != 0)
            {
                logMessage(tr("^80FFFFReceived %1 soul/^").arg(e->soul()));
            }
            break;
        }

        case BaseEvent::PlayerLevelup:
        {
            logMessage(tr("^80FFFFReached level %1/^").arg(ctl_->myLevel()));
            break;
        }

        case BaseEvent::PlayerEnableResurrect:
        {
            // need trailing space
            logMessage(tr("Revival is enabled. <a href=\"action:revive\" style=\"color:#33FFFF\">Revive</a>?"));
            break;
        }

        case BaseEvent::TradeRequest:
        {
            TradeRequestEvent *e = static_cast<TradeRequestEvent*>(event);

            logMessage(tr("%1 requests you to trade. "
                "<a href=\"action:acceptTrade:%2\" style=\"color:#33FFFF\">Accept</a>"
                "/<a href=\"action:rejectTrade:%2\" style=\"color:#33FFFF\">Decline</a>?")
                .arg(getPlayerNameString(e->playerId()))
                .arg(QString::number(e->tradeId(), 16).toUpper(), 8, '0'));
            break;
        }

        case BaseEvent::TradeStart:
        {
            TradeStartEvent *e = static_cast<TradeStartEvent*>(event);
            if (e->success())
            {
                QString pName = getPlayerNameString(e->playerId());
                logMessage(tr("Started trade with %1").arg(pName.toHtmlEscaped()));
                openTrade(pName);
            }
            else
            {
                logMessage(tr("Trade canceled"));
            }
            break;
        }

        case BaseEvent::TradeEnd:
        {
            TradeEndEvent *e = static_cast<TradeEndEvent*>(event);
            if (e->success())
            {
                logMessage(tr("Trade completed"));
            }
            else
            {
                logMessage(tr("Trade failed"));
            }
            break;
        }

        case BaseEvent::PlayerResolved:
        {
            PlayerResolvedEvent *e = static_cast<PlayerResolvedEvent*>(event);
            playerResolved(e->gId(), QString::fromStdWString(e->name()));
            break;
        }

        case BaseEvent::PlayerPositionNotify:
        {
            PlayerPositionNotifyEvent *e = static_cast<PlayerPositionNotifyEvent*>(event);
            logMessage(tr("Position: %1 %2 %3")
                .arg(e->position().x(), 0, 'f', 2)
                .arg(e->position().y(), 0, 'f', 2)
                .arg(e->position().z(), 0, 'f', 2));
            break;
        }

        case BaseEvent::GameError:
        {
            GameErrorEvent *e = static_cast<GameErrorEvent*>(event);
            logMessage(tr("^FFD0D0%1./^")
                .arg(QString::fromStdWString(e->message())));
            break;
        }

        case BaseEvent::ServerError:
        {
            ServerErrorEvent *e = static_cast<ServerErrorEvent*>(event);
            logMessage(tr("^FFD0D0%1 %2/^")
                .arg(QString::fromStdWString(e->baseMessage()))
                .arg(QString::fromStdWString(e->message())));
            break;
        }

        case BaseEvent::BanMessage:
        {
            BanMessageEvent *e = static_cast<BanMessageEvent*>(event);
            logMessage(QString::fromStdWString(e->message()));

            QDateTime banTime(QDateTime::fromTime_t(e->banTs()));
            int secsRemaining = e->secsRemaining();

            QString message = tr("^FFD0D0Account blocked at %1. Time to unblock:")
                .arg(banTime.toString(Qt::RFC2822Date));

            int daysLeft = secsRemaining / 86400;
            if (daysLeft > 0)   message += tr(" %1 day(s)").arg(daysLeft);
            int hoursLeft = (secsRemaining - (daysLeft * 86400)) / 3600;
            if (hoursLeft > 0)  message += tr(" %1 hour(s)").arg(hoursLeft);
            int minsLeft = (secsRemaining - (daysLeft * 86400) - (hoursLeft * 3600)) / 60;
            if (minsLeft > 0)   message += tr(" %1 min(s)").arg(minsLeft);
            message += "/^";

            logMessage(message);
            break;
        }

        default:
            break;
    }
}

void CatView::onReconnectEvent(int remainingSeconds)
{
    if (remainingSeconds >= 0)
    {
        // timer is running
        ui->statusBar->showMessage(tr("Connecting in: %1").arg(remainingSeconds));
        if (timerState_ != true)
        {
            timerState_ = true;
            emit stateUpdated();
        }
    }
    else
    {
        // timer is over
        ui->statusBar->clearMessage();

        if (timerState_ != false)
        {
            timerState_ = false;
            emit stateUpdated();
        }

        if (remainingSeconds < 0)
        {
            on_btnLogin_clicked();
        }
    }
}

void CatView::onLockInfo(bool status, unsigned long long remainingSeconds)
{
    setLockState(status, remainingSeconds);
}

void CatView::charSelectDone(int r)
{
    if (r == QDialog::Accepted)
    {
        currentAccount_.characterIndex = charselView_->getSelected();
        if (currentAccount_.characterIndex < 0)
        {
            currentAccount_.characterIndex = 0;
        }
        updateAccountsCb(false);

        logMessage(tr("Selecting character %1")
                   .arg(getCurrentName().toHtmlEscaped()));

        ctl_->selectChar(currentAccount_.accountIndex, currentAccount_.characterIndex);
    }
    else
    {
        ctl_->disconnect(false);
    }
}

void CatView::pinAccepted()
{
    auto pin = getPin_->pin();
    if (!pin.isEmpty())
    {
        ctl_->setPin(pin);
    }
    else
    {
        ctl_->disconnect(false);
        onDisconnected();
    }
}

void CatView::pinRejected()
{
    ctl_->disconnect(false);
    onDisconnected();
}

//

void CatView::displayAbout()
{
    AboutView *about = new AboutView(this);
    about->setAttribute(Qt::WA_DeleteOnClose);
    about->show();
}

void CatView::copyToCb(QWidget *sender)
{
    QLabel *o = qobject_cast<QLabel *>(sender);
    if (o != NULL)
    {
        QClipboard *cb = QApplication::clipboard();
        cb->setText(o->text().remove(QRegularExpression("<img.*\\/>")).trimmed());
    }
}

//

void CatView::updateAccountsCb(bool keepSelection)
{
    QString oldName = ui->cbAccounts->currentText();

    ui->cbAccounts->blockSignals(true);
    ui->cbAccounts->clear();

    for (int iAcc = 0; iAcc < accounts_.size(); iAcc++)
    {
        const CatCtl::Account & acc = accounts_[iAcc];

        ui->cbAccounts->addItem( QIcon(":/catView/group.png"), acc.name
                               , QVariant::fromValue(CatCtl::AccountSelection(iAcc, -1)));
        for (int iCh = 0; iCh < acc.characters.size(); iCh++)
        {
            ui->cbAccounts->addItem( QString::fromWCharArray(L"\x2001\x2001\x2001") + acc.characters[iCh]
                                   , QVariant::fromValue(CatCtl::AccountSelection(iAcc, iCh)));
        }
    }

    ui->cbAccounts->setCurrentIndex(-1);
    ui->cbAccounts->blockSignals(false);

    if (keepSelection && ! oldName.isEmpty())
    {
        int previous = ui->cbAccounts->findText(oldName);
        if (previous >= 0)
        {
            currentAccount_ = ui->cbAccounts->itemData(previous).value<CatCtl::AccountSelection>();
        }
        else
        {
            // current was removed
            currentAccount_.accountIndex = 0;
            currentAccount_.characterIndex = -1;
        }
    }

    // set selection to current account

    int idx = accounts_.count() > 0 ? 0 : -1;   // we set first if selected doesn't match any acc
    for (int i = 0; i < ui->cbAccounts->count(); i++)
    {
        CatCtl::AccountSelection item = ui->cbAccounts->itemData(i).value<CatCtl::AccountSelection>();
        if (item == currentAccount_)
        {
            idx = i;
            break;
        }
    }

    ui->cbAccounts->setCurrentIndex(idx);
}

void CatView::startCharselect()
{
    accounts_ = ctl_->getAccounts(currentAccount_);

    if (currentAccount_.accountIndex < 0 
        || currentAccount_.accountIndex >= accounts_.count())
    {
        assert(0);
        logMessage(tr("Invalid account info or no accounts configured"));
        ctl_->disconnect(false);
        return;
    }

    if (charselView_)
    {
        delete charselView_;
    }

    charselView_ = new CharselView(accounts_[currentAccount_.accountIndex], this);
    connect(charselView_.data(), &QDialog::finished, this, &CatView::charSelectDone);
    charselView_->setModal(true);
    charselView_->show();
}

void CatView::logOut()
{
    ui->btnLogout->setEnabled(false);
    if (tradeView_)
    {
        tradeView_->close();
    }
    ctl_->disconnect();
}

void CatView::logError(const QString & reason)
{
    logMessage(tr("%1.").arg(reason));
}

void CatView::setConnectedState(bool connected)
{
    if (connected)
    {
        ui->btnLogin->setEnabled(false);
        ui->btnLogout->setEnabled(true);
        // accounts
        ui->btnAccountsSetup->setEnabled(false);
        ui->cbAccounts->setEnabled(false);
        // status
        lbStatusLock_->setVisible(false);
        lbStatusLockImage_->setPixmap(QPixmap());
        lbStatusMarket_->setPixmap(QPixmap());

        lbStatusMoney_->setText(QString());
        lbStatusBuying_->setText(QString());
        lbStatusSelling_->setText(QString());
        lbStatusGold_->setText(QString());

        lbStatusMoney_->show();
        lbStatusBuying_->show();
        lbStatusSelling_->show();
        lbStatusGold_->show();

        if (lbStatusProfit_)
        {
            lbStatusProfit_->setText(QString());
            lbStatusProfit_->show();
        }

        isMarketOpened_ = false;
        timerState_ = false;
    }
    else
    {
        removeAnchors(tr("Revive"));

        ui->btnLogin->setEnabled(true);
        ui->btnLogout->setEnabled(false);
        // accounts
        ui->btnAccountsSetup->setEnabled(true);
        ui->cbAccounts->setEnabled(true);
        // status
        lbStatusLock_->setVisible(false);
        lbStatusLockImage_->setPixmap(QPixmap());
        lbStatusMarket_->setPixmap(QPixmap());

        lbStatusMoney_->hide();
        lbStatusBuying_->hide();
        lbStatusSelling_->hide();
        lbStatusGold_->hide();
        if (lbStatusProfit_)
        {
            lbStatusProfit_->hide();
        }

        ui->lbSellingItems->setText(QString());
        ui->lbBuyingItems->setText(QString());
        ui->tabWidget->setTabText(tabMarket, tr("Market"));

        lockState_ = -1;
        isMarketOpened_ = false;
        isDead_ = false;
        timerState_ = false;
    }

    isLoggedIn_ = connected;
    emit stateUpdated();
}

void CatView::setLoggedInState(bool loggedIn)
{
    if (loggedIn)
    {
        ui->btnOpenMarket->setEnabled(true);
        ui->btnCloseMarket->setEnabled(true);
        ui->btnCommi->setEnabled(true);
        // history
        ui->btnHistory->setEnabled(true);
        // chat
        chat_->setEnabled(true);
    }
    else
    {
        ui->btnOpenMarket->setEnabled(false);
        ui->btnCloseMarket->setEnabled(false);
        ui->btnCommi->setEnabled(false);
        // history
        ui->btnHistory->setEnabled(false);
        // chat
        chat_->setEnabled(false);
    }
}

void CatView::setLockState(bool isLockOn, unsigned long long remainingSeconds)
{
    if (isLockOn)
    {
        // lock enabled
        if (lockState_ != 1)
        {
            // was disabled
            lockState_ = 1;

            ui->btnCloseMarket->setEnabled(false);
            lbStatusLock_->setVisible(true);
            lbStatusLockImage_->setPixmap(QPixmap(":/catView/lock_16.png"));

            logMessage(tr("Lock is on for %1 seconds").arg(remainingSeconds / 1000));
            emit stateUpdated();
        }

        lbStatusLock_->setText(tr("%1").arg(remainingSeconds / 1000));
    }
    else
    {
        // lock disabled
        if (lockState_ != 0)
        {
            // was enabled
            lockState_ = 0;

            ui->btnCloseMarket->setEnabled(false);
            ui->btnOpenMarket->setEnabled(isDead_ ? false : true);
            lbStatusLock_->setText(QString());
            lbStatusLock_->setVisible(false);
            lbStatusLockImage_->setPixmap(QPixmap(":/catView/lock-open-icon.png"));

            logMessage(tr("Lock expired"));

            const Options opts = ctl_->getOptions();
            if (opts.autoShop)
            {
                on_btnOpenMarket_clicked();
            }

            emit stateUpdated();
        }
    }
}

void CatView::setShopState(bool isOpen)
{       
    if (isOpen)
    {
        // shop opened
        ui->btnOpenMarket->setEnabled(false);
        ui->btnCloseMarket->setEnabled(true);

        lbStatusMarket_->setPixmap(QPixmap(":/catView/cat.png"));
    }
    else
    {
        //shop closed
        ui->btnOpenMarket->setEnabled(true);
        ui->btnCloseMarket->setEnabled(false);

        lbStatusMarket_->setPixmap(QPixmap());
        lbStatusSelling_->setText(QString());
        lbStatusBuying_->setText(QString());

        ui->lbSellingItems->setText(QString());
        ui->lbBuyingItems->setText(QString());
        ui->tabWidget->setTabText(tabMarket, tr("Market"));
    }

    isMarketOpened_ = isOpen;
    emit stateUpdated();
}

void CatView::setMoney(unsigned money, unsigned banknotes)
{
    QString text;
    if (banknotes > 0)
    {
        text += tr("<img src=\":/catView/money.png\"/> %1 &nbsp; ").arg(banknotes);
    }
    text += tr("<img src=\":/catView/coins.png\"/> %1").arg(spacedNumber(money));

    lbStatusMoney_->setText(text);
    moneyUpdate(money, banknotes);
}

void CatView::setGold(unsigned gold)
{
    QString text;
    if (gold > 0)
    {
        text += tr("<img src=\":/catView/coin_gold.png\"/> %1.%2").arg(gold / 100).arg(gold % 100, 2, 10, QChar('0'));
    }

    lbStatusGold_->setText(text);
    goldUpdate(gold);
}

QString CatView::getCurrentName() const
{
    if (currentAccount_.accountIndex >= 0
        && currentAccount_.accountIndex < accounts_.count())
    {
        const CatCtl::Account & acc = accounts_[currentAccount_.accountIndex];
        if (currentAccount_.characterIndex >= 0
            && currentAccount_.characterIndex < acc.characters.count())
        {
            return acc.characters[currentAccount_.characterIndex];
        }
        else
        {
            return QString("[%1]").arg(acc.name);
        }
    }

    return QString();
}

void CatView::removeAnchors(const QString & text, const QString & anchor)
{
    QTextCursor cursor = ui->teLog->textCursor();
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);

    while( ! (cursor = ui->teLog->document()->find(text, cursor.selectionEnd(), 0)).isNull())
    {
        QTextCharFormat fmt = cursor.charFormat();
        if (fmt.isAnchor()
            && (anchor.isNull() 
                || ( ! anchor.isNull() && anchor == fmt.anchorHref())))
        {
            fmt.setAnchorHref(QString());
            fmt.setAnchor(false);
            fmt.setUnderlineStyle(QTextCharFormat::NoUnderline);
            fmt.setUnderlineColor(QColor("yellow"));
            fmt.setForeground(ui->teLog->palette().text());
            cursor.setCharFormat(fmt);
        }
    }
}

void CatView::stopReconnect()
{
    // cancel reconnect
    ui->statusBar->clearMessage();
    ctl_->disconnect();
    timerState_ = false;
    emit stateUpdated();
}

void CatView::updateText()
{
    copyAction1_->setText(tr("Copy to clipboard"));
    copyAction2_->setText(tr("Copy to clipboard"));
    copyAction3_->setText(tr("Copy to clipboard"));

    lbStatusMoney_->setToolTip(tr("Money"));
    lbStatusSelling_->setToolTip(tr("Selling"));
    lbStatusBuying_->setToolTip(tr("Buying"));
    lbStatusGold_->setToolTip(tr("Gold"));
    if (lbStatusProfit_)
    {
        lbStatusProfit_->setToolTip(tr("Profit"));
    }

    lbStatusMarket_->setToolTip(tr("Market status"));
}

void CatView::playerResolved(unsigned playerId, const QString & name)
{
    QString hex = QString("%1").arg(QString::number(playerId, 16).toUpper(), 8, '0');

    QDateTime now = QDateTime::currentDateTime();
    writeLog(now, hex + ": " + name);

    QTextCursor cursor = ui->teLog->textCursor();
    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    while( ! (cursor = ui->teLog->document()->find(hex, cursor.selectionEnd(), 0)).isNull())
    {
        QTextCharFormat fmt = cursor.charFormat();
        if (fmt.toolTip() == "char_id")
        {
            fmt.setToolTip(QString());
            cursor.setCharFormat(fmt);
            cursor.insertText(name);
        }
    }
}

void CatView::writeLog(const QDateTime & now, const QString & msg)
{
    if (saveLog_)
    {
        QString logName = currentServer_ >= 0 ? QString::number(currentServer_) : "#";

        QString ch = getCurrentName();
        if ( ! ch.isEmpty() )
        {
            logName += '-' + ch;
        }
        else
        {
            logName += "-#";
        }

        if (rotateLog_)
        {
            logName += now.toString("-MM-dd");
        }

        logName += ".log";

        // cleanup filename
        logName = sanitizeFilename(logName);

        QDir logDir(logPath_);
        if (logDir.exists())
        {
            logName = logDir.absoluteFilePath(logName);
        }

        QFile logFile(logName);
        if (logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        {
            QString stripped = msg;
            stripped.replace(QRegExp("\\^([0-9A-F]{6})(.+)(?=(/\\^))(/\\^)"), "\\2");
            stripped.remove(QRegExp("</?(?:span|a href)[^>]*>"));
            QString time = now.toString("MM-dd hh:mm:ss");
            QString plaintext = QString("[%1] %2\n").arg(time).arg(stripped);
            logFile.write(plaintext.toUtf8());
        }
    }
}

QString CatView::getPlayerNameString(DWORD id)
{
    if ((id & 0x80000000) == 0)   // isPlayer
    {
        QString name = ctl_->resolvePlayerName(id);
        if ( ! name.isNull())
        {
            return name;
        }
        else
        {
            return QString("<span title=\"char_id\">%1</span>")
                .arg(QString::number(id, 16).toUpper(), 8, '0');
        }
    }
    else
    {
        return QString("%1").arg(QString::number(id, 16).toUpper(), 8, '0');
    }
}

void CatView::openTrade(const QString & playerName)
{
    if (tradeView_)
    {
        delete tradeView_;
    }

    tradeView_ = new TradeView(ctl_.data(), playerName, this);
    tradeView_->show();
}

void CatView::updateCurrentMarketItems()
{
    QString marketName = ctl_->getMarketName();
    ui->tabWidget->setTabText(tabMarket, tr("Market [%1]").arg(marketName));

    ui->lbSellingItems->clear();
    ui->lbBuyingItems->clear();

    QList<MarketItemSetup> items = ctl_->getCurrentShopItems();

    if (items.count() == 0)
    {
        logMessage(tr("No more items to trade"));
        return;
    }

    long long sellSum = 0, buySum = 0;
    int sellCount = 0, buyCount = 0;
    QString sellText, buyText;

    for (int i = 0; i < items.count(); i++)
    {
        const MarketItemSetup & itemSetup = items.at(i);

        if (itemSetup.doSell)
        {
            sellCount ++;
            sellSum += long long(itemSetup.sellPrice) * itemSetup.sellCount;

            sellText += QString("%1 x <font color=\"blue\">%2</font>  %3<br>")
                .arg(itemSetup.sellCount)
                .arg(spacedNumber(itemSetup.sellPrice))
                .arg(QString::fromStdWString(itemSetup.item.name));
        }
        else if (itemSetup.doBuy)
        {
            buyCount ++;
            buySum += long long(itemSetup.buyPrice) * itemSetup.buyCount;

            buyText += QString("%1 x <font color=\"darkred\">%2</font>  %3<br>")
                .arg(itemSetup.buyCount)
                .arg(spacedNumber(itemSetup.buyPrice))
                .arg(QString::fromStdWString(itemSetup.item.name));
        }
    }

    ui->lbSellingItems->setText(sellText);
    ui->lbBuyingItems->setText(buyText);
    lbStatusSelling_->setText(sellCount > 0 ? QString("<img src=\":/catView/coins_add.png\"/>  %1 [%2]").arg(spacedNumber(sellSum)).arg(sellCount) : QString());
    lbStatusBuying_->setText(buyCount > 0 ? QString("<img src=\":/catView/coins_delete.png\"/>  %1 [%2]").arg(spacedNumber(buySum)).arg(buyCount) : QString());
}
