
#include "stdafx.h"

#include "ui_trade.h"
#include "uitrade.h"
#include "catctl.h"
#include "qlib/game/iconmanager.h"
#include "util.h"


TradeView::TradeView(CatCtl * ctl, const QString & playerName, QWidget *parent /*= 0*/)
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
    , ui_(new Ui::Trade)
    , ctl_(ctl)
    , myMoney_(0)
    , partnerMoney_(0)
    , isMyAccepted_(false)
    , isPartnerAccepted_(false)
{
    ui_->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose, true);

    moneyValidator_.setBottom(0);
    ui_->eMoney->setValidator(&moneyValidator_);

    assert(ctl_ != 0);
    connect(ctl_, &CatCtl::gameEvent,       this, &TradeView::onGameEvent);
    connect(this, &TradeView::rejected,     ctl_, &CatCtl::discardTrade);
    connect(&errorTimer_, &QTimer::timeout, this, &TradeView::onErrorTimer);

    ui_->lbPlayerName->setText(playerName);

    // items
    const QList<InventoryItem> inventory = ctl_->getInventoryItems();
    int row = 0;
    for(int slot = 0; slot < inventory.count(); slot++)
    {
        const InventoryItem & item = inventory.at(slot);
        if (item.id == InventoryItem::INVALID_ID)
        {
            continue;
        }

        inventoryItems_.append(InventoryItemEx(item, slot));
        addTableItem(ui_->twInventory, item, row);
        row++;
    }

    // money
    money_ = ctl->getMoney();
    moneyValidator_.setTop(money_);
    ui_->lbInventoryMoney->setText(spacedNumber(money_));

    disableButtons();
    setPartnerConfirm(false);
    setPartnerAccept(false);
}

TradeView::~TradeView()
{
}

void TradeView::onGameEvent(const qlib::GameEvent & ev)
{
    switch (ev->type())
    {
        case BaseEvent::TradeEnd:
            accept();
            break;

        case BaseEvent::TradeAddItem:
        {
            const TradeAddItemEvent *event = static_cast<const TradeAddItemEvent *>(ev.data());
            if (event->status() != 0)
            {
                Log("Item adding failed with %i", event->status());
                showError(tr("Item %1 cannot be traded").arg(ctl_->getItemName(event->itemId())));
                // do nothing with locking
                break;
            }

            if (event->isSelf())
            {
                // add to my items
                itemAddedToTrade(event->inventorySlot(), event->itemId(), event->attrs(), event->count());
                moneyAddedToTrade(event->money());

                setPartnerConfirm(false);
                setPartnerAccept(false);
                isPartnerAccepted_ = false;
            }
            else
            {
                // add to partner items
                itemAddedToPartnerTrade(event->inventorySlot(), event->itemId(), event->attrs(), event->count());
                moneyAddedToPartnerTrade(event->money());

                if (isMyAccepted_)
                {
                    isMyAccepted_ = false;
                    isPartnerAccepted_ = false;
                    showError(tr("Items changed, trade reopened"));
                    initLockStage();
                }
            }

            break;
        }

        case BaseEvent::TradeRemoveItem:
        {
            const TradeRemoveItemEvent *event = static_cast<const TradeRemoveItemEvent *>(ev.data());
            if (event->status() != 0)
            {
                Log("Item removing failed with %i", event->status());
                showError(tr("Failed to remove item"));
                enableRemoveButtons();
                // do nothing with locking
                break;
            }

            if (event->isSelf())
            {
                // remove my items
                itemRemovedFromTrade(event->inventorySlot(), event->itemId(), event->attrs(), event->count());
                moneyRemovedFromTrade(event->money());

                setPartnerConfirm(false);
                setPartnerAccept(false);
                isPartnerAccepted_ = false;
            }
            else
            {
                // remove partner items
                itemRemovedFromPartnerTrade(event->inventorySlot(), event->itemId(), event->attrs(), event->count());
                moneyRemovedFromPartnerTrade(event->money());

                if (isMyAccepted_)
                {
                    isMyAccepted_ = false;
                    isPartnerAccepted_ = false;
                    showError(tr("Items changed, trade reopened"));
                    initLockStage();
                }
            }

            break;
        }


        case BaseEvent::TradeConfirm:
        {
            const TradeConfirmEvent *event = static_cast<const TradeConfirmEvent *>(ev.data());
            if (event->isSelf())
            {
                if (event->stage() == TradeConfirmEvent::Stage1)
                {
                    isMyAccepted_ = true;

                    lockInput(true);
                    setConfirm(true);
                    if (isPartnerAccepted_)
                    {
                        setAccept(false);
                    }
                }
                else if (event->stage() == TradeConfirmEvent::Stage2)
                {
                    lockInput(true);
                    setAccept(true);
                }
                else
                {
                    initLockStage();
                    showError(tr("Cannot complete trade"));

                    setPartnerConfirm(false);
                    setPartnerAccept(false);
                }
            }
            else
            {
                if (event->stage() == TradeConfirmEvent::Stage1)
                {
                    isPartnerAccepted_ = true;
                    setPartnerConfirm(true);
                    setPartnerAccept(false);
                    if (isMyAccepted_)
                    {
                        setAccept(false);
                    }
                }
                else if (event->stage() == TradeConfirmEvent::Stage2)
                {
                    setPartnerConfirm(true);
                    setPartnerAccept(true);
                }
                else
                {
                    initLockStage();
                    showError(tr("Cannot complete trade"));

                    setPartnerConfirm(false);
                    setPartnerAccept(false);
                }
            }
        }

        default:
            break;
    }
}

