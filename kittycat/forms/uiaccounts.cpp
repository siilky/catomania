
#include "stdafx.h"

#include "ui_accounts.h"
#include "uiaccounts.h"
#include "catctl.h"
#include "mailru/mailruauth.h"
#include "arc/arcauth.h"
#include "auth_errors.h"

AccountsView::AccountsView(QSharedPointer<CatCtl> ctl, QWidget *parent /*= 0*/)
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
    , ctl_(ctl)
    , ui(new Ui::Accounts)
    , needCloseAfterAuth_(false)
{
    ui->setupUi(this);
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);

    assert(ctl_ != 0);

    CatCtl::AccountSelection currentAccount;
    QList<CatCtl::Account>  accounts = ctl_->getAccounts(currentAccount);
    setAccounts(accounts);

    //setServers(servers_, currentServer_);
    int currentServer = -1;
    QStringList servers = ctl_->getServers(currentServer);

    ui->cbServers->clear();
    ui->cbServers->addItems(servers);;
    ui->cbServers->setCurrentIndex(currentServer);

    QString email, password;
    ctl_->getAuth(email, password);
    ui->leEmail->setText(email);
    ui->lePassword->setText(password);

    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
}

AccountsView::~AccountsView()
{
}

void AccountsView::changeEvent(QEvent *event)
{
    switch (event->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
    }

    QWidget::changeEvent(event);
}

//

void AccountsView::done(int r)
{
    if (r == QDialog::Accepted)
    {
        if (!save())
        {
            // not done
            return;
        }
    }

    QDialog::done(r);
}

void AccountsView::on_buttonBox_clicked(QAbstractButton * button)
{
    if (button != ui->buttonBox->button(QDialogButtonBox::Apply))
    {
        return;
    }

    // Apply operation
    save();
}

void AccountsView::on_leEmail_textChanged(const QString &)
{
    updateButtonState();
}

void AccountsView::on_cbServers_currentIndexChanged(int)
{
    updateButtonState();
}

void AccountsView::on_lePassword_textChanged(const QString &)
{
    updateButtonState();
}

#if defined MAILRU_TOKEN_AUTH
void AccountsView::onMailAuthProgress(int value, int maxValue)
{
    showSuccess(tr("Authorization: %1/%2").arg(value).arg(maxValue));
}

void AccountsView::onMailAuthFinished(bool success)
{
    setRunning(false);

    if (!mailruAuth_)
    {
        return;
    }

    if (!success)
    {
        QString message = mailruAuthErrorToString(mailruAuth_->statusCode());
        if (!message.isEmpty())
        {
            QMessageBox::critical(this, QApplication::applicationName(), message);
        }
        return;
    }

    QList<PwMailRu::GameAccount> accounts = mailruAuth_->accounts();
    if (accounts.isEmpty())
    {
        showError(tr("No valid accounts"));
        return;
    }

    int accId = accounts[0].id;
    QString token = mailruAuth_->token();
    if (token.isEmpty() || accId == 0)
    {
        showError(tr("Failed to get authorization token"));
        return;
    }

    // OK

    ctl_->setAuth(email_, password_);

    CatCtl::AccountSelection oldSelection;
    QList<CatCtl::Account>  oldAccounts = ctl_->getAccounts(oldSelection);

    QString oldAccount, oldCharacter;
    if (oldSelection.accountIndex >= 0 && oldSelection.accountIndex < oldAccounts.size())
    {
        oldAccount = oldAccounts[oldSelection.accountIndex].name;
        if (oldSelection.characterIndex >= 0 && oldSelection.characterIndex < oldAccounts[oldSelection.accountIndex].characters.size())
        {
            oldCharacter = oldAccounts[oldSelection.accountIndex].characters[oldSelection.characterIndex];
        }
    }

    QList<CatCtl::Account>  newAccounts;
    QListIterator<PwMailRu::GameAccount> it(accounts);
    while (it.hasNext())
    {
        CatCtl::Account acc;
        acc.name = it.next().name;
        newAccounts << acc;
    }

    // get new selection
    CatCtl::AccountSelection newSelection;
    for (int accIndex = 0; accIndex < newAccounts.size(); accIndex++)
    {
        const CatCtl::Account & acc = newAccounts[accIndex];

        if (oldAccount == acc.name)
        {
            newSelection.accountIndex = accIndex;

            for (int charIndex = 0; charIndex < newAccounts[accIndex].characters.size(); charIndex++)
            {
                if (oldCharacter == newAccounts[accIndex].characters[charIndex])
                {
                    newSelection.characterIndex = charIndex;
                    break;
                }
            }
            break;
        }
    }

    setAccounts(newAccounts);
    ctl_->setAccounts(newAccounts);

    if (needCloseAfterAuth_)
    {
        close();
    }
    else
    {
        ui->leEmail->setModified(false);
        ui->lePassword->setModified(false);
        ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
        ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);
        showSuccess(QString());
    }
}

