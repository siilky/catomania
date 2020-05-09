
#include "stdafx.h"

#include "ui_gconfig.h"
#include "uigconfig.h"

GConfigView::GConfigView(QWidget *parent /*= 0*/)
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
    , ui(new Ui::GConfig)
{
    ui->setupUi(this);
}

GConfigView::~GConfigView()
{
}

void GConfigView::setOptions(const GlobalOptions & opts)
{
    ui->cbMinimizeToTray->setChecked(opts.minimizeToTray);
    ui->cbLanguages->clear();
    ui->cbLanguages->addItems(opts.languages);
    ui->cbLanguages->setCurrentIndex(opts.currentLanguage);
}

GlobalOptions GConfigView::getOptions() const
{
    GlobalOptions opts;

    opts.minimizeToTray  = ui->cbMinimizeToTray->isChecked();
    opts.currentLanguage = ui->cbLanguages->currentIndex();

    return opts;
}

void GConfigView::changeEvent(QEvent *event)
{
    switch (event->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
    }

    QWidget::changeEvent(event);
}