//

void TradeView::on_btnAddOne_clicked()
{
    addSelectedItemsToTrade(1);
    ui_->twInventory->clearSelection();
}

void TradeView::on_btnAddAll_clicked()
{
    addSelectedItemsToTrade(-1);
    ui_->twInventory->clearSelection();
}

void TradeView::on_btnAddAllExOne_clicked()
{
    addSelectedItemsToTrade(-2);
    ui_->twInventory->clearSelection();
}

void TradeView::on_btnRemoveOne_clicked()
{
    removeSelectedItemsFromTrade(1);
    ui_->twMyItems->clearSelection();
}

void TradeView::on_btnRemoveAll_clicked()
{
    removeSelectedItemsFromTrade(-1);
    ui_->twMyItems->clearSelection();
}

void TradeView::on_btnSetMoney_clicked()
{
    bool success;
    unsigned value = ui_->eMoney->text().toUInt(&success);
    ui_->eMoney->setText(QString());

    if (success)
    {
        int difference = value - myMoney_;
        if (difference > 0)
        {
            ctl_->addTradeMoney(difference);
        }
        else if (difference < 0)
        {
            ctl_->removeTradeMoney(-difference);
        }
    }
}

void TradeView::on_btnSetMax_clicked()
{
    ui_->eMoney->setText(QString::number(money_));
}

void TradeView::on_btnConfirm_clicked()
{
    ctl_->submitTrade();
}

void TradeView::on_btnAccept_clicked()
{
    ctl_->confirmTrade();
}

void TradeView::on_twInventory_itemDoubleClicked(QTableWidgetItem * item)
{
    if (item == 0)
    {
        return;
    }

    int row = item->row();
    addItemToTrade(row);
}

void TradeView::on_twInventory_itemSelectionChanged()
{
    // enable only if not disabled items are selected
    QList<QTableWidgetItem *> selection = ui_->twInventory->selectedItems();
    if (hasEnabledItems(selection))
    {
        enableAddButtons();
    }
    else
    {
        disableAddButtons();
    }
}

void TradeView::on_twMyItems_itemDoubleClicked(QTableWidgetItem * item)
{
    if (item == 0)
    {
        return;
    }

    int row = item->row();
    removeItemFromTrade(row);
}

void TradeView::on_twMyItems_itemSelectionChanged()
{
    QList<QTableWidgetItem *> selection = ui_->twMyItems->selectedItems();
    if (hasEnabledItems(selection))
    {
        enableRemoveButtons();
    }
    else
    {
        disableRemoveButtons();
    }
}

void TradeView::on_twMyItems_itemChanged(QTableWidgetItem * item)
{
    if (item->column() != 0
        || item->row() > myItems_.count()
        || item->row() < 0)
    {
        return;
    }

    const InventoryItemEx & myItem = myItems_.at(item->row());

    QString numText = item->text();
    numText.remove(' ');
    bool ok;
    unsigned newCount = numText.toUInt(&ok);
    if (ok)
    {
        int diff = newCount - myItem.count;
        if (diff == 0)
        {
            return;
        }

        if (diff > 0)
        {
            int invRow = findItem(inventoryItems_, myItem.id, myItem.inventorySlot);
            if (invRow >= 0)
            {
                addItemToTrade(invRow, diff);
            }
            else
            {
                assert(0);
            }
        }
        else
        {
            removeItemFromTrade(item->row(), -diff);
        }
    }

    item->setText(spacedNumber(myItem.count));
}