#endif

void AccountsView::showError(const QString & str)
{
    ui->lbStatus->setStyleSheet("color: red;");
    ui->lbStatus->setText(str);
}

void AccountsView::showSuccess(const QString & str)
{
    ui->lbStatus->setStyleSheet("color: green;");
    ui->lbStatus->setText(str);
}

//

void AccountsView::setAccounts(const QList<CatCtl::Account> & accounts)
{
    ui->treeWidget->clear();

    foreach (CatCtl::Account account, accounts)
    {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget, QStringList() << account.name);
        item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
        item->setIcon(0, QIcon(":/config/group.png"));

        QListIterator<QString> i(account.characters);
        while (i.hasNext())
        {
            QTreeWidgetItem *child = new QTreeWidgetItem(item, QStringList() << i.next());
            child->setFlags(Qt::ItemIsEnabled);
            child->setIcon(0, QIcon(":/config/user.png"));
        }
    }
    ui->treeWidget->expandAll();
}

void AccountsView::updateButtonState()
{
    bool isEnabled = !ui->leEmail->text().isEmpty() && !ui->lePassword->text().isEmpty();

    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(isEnabled);
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(isEnabled);
}

void AccountsView::setRunning(bool isRunning)
{
    ui->buttonBox->button(QDialogButtonBox::Apply)->setEnabled(!isRunning);
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(!isRunning);
}

bool AccountsView::save()
{
    // set current server
    int currentServer = ui->cbServers->currentIndex();
    ctl_->setCurrentServer(currentServer);

    if ((ui->leEmail->isModified() || ui->lePassword->isModified()))
    {
#if defined MAILRU_TOKEN_AUTH
        needCloseAfterAuth_ = true;
        startAuth();
        return false;
#else
        QString name = ui->leEmail->text();
        ctl_->setAuth(name, ui->lePassword->text());
        CatCtl::AccountSelection oldSelection;
        QList<CatCtl::Account>  accounts = ctl_->getAccounts(oldSelection);
        if (accounts.empty())
        {
            accounts.push_back(CatCtl::Account());
        }
        accounts[0].name = name;
        ctl_->setAccounts(accounts);
#endif
    }
    return true;
}

#if defined MAILRU_TOKEN_AUTH
void AccountsView::startAuth()
{
    if (mailruAuth_)
    {
        delete mailruAuth_;
    }

    email_ = ui->leEmail->text().trimmed();
    password_ = ui->lePassword->text();

    mailruAuth_ = PwMailRu::getAuth(email_);
    if (!mailruAuth_)
    {
        showError(tr("Invalid email or password"));
        return;
    }

    const Options options = ctl_->getOptions();
    if (options.useProxy)
    {
        mailruAuth_->setProxy(ctl_->getProxy());
    }

    connect(mailruAuth_.data(), &PwMailRu::MailruAuth::progress,  this, &AccountsView::onMailAuthProgress);
    connect(mailruAuth_.data(), &PwMailRu::MailruAuth::finished,  this, &AccountsView::onMailAuthFinished);
    connect(mailruAuth_.data(), &PwMailRu::MailruAuth::error,     this, &AccountsView::showError);

    mailruAuth_->start(email_, password_);
    setRunning();
}

#endif
