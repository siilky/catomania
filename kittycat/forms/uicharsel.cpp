
#include "stdafx.h"

#include "ui_charsel.h"
#include "uicharsel.h"
#include "catctl.h"

CharselView::CharselView(const CatCtl::Account & account, QWidget *parent /*= 0*/)
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
    , ui(new Ui::Charsel)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);

    ui->listChars->addItems(account.characters);
}

CharselView::~CharselView()
{
}

int CharselView::getSelected() const
{
    return ui->listChars->currentRow();
}

void CharselView::changeEvent(QEvent *event)
{
    switch (event->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
    }

    QWidget::changeEvent(event);
}