void TradeView::onErrorTimer()
{
    ui_->lbError->setText(QString());
}

//

void TradeView::disableAddButtons()
{
    ui_->btnAddOne->setEnabled(false);
    ui_->btnAddAll->setEnabled(false);
    ui_->btnAddAllExOne->setEnabled(false);
}

void TradeView::disableRemoveButtons()
{
    ui_->btnRemoveOne->setEnabled(false);
    ui_->btnRemoveAll->setEnabled(false);
}

void TradeView::enableAddButtons()
{
    ui_->btnAddOne->setEnabled(true);
    ui_->btnAddAll->setEnabled(true);
    ui_->btnAddAllExOne->setEnabled(true);
}

void TradeView::enableRemoveButtons()
{
    ui_->btnRemoveOne->setEnabled(true);
    ui_->btnRemoveAll->setEnabled(true);
}

void TradeView::disableButtons()
{
    disableAddButtons();
    disableRemoveButtons();
}

void TradeView::enableButtons()
{
    enableAddButtons();
    enableRemoveButtons();
}

void TradeView::addTableItem(QTableWidget *table, const InventoryItem & item, int row)
{
    int tRow = row >= 0 ? row : table->rowCount();
    table->insertRow(tRow);
    
    table->blockSignals(true);

    QTableWidgetItem *countItem = new QTableWidgetItem(spacedNumber(item.count));
    countItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
    table->setItem(tRow, 0, countItem);

    QString name = item.name.empty() ? spacedNumber(item.id) : QString::fromStdWString(item.name);
    QImage image;
    if ( ! item.icon.empty())
    {
        QTextCodec *tc = QTextCodec::codecForName("GBK");
        QString icon = tc->toUnicode(item.icon.c_str(), item.icon.size());
        image = IconManager::instance()->getIconByPath(icon);
    }
    QPixmap pixmap;
    if ( ! image.isNull())
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

    QTableWidgetItem *nameItem = new QTableWidgetItem(pixmap, name);
    nameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    nameItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    table->setItem(tRow, 1, nameItem);

    table->blockSignals(false);
}

void TradeView::disableTableItem(int row)
{
    ui_->twInventory->item(row, 0)->setFlags(Qt::NoItemFlags);
    ui_->twInventory->item(row, 1)->setFlags(Qt::NoItemFlags);
}

void TradeView::enableTableItem(int row)
{
    ui_->twInventory->item(row, 0)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    ui_->twInventory->item(row, 1)->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
}

bool TradeView::hasEnabledItems(QList<QTableWidgetItem *> list)
{
    foreach(QTableWidgetItem *item, list)
    {
        assert(item != 0);
        if (item == 0)
        {
            continue;
        }

        // take first column only
        if (item->column() != 0)
        {
            continue;
        }
        if (item->flags().testFlag(Qt::ItemIsEnabled))
        {
            return true;
        }
    }
    return false;
}

int TradeView::findItem(const QList<InventoryItemEx> & list, unsigned itemId, unsigned inventorySlot)
{
    for(int i = 0; i < list.count(); i++)
    {
        if (list.at(i).inventorySlot == inventorySlot
            && list.at(i).id == itemId)
        {
            return i;
        }
    }
    return -1;
}

// actions

void TradeView::addItemToTrade(int row, int count /*= -1*/)
{
    assert(row < inventoryItems_.size());

    const int itemId = inventoryItems_.at(row).id;
    const int inventorySlot = inventoryItems_.at(row).inventorySlot;

    int maxItemCount = inventoryItems_.at(row).count;
    //check if it is already added
    int myRow = findItem(myItems_, itemId, inventorySlot);
    if (myRow >= 0)
    {
        maxItemCount -= myItems_[myRow].count;
        ui_->twMyItems->item(myRow, 0)->setText(spacedNumber(myItems_.at(myRow).count));
    }

    int itemCount = count == -1 ? maxItemCount
        : (count < -1 ? maxItemCount - 1 : std::min(maxItemCount, count));
    if (itemCount > 0)
    {
        ctl_->addTradeItem(inventoryItems_.at(row).inventorySlot, itemCount);
    }
}

