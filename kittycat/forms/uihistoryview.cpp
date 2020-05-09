#include <StdAfx.h>

#include "uihistoryview.hpp"
#include "ui_history.h"

#include "historydb.h"
#include "catctl.h"
#include "qlib\game\iconmanager.h"
#include "qlib\vmp.h"
#include "util.h"

const int HistoryView::itemsWidths[ItemsColumns::NumItemsColumns] = { 150, 50, 80, 80 };
const int HistoryView::eventsWidths[EventsColumns::NumEventsColumns] = { 25, 100, 50, 60};


HistoryView::HistoryView(QSharedPointer<CatCtl> ctl, QWidget * parent)
    : QDialog(parent)
    , ctl_(ctl)
{
    VMProtectBeginVirtualization(__FUNCTION__);
    if (!Vmp::hasOption(Vmp::OptionHistory))
    {
        return;
    }

	ui = new Ui::History();
	ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    ui->splitter->setStretchFactor(0, 3);
    ui->splitter->setStretchFactor(1, 2);

    {
        QHeaderView *header = ui->tableItems->horizontalHeader();
        header->setFont(ui->tableItems->font());
        header->setSectionResizeMode(0, QHeaderView::ResizeMode::Stretch);
        for (int i = 0; i < ItemsColumns::NumItemsColumns; i++)
        {
            header->resizeSection(i, itemsWidths[i]);
        }
    }
    {
        QHeaderView *header = ui->tableEvents->horizontalHeader();
        header->setFont(ui->tableEvents->font());
        for (int i = 0; i < EventsColumns::NumEventsColumns; i++)
        {
            header->resizeSection(i, eventsWidths[i]);
        }
    }

    connect(ui->tableItems, &QTableWidget::itemSelectionChanged, this, &HistoryView::updateEvents);

    VMProtectEnd();

    ui->btnOpOut->setChecked(true);
    ui->btnOpIn->setChecked(true);

    CatCtl::AccountSelection currentAccount;
    QList<CatCtl::Account> accounts = ctl_->getAccounts(currentAccount);
    if (currentAccount.accountIndex >= 0
        && currentAccount.accountIndex < accounts.count()
        && currentAccount.characterIndex >= 0
        && currentAccount.characterIndex < accounts[currentAccount.accountIndex].characters.count())
    {
        setWindowTitle(tr("Trade history - ") + accounts[currentAccount.accountIndex].characters[currentAccount.characterIndex]);
    }

    fillItems();
    ui->tableItems->selectRow(0);
}

HistoryView::~HistoryView()
{
	delete ui;
}


void HistoryView::on_btnOpIn_toggled(bool checked)
{
    if (!checked && !ui->btnOpOut->isChecked())
    {
        ui->btnOpOut->setChecked(true);
    }
    updateEvents();
}


void HistoryView::on_btnOpOut_toggled(bool checked)
{
    if (!checked && !ui->btnOpIn->isChecked())
    {
        ui->btnOpIn->setChecked(true);
    }
    updateEvents();
}

void HistoryView::updateEvents()
{
    quint32 itemId = 0;

    QList<QTableWidgetSelectionRange>	selection = ui->tableItems->selectedRanges();
    if (!selection.isEmpty())
    {
        QTableWidgetItem *i = ui->tableItems->item(selection[0].topRow(), ItemsColumns::Name);
        itemId = i->data(Qt::UserRole).toUInt();
    }

    fillEvents(itemId, ui->btnOpIn->isChecked(), ui->btnOpOut->isChecked());
}

void HistoryView::on_btnReset_clicked()
{
    QList<QTableWidgetSelectionRange>	selection = ui->tableItems->selectedRanges();
    ui->tableItems->clearSelection();

    ctl_->resetProfit();
    fillItems();

    if (!selection.isEmpty())
    {
        ui->tableItems->selectRow(selection[0].topRow());
    }
}

//

