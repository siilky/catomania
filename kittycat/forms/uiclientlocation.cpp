
#include "stdafx.h"

#include "ui_clientlocation.h"
#include "uiclientlocation.h"
#include "config.h"

ClientLocationView::ClientLocationView(QWidget *parent /*= 0*/)
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
    , ui(new Ui::ClientLocation)
{
    ui->setupUi(this);
    on_tbPath_textChanged("");

#if !defined(GAME_USING_CLIENT)
    ui->leExecutable->hide();
    ui->lbExecutable->hide();
#endif
}

ClientLocationView::~ClientLocationView()
{
}

void ClientLocationView::setPath(const QString & path)
{
    ui->tbPath->setText(path);
}

QString ClientLocationView::getPath() const
{
    return ui->tbPath->text();
}

void ClientLocationView::setExeName(const QString & name)
{
    ui->leExecutable->setText(name);
}

QString ClientLocationView::getExeName() const
{
    return ui->leExecutable->text();
}

//

void ClientLocationView::on_btnOpenPath_clicked()
{
    //fd = new QFileDialog(this, "Open client folder", getPath());
    QString path = QFileDialog::getExistingDirectory(this, "Open client folder", getPath());
    if (!path.isEmpty())
    {
        setPath(path);
    }
}

void ClientLocationView::on_tbPath_textChanged(const QString & text)
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled( ! text.isEmpty());
}

void ClientLocationView::on_buttonBox_accepted()
{
    QString path = getPath();
    if (!QDir(path).exists())
    {
        QMessageBox::critical(this, "Select game path", QString("The path you entered\n%1\n does not exist").arg(path));
    }
    else
    {
        accept();
    }
}

void ClientLocationView::changeEvent(QEvent *event)
{
    switch (event->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
    }

    QWidget::changeEvent(event);
}
