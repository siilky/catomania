#ifndef uiaccounts_h
#define uiaccounts_h

#include <QDialog>
#include <QScopedPointer>
#include <QAbstractButton>

#include "catctl.h"

namespace Ui
{
    class Accounts;
}
namespace PwMailRu
{
    class MailruAuth;
}
namespace ARC
{
    class ArcAuth;
}

class CatCtl;

class AccountsView : public QDialog
{
    Q_OBJECT
public:
    AccountsView(QSharedPointer<CatCtl> ctl, QWidget *parent = 0);
    ~AccountsView();

public slots:
    virtual void done(int r);

protected:
    virtual void changeEvent(QEvent *event);

private slots:
    void on_buttonBox_clicked(QAbstractButton * button);
    void on_leEmail_textChanged(const QString &);
    void on_lePassword_textChanged(const QString &);
    void on_cbServers_currentIndexChanged(int);

#if defined MAILRU_TOKEN_AUTH
    void onMailAuthProgress(int value, int maxValue);
    void onMailAuthFinished(bool success);
#endif
    void showError(const QString & str);
    void showSuccess(const QString & str);

private:
    void setAccounts(const QList<CatCtl::Account> & accounts);
    void updateButtonState();
    void setRunning(bool isRunning = true);
    bool save();        // returns true is dialog can be closed
#if defined(MAILRU_TOKEN_AUTH)
    void startAuth();
#endif

    QScopedPointer<Ui::Accounts> ui;

    QSharedPointer<CatCtl>          ctl_;
#if defined MAILRU_TOKEN_AUTH
    QPointer<PwMailRu::MailruAuth>  mailruAuth_;
#endif
    QList<CatCtl::Account>  accounts_;

    QString email_, password_;
    bool    needCloseAfterAuth_;
};

#endif