void HistoryView::fillItems()
{
    const HistoryDb *h = ctl_->tradeHistory();
    if (!h)
    {
        return;
    }

    const HistoryDb::TradeHistory & items = h->get();
    ui->tableItems->setRowCount(items.size());

    qint64 totalProfit = 0;

    int row = 0;
    for (auto it = items.constBegin(); it != items.constEnd(); ++it, ++row)
    {
        quint32 itemId = it.key();

        QImage image;
        std::string itemIconPath = ctl_->getItemIcon(itemId);
        if (!itemIconPath.empty())
        {
            QTextCodec *tc = QTextCodec::codecForName("GBK");
            QString iconPath = tc->toUnicode(itemIconPath.c_str(), itemIconPath.size());
            image = IconManager::instance()->getIconByPath(iconPath);
        }
        QPixmap pixmap;
        if (!image.isNull())
        {
            pixmap = QPixmap::fromImage(image);
        }
        else
        {
            static QPixmap px(32, 32);
            static bool pxInit = false;
            if (!pxInit)
            {
                px.fill(Qt::transparent);
                pxInit = true;
            }
            pixmap = px;
        }

        QString itemName = ctl_->getItemName(itemId);
        QTableWidgetItem *twItemName = new QTableWidgetItem(QIcon(pixmap), itemName);
        twItemName->setData(Qt::UserRole, itemId);
        ui->tableItems->setItem(row, ItemsColumns::Name, twItemName);


        const HistoryDb::ItemHistory & item = it.value();
        int stock;
        qint64 value, profit;
        getIndicators(item, stock, value, profit);

        QTableWidgetItem *itemStock = new QTableWidgetItem(spacedNumber(stock));
        itemStock->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->tableItems->setItem(row, ItemsColumns::Stock, itemStock);

        QTableWidgetItem *itemValue = new QTableWidgetItem(spacedNumber(value));
        itemValue->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->tableItems->setItem(row, ItemsColumns::StockValue, itemValue);

        QTableWidgetItem *itemProfit = new QTableWidgetItem(spacedNumber(profit));
        itemProfit->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        ui->tableItems->setItem(row, ItemsColumns::Profit, itemProfit);
        totalProfit += profit;
    }

    ui->leProfit->setText(spacedNumber(totalProfit));
}

void HistoryView::fillEvents(quint32 itemId, bool showin, bool showOut)
{
    if (itemId == 0)
    {
        ui->tableEvents->setRowCount(0);
        return;
    }

    const HistoryDb *h = ctl_->tradeHistory();
    if (!h)
    {
        return;
    }

    const HistoryDb::ItemHistory & itemHistory = h->get(itemId);
    ui->tableEvents->setRowCount(itemHistory.size());

    int row = 0;
    for (auto it = itemHistory.constBegin(); it != itemHistory.constEnd(); ++it, ++row)
    {
        const HistoryItem & item = *it;

        ui->tableEvents->setRowHidden(row, ! ((item.operation == HistoryItem::Operation::OperationBuy && showin)
                                            || (item.operation == HistoryItem::Operation::OperationSell && showOut)));

        QIcon opIcon = (item.operation == HistoryItem::Operation::OperationBuy ? QIcon(":/history/arrow_left_16.png") : QIcon(":/history/arrow_right_16.png"));
        ui->tableEvents->setItem(row, EventsColumns::Operation, new QTableWidgetItem(opIcon, QString()));

        QBrush fg(ui->tableEvents->palette().text());
        fg.setColor(item.operation == HistoryItem::Operation::OperationBuy ? QColor("darkred") : QColor("blue"));
        if (!item.isActual)
        {
            QColor fgc = fg.color();
            QColor bgc = ui->tableEvents->palette().base().color();
            fg.setColor(QColor((fgc.red() + bgc.red()) / 2,
                               (fgc.green() + bgc.green()) / 2,
                               (fgc.blue() + bgc.blue()) / 2));
        }

        QDateTime dt = QDateTime::fromTime_t(item.time);
        QString date = dt.toString(tr("MM-dd-yy hh:mm.ss"));
        QTableWidgetItem *itemDate = new QTableWidgetItem(date);
        itemDate->setForeground(fg);
        ui->tableEvents->setItem(row, EventsColumns::Date, itemDate);

        QTableWidgetItem *itemCount = new QTableWidgetItem();
        if (item.actualCount == 0)
        {
            itemCount->setText(spacedNumber(item.count));
        }
        else
        {
            itemCount->setText(QString("* ") + spacedNumber(item.actualCount));
            itemCount->setToolTip(tr("Actual: ") + spacedNumber(item.count));
        }
        itemCount->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemCount->setForeground(fg);
        ui->tableEvents->setItem(row, EventsColumns::Count, itemCount);

        QTableWidgetItem *itemPrice = new QTableWidgetItem(spacedNumber(item.pricePerItem));
        itemPrice->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
        itemPrice->setForeground(fg);
        ui->tableEvents->setItem(row, EventsColumns::Price, itemPrice);
    }
}