void TradeView::addSelectedItemsToTrade(int count)
{
    QList<QTableWidgetItem *> selection = ui_->twInventory->selectedItems();
    foreach(QTableWidgetItem *item, selection)
    {
        assert(item != 0);
        if (item == 0)
        {
            continue;
        }

        // take first column only
        if (item->column() != 0)
        {
            continue;
        }
        // skip already disabled
        if ( ! item->flags().testFlag(Qt::ItemIsEnabled))
        {
            continue;
        }

        int row = item->row();
        assert(row < inventoryItems_.size());
        addItemToTrade(row, count);
    }
}

void TradeView::removeItemFromTrade(int row, int count /*= -1*/)
{
    assert(row < inventoryItems_.size());

    int itemCount = count < 0 ? myItems_.at(row).count : count;
    ctl_->removeTradeItem(myItems_.at(row).inventorySlot, itemCount);
}

void TradeView::removeSelectedItemsFromTrade(int count)
{
    QList<QTableWidgetItem *> selection = ui_->twMyItems->selectedItems();
    foreach(QTableWidgetItem *item, selection)
    {
        assert(item != 0);
        if (item == 0)
        {
            continue;
        }

        // take first column only
        if (item->column() != 0)
        {
            continue;
        }

        int row = item->row();
        assert(row < myItems_.size());
        removeItemFromTrade(row, count);
    }
}

// results

void TradeView::itemAddedToTrade(unsigned inventorySlot, DWORD itemId, const barray & /*attrs*/, unsigned count)
{
    if (itemId == 0)
    {
        // no item
        return;
    }

    // get slot
    int iRow = findItem(inventoryItems_, itemId, inventorySlot);
    if (iRow < 0)
    {
        Log("Adding items slot not found: %i", inventorySlot);
        return;
    }

    if (count >= inventoryItems_.at(iRow).count)
    {
        disableTableItem(iRow);
    }

    int myRow = findItem(myItems_, itemId, inventorySlot);
    if (myRow >= 0)
    {
        myItems_[myRow].count += count;
        ui_->twMyItems->blockSignals(true);
        ui_->twMyItems->item(myRow, 0)->setText(spacedNumber(myItems_.at(myRow).count));
        ui_->twMyItems->blockSignals(false);

        if (myItems_[myRow].count >= inventoryItems_.at(iRow).count)
        {
            disableTableItem(iRow);
        }
    }
    else
    {
        InventoryItemEx myItem = inventoryItems_.at(iRow);
        myItem.count = count;
        // TBD attrs ?
        myItems_.append(myItem);
        addTableItem(ui_->twMyItems, myItem);
    }
}

void TradeView::itemRemovedFromTrade(unsigned inventorySlot, DWORD itemId, const barray & /*attrs*/, unsigned count)
{
    if (itemId == 0)
    {
        // no item
        return;
    }

    int myRow = findItem(myItems_, itemId, inventorySlot);
    if (myRow < 0)
    {
        Log("Removing items my slot not found: %i", inventorySlot);
        return;
    }

    int iRow = findItem(inventoryItems_, itemId, inventorySlot);
    if (iRow >= 0)
    {
        enableTableItem(iRow);
    }
    else
    {
        Log("Removing items slot not found: %i", inventorySlot);
    }

    if (myItems_.at(myRow).count <= count)
    {
        // removing all items
        ui_->twMyItems->removeRow(myRow);
        myItems_.removeAt(myRow);
    }
    else
    {
        myItems_[myRow].count -= count;
        ui_->twMyItems->blockSignals(true);
        ui_->twMyItems->item(myRow, 0)->setText(spacedNumber(myItems_.at(myRow).count));
        ui_->twMyItems->blockSignals(false);
    }
}

void TradeView::itemAddedToPartnerTrade(unsigned inventorySlot, DWORD itemId, const barray & /*attrs*/, unsigned count)
{
    if (itemId == 0)
    {
        // no item
        return;
    }

    int row = findItem(partnerItems_, itemId, inventorySlot);
    if (row >= 0)
    {
        partnerItems_[row].count += count;
        ui_->twPartnerItems->item(row, 0)->setText(spacedNumber(partnerItems_.at(row).count));
    }
    else
    {
        InventoryItemEx item(inventorySlot);
        item.id = itemId;
        item.count = count;
        item.name = ctl_->getItemName(itemId).toStdWString();
        item.icon = ctl_->getItemIcon(itemId);
        // TBD attrs
        partnerItems_.append(item);
        addTableItem(ui_->twPartnerItems, item);
    }
}

