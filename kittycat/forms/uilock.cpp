
#include "stdafx.h"

#include "ui_lock.h"
#include "uilock.h"


static const unsigned secsInMin = 60;
static const unsigned secsInHour = 60 * secsInMin;
static const unsigned secsInDay = 24 * secsInHour;


LockView::LockView(CatCtl * ctl, QWidget *parent /*= 0*/)
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
    , ui_(new Ui::Lock)
    , ctl_(ctl)
{
    ui_->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);

    updateUi();

    connect(ctl_, &CatCtl::gameEvent, this, &LockView::onGameEvent);
}

void LockView::changeEvent(QEvent *event)
{
    switch (event->type())
    {
        case QEvent::LanguageChange:
            ui_->retranslateUi(this);
            break;
    }

    QWidget::changeEvent(event);
}

void LockView::on_btnApply_clicked()
{
    ui_->lbMessage->setText(QString());

    int days = ui_->sbDays->value();
    QTime time = ui_->sbTime->time();

    unsigned lockTime = days * secsInDay + time.hour() * secsInHour + time.minute() * secsInMin + time.second();
    ctl_->setLockTime(lockTime);

    logMessage(tr("Setting lock time to %1").arg(lockTime));
}

void LockView::onGameEvent(const qlib::GameEvent & e)
{
    switch(e->type())
    {
        case BaseEvent::SetLockTime:
        {
            const SetLockTimeEvent *event = static_cast<const SetLockTimeEvent *>(e.data());
            if (event->isOk())
            {
                unsigned lockTime = event->seconds();
                unsigned days = lockTime / secsInDay;
                unsigned hours = (lockTime - days * secsInDay) / secsInHour;
                unsigned mins = (lockTime - days * secsInDay - hours * secsInHour) / secsInMin;
                unsigned secs = lockTime - days * secsInDay - hours * secsInHour - mins * secsInMin;

                ui_->lbMessage->setText(tr("Lock is set to %1 day(s) %2:%3:%4.")
                    .arg(days)
                    .arg(hours, 2, 10, QChar('0'))
                    .arg(mins, 2, 10, QChar('0'))
                    .arg(secs, 2, 10, QChar('0'))
                );

                updateUi();
            }
            else
            {
                ui_->lbMessage->setText(tr("Lock cannot be set at the moment."));
            }
            break;
        }
    }
}

//

void LockView::updateUi()
{
    unsigned lockTime = ctl_->lockTime();

    unsigned days = lockTime / secsInDay;
    unsigned hours = (lockTime - days * secsInDay) / secsInHour;
    unsigned mins = (lockTime - days * secsInDay - hours * secsInHour) / secsInMin;
    unsigned secs = lockTime - days * secsInDay - hours * secsInHour - mins * secsInMin;

    ui_->lbCurrentTime->setText(
        (days > 0 ? tr("%1 day(s) ").arg(days) : QString())
        + tr("%1:%2:%3")
            .arg(hours, 2, 10, QChar('0'))
            .arg(mins, 2, 10, QChar('0'))
            .arg(secs, 2, 10, QChar('0'))
        );

    CatCtl::AccountSelection currentAccount;
    QList<CatCtl::Account> accounts = ctl_->getAccounts(currentAccount);
    if (currentAccount.accountIndex >= 0 
        && currentAccount.accountIndex < accounts.count()
        && currentAccount.characterIndex >= 0
        && currentAccount.characterIndex < accounts[currentAccount.accountIndex].characters.count())
    {
        setWindowTitle(tr("Lock - ") + accounts[currentAccount.accountIndex].characters[currentAccount.characterIndex]);
    }
}
