#include "stdafx.h"

#include "ui_getpin.h"
#include "uigetpin.h"


GetPinView::GetPinView(QString email, QString pcName, QWidget *parent)
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
    , ui(new Ui::GetPin)
{
    ui->setupUi(this);

    ui->lbEmail->setText(email);
    ui->lbPcName->setText(pcName);

    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(false);
}

GetPinView::~GetPinView()
{
}

QString GetPinView::pin() const
{
    return ui->lePin->text();
}

void GetPinView::on_lePin_textChanged(const QString &text)
{
    ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(!text.isEmpty());
}