void TradeView::itemRemovedFromPartnerTrade(unsigned inventorySlot, DWORD itemId, const barray & /*attrs*/, unsigned count)
{
    if (itemId == 0)
    {
        // no item
        return;
    }

    int row = findItem(partnerItems_, itemId, inventorySlot);
    if (row < 0)
    {
        Log("Removing items slot not found: %i", inventorySlot);
        return;
    }

    if (partnerItems_.at(row).count <= count)
    {
        // removing all items
        ui_->twPartnerItems->removeRow(row);
        partnerItems_.removeAt(row);
    }
    else
    {
        partnerItems_[row].count -= count;
        ui_->twPartnerItems->item(row, 0)->setText(spacedNumber(partnerItems_.at(row).count));
    }
}

void TradeView::moneyAddedToTrade(unsigned money)
{
    myMoney_ += money;
    ui_->lbMyMoney->setText(spacedNumber(myMoney_));
}

void TradeView::moneyAddedToPartnerTrade(unsigned money)
{
    partnerMoney_ += money;
    ui_->lbPartnerMoney->setText(spacedNumber(partnerMoney_));
}

void TradeView::moneyRemovedFromTrade(unsigned money)
{
    myMoney_ -= money;
    ui_->lbMyMoney->setText(spacedNumber(myMoney_));
}

void TradeView::moneyRemovedFromPartnerTrade(unsigned money)
{
    partnerMoney_ -= money;
    ui_->lbPartnerMoney->setText(spacedNumber(partnerMoney_));
}

//

void TradeView::showError(const QString & msg)
{
    ui_->lbError->setText(msg);
    errorTimer_.start(4000);
}

void TradeView::setConfirm(bool isOn)
{
    if (isOn)
    {
        ui_->btnConfirm->setIcon(QIcon(":/trade/tick.png"));
        ui_->btnConfirm->setEnabled(false);
    }
    else
    {
        ui_->btnConfirm->setIcon(QIcon());
        ui_->btnConfirm->setEnabled(true);
    }
}

void TradeView::setAccept(bool isOn)
{
    if (isOn)
    {
        ui_->btnAccept->setIcon(QIcon(":/trade/tick.png"));
        ui_->btnAccept->setEnabled(false);
    }
    else
    {
        ui_->btnAccept->setIcon(QIcon());
        ui_->btnAccept->setEnabled(true);
    }
}

void TradeView::setPartnerConfirm(bool isOn)
{
    if (isOn)
    {
        ui_->lbConfirm->setText(QString("<span style=\"color:#000000;\">%1</span><br/><img src=\":/trade/tick.png\"/>")
            .arg(tr("Confirm")));
    }
    else
    {
        ui_->lbConfirm->setText(QString("<span style=\"color:#808080;\">%1</span><br/><img src=\":/trade/empty_16.png\"/>")
            .arg(tr("Confirm")));
    }
}

void TradeView::setPartnerAccept(bool isOn)
{
    if (isOn)
    {
        ui_->lbAccept->setText(QString("<span style=\"color:#000000;\">%1</span><br/><img src=\":/trade/tick.png\"/>")
            .arg(tr("Accept")));
    }
    else
    {
        ui_->lbAccept->setText(QString("<span style=\"color:#808080;\">%1</span><br/><img src=\":/trade/empty_16.png\"/>")
            .arg(tr("Accept")));
    }
}

void TradeView::lockInput(bool isOn)
{
    const bool isEnabled = !isOn;
    ui_->twMyItems->setEnabled(isEnabled);
    ui_->twInventory->setEnabled(isEnabled);
    ui_->eMoney->setEnabled(isEnabled);
    ui_->btnSetMoney->setEnabled(isEnabled);
    if (!isEnabled)
    {
        ui_->twInventory->clearSelection();
        ui_->twMyItems->clearSelection();
    }
}

void TradeView::initLockStage()
{
    lockInput(false);
    setConfirm(false);
    setAccept(false);
    ui_->btnAccept->setEnabled(false);
}
