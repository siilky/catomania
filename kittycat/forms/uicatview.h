#ifndef uicatview_h
#define uicatview_h

#include <QScopedPointer>
#include <QSystemTrayIcon>
#include <QLabel>

#include "catctl.h"
#include "qlib/vmp.h"

class Chat;
class TradeView;
class ConfigView;
class CharselView;
class HistoryView;
class CommiView;
class GetPinView;

namespace Ui { class CatView; }

class CatView : public QWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(CatView)

public:
    enum CatState
    {
        Offline,
        Online,
        Locked,
        MarketOpened,
        Dead,
        Waiting,
    };

    CatView(QSharedPointer<CatCtl> ctl, QWidget *parent = 0);
    ~CatView();

    void setClientExePath(const QString & exePath);

    QString currentAccount() const;
    CatState state() const;

signals:
    void moneyUpdate(unsigned money, unsigned banknotes);
    void goldUpdate(unsigned gold);
    void accountUpdate(const QString & current);
    void stateUpdated();
    void globalSettings();

public slots:
    void logMessage(const QString & msg);
    void writeLog(const QDateTime &now, const QString & msg);

    void logError(const QString & reason);
    void disableAll();

protected:
    virtual void changeEvent(QEvent *event);

private slots:
    void on_cbAutoLogin_clicked(bool checked);
    void on_cbForcedLogin_clicked(bool checked);
    void on_cbAutorelogin_clicked(bool checked);
    void on_cbTropophobia_clicked(bool checked);
//     void on_cbWandering_clicked(bool checked);
    void on_cbAutoMarket_clicked(bool checked);
    void on_cbAccounts_currentIndexChanged(int index);
    void on_btnAccountsSetup_clicked();
    void on_btnLogin_clicked();
    void on_btnLogout_clicked();
    void on_btnShowClientWindow_clicked();
    void on_btnOpenMarket_clicked();
    void on_btnCloseMarket_clicked();
    void on_btnMarketSetup_clicked();
    void on_btnHistory_clicked();
    void on_btnCommi_clicked();
    void on_teLog_anchorClicked(const QUrl & link);
    void copyToCb(QWidget *);
    void displayAbout();
    void displaySetLockTime();
    void updateOptions();

    // Ctl
    void onAuthProgress(int value, int maxValue);
    void onAuthGetPin();
    void onConnected();
    void onLoggedIn();
    void onDisconnected();
    void onCompleted();
    void onClientStarted();
    void onClientStopped();
    void onClientWindowClosed();

    void onEvent(CatCtl::Event event);
    void onGameEvent(const qlib::GameEvent & event);

    void onReconnectEvent(int remainingSeconds);
    void onLockInfo(bool status, unsigned long long remainingSeconds);

    void updateLicenseState(VMProtectSerialStateFlags state);
    void updateProfit();

    void charSelectDone(int r);

    void pinAccepted();
    void pinRejected();

private:
    void updateAccountsCb(bool keepSelection = true);
    void startCharselect();
    void logOut();
    void setLockState(bool isLockOn, unsigned long long remainingSeconds);
    void setConnectedState(bool connected);
    void setLoggedInState(bool loggedIn);
    void setMoney(unsigned money, unsigned banknotes);
    void setGold(unsigned gold);
    void setShopState(bool isOpen);
    void stopReconnect();
    
    QString CatView::getCurrentName() const;
    void removeAnchors(const QString & text, const QString & anchor = QString());
    void updateText();
    void playerResolved(unsigned playerId, const QString & name);
    QString getPlayerNameString(DWORD id);    // also resolves and returns formatted string
    void openTrade(const QString & playerName);
    void updateCurrentMarketItems();

    //
    
    enum
    {
        tabHistory = 0,
        tabMarket = 1,
    };
    enum
    {
        PageLoginProgress   = 0,
        PageClientOperation = 1,
    };

    //

    QScopedPointer<Ui::CatView> ui;
    QLabel                      *lbHelp_;
    Chat                        *chat_;
    QPointer<ConfigView>        configView_;
    QPointer<TradeView>         tradeView_;
    QPointer<CharselView>       charselView_;
    QPointer<HistoryView>       historyView_;
    QPointer<CommiView>         commiView_;
    QPointer<GetPinView>        getPin_;

    QAction     *copyAction1_;
    QAction     *copyAction2_;
    QAction     *copyAction3_;
    QLabel      *lbStatusLock_;
    QLabel      *lbStatusLockImage_;
    QLabel      *lbStatusMarket_;
    QLabel      *lbStatusMoney_;
    QLabel      *lbStatusSelling_;
    QLabel      *lbStatusBuying_;
    QLabel      *lbStatusGold_;
    QLabel      *lbStatusProfit_;

    QSharedPointer<CatCtl>  ctl_;

    bool    isLoggedIn_;
    bool    timerState_;
    int     lockState_;     // -1 = unknown
    bool    isMarketOpened_;
    bool    isDead_;

    QList<CatCtl::Account>      accounts_;
    CatCtl::AccountSelection    currentAccount_;
    int                         currentServer_;

    bool    saveLog_;
    bool    rotateLog_;
    QString logPath_;

    QString clientExePath_;
};

#endif
