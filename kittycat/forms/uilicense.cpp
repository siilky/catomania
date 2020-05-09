
#include "stdafx.h"

#include "ui_license.h"
#include "uilicense.h"

LicenseView::LicenseView(const QString & status, const QString & id, QWidget *parent /*= 0*/)
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
    , ui(new Ui::License)
{
    ui->setupUi(this);
    ui->lbStatus->setText(status);
    ui->edIdentifier->setText(id);
    //setModal(true);
}

QString LicenseView::license() const
{
    return ui->edLicense->toPlainText();
}
