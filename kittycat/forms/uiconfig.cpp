
#include "stdafx.h"

#include <array>

#include "ui_config.h"
#include "uiconfig.h"
#include "catctl.h"
#include "MarketSetupModel.h"
#include "util.h"
#include "marketbuilder.h"
#include "game/data/gshop.h"
#include "game/inventory_types.h"
#include "config.h"
#include "uiprices.h"
#include "uimoveitems.h"
#include "uihistoryview.hpp"
#include "historydb.h"
#include "qlib/vmp.h"


extern elements::ItemListCollection  g_elements_;


class ByteConversion
{
public:
    JsonValue convert(const char & v) const
    {
        return JsonValue((int) v, NULL);
    }

    char convertIn(const JsonValue & v) const
    {
        return (char)static_cast<int>(v);
    }
};

//

static inline Qt::CheckState checkStateFrom(const Tristate<bool> & value)
{
    return value.isTristate() ? Qt::PartiallyChecked : (value ? Qt::Checked : Qt::Unchecked);
}

static inline void checkStateTo(Tristate<bool> & value, Qt::CheckState state)
{
    if (state == Qt::PartiallyChecked)
        value.setTristate();
    else
        value = (state == Qt::Checked);
}

static inline void setEditTextFrom(const Tristate<std::wstring> & value, QLineEdit * edit)
{
    if (value.isTristate())
    {
        edit->setPlaceholderText(QObject::tr("Different"));
        edit->setText(QString());
    }
    else
    {
        edit->setPlaceholderText(QString());
        edit->setText(QString::fromStdWString(value));
    }
}

static inline void setEditTextTo(Tristate<std::wstring> & value, QLineEdit * edit)
{
    if ( ! edit->isModified())
    {
        value.setTristate();
    }
    else
    {
        value = edit->text().toStdWString();
    }
}

//

ConfigView::ConfigView(const QList< QSharedPointer<CatCtl> > & cats, JsonValue * config, QWidget *parent /*= 0*/)
    : QDialog(parent, Qt::CustomizeWindowHint | Qt::WindowTitleHint | Qt::WindowCloseButtonHint)
    , ui(new Ui::Config)
    , config_(config)
    , cats_(cats)
    , catShopEnding_(0)
    , serverIndex_(-1)
    , money_(0)
    , gold_(0)
{
    ui->setupUi(this);

    if (config_)
    {
        std::vector<char> geometry;
        config_->get(L"ConfigGeometry", geometry, ByteConversion());
        restoreGeometry(qUncompress(QByteArray(geometry.data(), geometry.size())));
    }

    equipAction_ = new QAction(tr("Equip"), this);

    ticker_.setInterval(1000);
    connect(&ticker_, &QTimer::timeout, this, &ConfigView::ticker);

    proxyModel_ = new QSortFilterProxyModel(this);
    proxyModel_->setSourceModel(&marketModel_);
    ui->tableView->setModel(proxyModel_);
    ui->tableView->setItemDelegateForColumn(MarketSetupModel::IsSelling, new CenteredItemDelegate(this/*ui->tableView*/));
    ui->tableView->setItemDelegateForColumn(MarketSetupModel::SellAutoPrice, new CenteredItemDelegate(this/*ui->tableView*/));
    ui->tableView->setItemDelegateForColumn(MarketSetupModel::IsBuying, new CenteredItemDelegate(this/*ui->tableView*/));
    ui->tableView->setItemDelegateForColumn(MarketSetupModel::BuyAutoPrice, new CenteredItemDelegate(this/*ui->tableView*/));
    ui->tableView->setItemDelegateForColumn(MarketSetupModel::Amount, new SpacedNumberDelegate(this/*ui->tableView*/));
    ui->tableView->setItemDelegateForColumn(MarketSetupModel::SellPrice, new SpacedNumberDelegate(this/*ui->tableView*/));
    ui->tableView->setItemDelegateForColumn(MarketSetupModel::SellAmount, new SpacedNumberDelegate(this/*ui->tableView*/));
    ui->tableView->setItemDelegateForColumn(MarketSetupModel::KeepMax, new SpacedNumberDelegate(this/*ui->tableView*/));
    ui->tableView->setItemDelegateForColumn(MarketSetupModel::BuyPrice, new SpacedNumberDelegate(this/*ui->tableView*/));
    ui->tableView->setItemDelegateForColumn(MarketSetupModel::BuyAmount, new SpacedNumberDelegate(this/*ui->tableView*/));
    ui->tableView->setItemDelegateForColumn(MarketSetupModel::KeepMin, new SpacedNumberDelegate(this/*ui->tableView*/));
    ui->tableView->setItemDelegateForColumn(MarketSetupModel::ApPriceStep, new SpacedNumberDelegate(this/*ui->tableView*/));
    ui->tableView->setItemDelegateForColumn(MarketSetupModel::ApMinCount, new SpacedNumberDelegate(this/*ui->tableView*/));

    QHeaderView *header = ui->tableView->horizontalHeader();
    header->setFont(ui->tableView->font());
    bool headerStateSet = false;
    if (config_)
    {
        std::vector<char> geometry;
        config_->get(L"ConfigHeader", geometry, ByteConversion());
        if (!geometry.empty())
        {
            headerStateSet = header->restoreState(qUncompress(QByteArray(geometry.data(), geometry.size())));
        }
    }
    if (!headerStateSet)
    {
        for (int i = 0; i < header->count(); i++)
        {
            header->resizeSection(i, marketModel_.columnWidth(i));
        }
        // sort indicator shown with unmeaning state
        header->setSortIndicator(-1, Qt::SortOrder::AscendingOrder);
    }
    else
    {
        setDragDropEnabled(header->sortIndicatorSection() < 0);
    }
    connect(header, &QHeaderView::sectionDoubleClicked, this, &ConfigView::resetTableSorting);
    connect(header, &QHeaderView::sortIndicatorChanged, this, &ConfigView::tableSortingChanged);

    connect(equipAction_,  &QAction::triggered,             this, &ConfigView::onEquipAction);
    connect(&catshopMenu_, &QMenu::triggered,               this, &ConfigView::onCatshopMenuTriggered);
    connect(&marketModel_, &MarketSetupModel::dataChanged,  this, &ConfigView::onModelDataChanged);

    // invalidate states
    ui->cbEnableAutoReopen->toggle();
    ui->cbEnableAutoreopenValue->toggle();
    ui->cbEnableAutoreopenPercentage->toggle();
    ui->cbEnableAutoReopenExternal->toggle();
    ui->cbLimitHistory->toggle();
    ui->cbSaveLog->toggle();
    ui->cbDelayConnectStart->toggle();
    ui->cbEnableProxy->toggle();
    ui->cbOnlineRegistration->toggle();

    ui->tbShopTitle->setMaxLength(CatshopMaxCharsMax);

    if (isSingleMode())
    {
        setCatshop(cats_[0]->getCatshop());
            
        itemMenu_.addAction(equipAction_);

        ui->btnBuyCatshop->setMenu(&catshopMenu_);

        setMoney(cats_[0]->getMoney(), cats_[0]->getBanknotes());
        setGold(cats_[0]->getGold());

        catshops_ = getAvailableCatshops();
        updateAvailableCatshops();
    }
    else
    {
        hasCatshop_ = true;    // in multi we generally put no catshop-related limits
        catshopMaxSlots_ = CatshopMaxSlotsMax;
        catshopMaxChars_ = 0;

        ui->lbCatshopTimer->setText(QString());
        ui->btnBuyCatshop->setEnabled(false);

        ui->lbToSell->setText(QString());
        ui->lbToSell->setEnabled(false);
        ui->lbToBuy->setText(QString());
        ui->lbToBuy->setEnabled(false);

        ui->cbLowConverter->setTristate();
        ui->cbHighConverter->setTristate();
        ui->cbEnableAutoReopen->setTristate();
        ui->cbEnableAutoreopenPercentage->setTristate();
        ui->cbEnableAutoreopenValue->setTristate();
        ui->cbOnlineRegistration->setTristate();
        ui->cbLimitHistory->setTristate();
        ui->cbSaveLog->setTristate();
        ui->cbSplitLog->setTristate();
        ui->cbDelayConnectStart->setTristate();
        ui->cbEnableProxy->setTristate();

        ui->udAutoreopenModeSelector->setSpecialValueText(tr("Different"));
        ui->udAutoreopenModeSelector->setMinimum(-1);
        ui->udAutoreopenPercentage->setSpecialValueText(tr("Different"));
        ui->udAutoreopenPercentage->setMinimum(-1);
        ui->udAutoreopenValue->setSpecialValueText(tr("Different"));
        ui->udAutoreopenValue->setMinimum(-1);
        ui->udAutoreopenDelayClose->setSpecialValueText(tr("Different"));
        ui->udAutoreopenDelayClose->setMinimum(-1);
        ui->udAutoreopenDelayCloseRnd->setSpecialValueText(tr("Different"));
        ui->udAutoreopenDelayCloseRnd->setMinimum(-1);
        ui->udAutoreopenDelayOpen->setSpecialValueText(tr("Different"));
        ui->udAutoreopenDelayOpen->setMinimum(-1);
        ui->udAutoreopenDelayOpenRnd->setSpecialValueText(tr("Different"));
        ui->udAutoreopenDelayOpenRnd->setMinimum(-1);

        ui->udConnectDelayFixed->setSpecialValueText(tr("Different"));
        ui->udConnectDelayFixed->setMinimum(-1);
        ui->udConnectDelayRandom->setSpecialValueText(tr("Different"));
        ui->udConnectDelayRandom->setMinimum(-1);

        ui->wConvert->setEnabled(true);
        ui->sbLowConverter->setSpecialValueText(tr("Different"));
        ui->sbLowConverter->setMinimum(0);
        ui->sbHighConverter->setSpecialValueText(tr("Different"));
        ui->sbHighConverter->setMinimum(0);
    }

    VMProtectBeginVirtualization(__FUNCTION__);
    if (!Vmp::hasOption(Vmp::OptionHistory))
    {
        ui->wProfit->hide();
    }
    // header saves hidden state so we neet to set it each time
    if (Vmp::hasOption(Vmp::OptionAutoprice))
    {
        ui->tableView->showColumn(MarketSetupModel::SellAutoPrice);
        ui->tableView->showColumn(MarketSetupModel::BuyAutoPrice);
        ui->tableView->showColumn(MarketSetupModel::ApPriceStep);
        ui->tableView->showColumn(MarketSetupModel::ApMinCount);
    }
    else
    {
        ui->tableView->hideColumn(MarketSetupModel::SellAutoPrice);
        ui->tableView->hideColumn(MarketSetupModel::BuyAutoPrice);
        ui->tableView->hideColumn(MarketSetupModel::ApPriceStep);
        ui->tableView->hideColumn(MarketSetupModel::ApMinCount);
    }
    VMProtectEnd();

#if !defined(ARC_TOKEN_AUTH)
    ui->gbArc->hide();
#endif

    setupFromCtls();
}

ConfigView::~ConfigView()
{
    if (config_)
    {
        config_->set(L"ConfigGeometry", toVector(qCompress(saveGeometry(), 9)), ByteConversion());
        config_->set(L"ConfigHeader", toVector(qCompress(ui->tableView->horizontalHeader()->saveState(), 9)), ByteConversion());
    }
}

//

void ConfigView::done(int r)
{
    if (r == QDialog::Accepted)
    {
        saveSettings();
    }

    return QDialog::done(r);
}

void ConfigView::updateMarketSetup()
{
    setupFromCtls(false);
}

//

void ConfigView::changeEvent(QEvent *event)
{
    switch (event->type())
    {
        case QEvent::LanguageChange:
            ui->retranslateUi(this);
            break;
    }

    QWidget::changeEvent(event);
}

void ConfigView::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (historyView_)
    {
        int frameW = (frameGeometry().width() - geometry().width()) / 2;
        historyView_->move(frameGeometry().right() - frameW, frameGeometry().top());
        historyView_->resize(historyView_->width(), event->size().height());
    }
}

void ConfigView::moveEvent(QMoveEvent *event)
{
    QDialog::moveEvent(event);
    if (historyView_)
    {
        //historyView_->move(frameGeometry().left() + width() /*+ frameW*/, frameGeometry().top());
        int frameW = (frameGeometry().width() - geometry().width()) / 2;
        historyView_->move(frameGeometry().right() - frameW, frameGeometry().top());
    }
}

//

void ConfigView::on_btnProfitDetails_toggled(bool checked)
{
    if (checked && !historyView_)
    {
        VMProtectBeginVirtualization(__FUNCTION__);
        if (!Vmp::hasOption(Vmp::OptionHistory))
        {
            return;
        }

        historyView_ = new HistoryView(cats_.at(0), this);
        historyView_->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
        historyView_->setAttribute(Qt::WidgetAttribute::WA_ShowWithoutActivating);
        historyView_->setWindowFlags(historyView_->windowFlags() | Qt::Tool /*| Qt::FramelessWindowHint*/);
        connect(historyView_, &QDialog::finished, this, &ConfigView::onHistoryViewClosed);
        historyView_->show();

        VMProtectEnd();

        historyView_->resize(historyView_->width(), height());

        QRect g = qApp->desktop()->screenGeometry(this);
        int maxw = frameGeometry().right() + historyView_->frameGeometry().width();
        if (maxw > g.width())
        {
            QPoint pp = pos();
            pp.setX(qMax((pp.x() - (maxw - g.width())), 0));
            move(pp);   // move updates history view
        }
        else
        {
            QRect f = frameGeometry();
            int frameW = (f.width() - geometry().width()) / 2;
            historyView_->move(f.right() - frameW, f.top());
        }
    }
    else if (!checked && historyView_)
    {
        historyView_->close();
    }
}

void ConfigView::onHistoryViewClosed()
{
    ui->btnProfitDetails->setChecked(false);
}

void ConfigView::on_slChatBufferSize_valueChanged(int value)
{
    chatBufferActualValue_ = value;
    setSlChatComment(value);
}

void ConfigView::on_btnGlobalSettings_clicked()
{
//     reject();
    globalConfig();
}

void ConfigView::on_btnOpenLogPath_clicked()
{
    QString r = QFileDialog::getExistingDirectory( this
                                                 , tr("Save logs to folder")
                                                 , ui->tbLogPath->text());
    if ( ! r.isEmpty())
    {
        ui->tbLogPath->setText(r);
    }
}

void ConfigView::on_cbEnableAutoReopen_toggled(bool checked)
{
    ui->udAutoreopenPercentage->setEnabled(checked && ui->cbEnableAutoreopenPercentage->isChecked());
    ui->udAutoreopenValue->setEnabled(checked && ui->cbEnableAutoreopenValue->isChecked());

    ui->udAutoreopenDelayClose->setEnabled(checked || ui->cbEnableAutoReopenExternal->isChecked());
    ui->udAutoreopenDelayCloseRnd->setEnabled(checked || ui->cbEnableAutoReopenExternal->isChecked());
    ui->udAutoreopenDelayOpen->setEnabled(checked || ui->cbEnableAutoReopenExternal->isChecked());
    ui->udAutoreopenDelayOpenRnd->setEnabled(checked || ui->cbEnableAutoReopenExternal->isChecked());
}

void ConfigView::on_cbEnableAutoreopenPercentage_toggled(bool checked)
{
    ui->udAutoreopenPercentage->setEnabled(checked && ui->cbEnableAutoReopen->isChecked());
}

void ConfigView::on_cbEnableAutoreopenValue_toggled(bool checked)
{
    ui->udAutoreopenValue->setEnabled(checked && ui->cbEnableAutoReopen->isChecked());
}

void ConfigView::on_cbEnableAutoReopenExternal_toggled(bool checked)
{
    ui->udAutoreopenDelayClose->setEnabled(checked || ui->cbEnableAutoReopen->isChecked());
    ui->udAutoreopenDelayCloseRnd->setEnabled(checked || ui->cbEnableAutoReopen->isChecked());
    ui->udAutoreopenDelayOpen->setEnabled(checked || ui->cbEnableAutoReopen->isChecked());
    ui->udAutoreopenDelayOpenRnd->setEnabled(checked || ui->cbEnableAutoReopen->isChecked());
}

void ConfigView::on_tbShopTitle_textChanged(const QString & /*text*/)
{
    updateMarketTitleCount();
    setUnsavedData(true);
}

void ConfigView::on_cbLowConverter_toggled(bool /*checked*/)
{
    updateConverter();
    setUnsavedData(true);
}

void ConfigView::on_sbLowConverter_valueChanged(int i)
{
    if (i > 0 && ui->sbHighConverter->value() < (i + 1))
    {
        ui->sbHighConverter->setValue(i + 1);
    }
    setUnsavedData(true);
}

void ConfigView::on_cbHighConverter_toggled(bool)
{
    updateMarketStats();    // update buying sum
    setUnsavedData(true);
}

void ConfigView::on_sbHighConverter_valueChanged(int i)
{
    if (i > 0 && ui->sbLowConverter->value() > (i - 1))
    {
        ui->sbLowConverter->setValue(i - 1);
    }
    updateMarketStats();    // update buying sum
    setUnsavedData(true);
}

void ConfigView::on_tableView_customContextMenuRequested(const QPoint & pos)
{
    if ( ! isSingleMode())
    {
        return;
    }

    QModelIndex index = ui->tableView->indexAt(pos);
    if (index.row() < 0
        || index.column() != MarketSetupModel::Name)
    {
        return;
    }

    QVariant v = marketModel_.data(proxyModel_->mapToSource(index), MarketSetupModel::MarketItemRole);
    if (v.isNull())
    {
        return;
    }

    MarketItemSetup *mi = (MarketItemSetup *)v.value<void*>();
    // check if item is one of catshops
    int i = 0;
    for (; i < catshops_.size(); i++)
    {
        if (mi->item.id == catshops_[i].itemId)
        {
            break;
        }
    }
    if (i < catshops_.size())
    {
        equipAction_->setData(int(mi->slot));

        itemMenu_.popup(ui->tableView->viewport()->mapToGlobal(pos));
    }
}

void ConfigView::onEquipAction()
{
    bool ok;
    int inventorySlot = equipAction_->data().toInt(&ok);
    if (ok)
    {
        cats_[0]->equipCatShop(inventorySlot);
    }
}

void ConfigView::onTableViewClicked(const QModelIndex & index)
{
    if (index.column() != MarketSetupModel::Name)
    {
        return;
    }
    QVariant v = marketModel_.data(proxyModel_->mapToSource(index), MarketSetupModel::MarketItemRole);
    if (v.isNull())
    {
        return;
    }

    QPoint point( 5 + ui->tableView->columnViewportPosition(index.column())
                , ui->tableView->rowViewportPosition(index.row()));
    point = ui->tableView->mapToGlobal(point);

    MarketItemSetup *mi = (MarketItemSetup *)v.value<void*>();
    PricesView *p = new PricesView(mi->item.id, QString::fromStdWString(mi->item.name), serverIndex_, this);
    p->setAttribute(Qt::WA_DeleteOnClose, true);

    p->move(point);
    p->show();
    p->activateWindow();
}

void ConfigView::onModelDataChanged(const QModelIndex & /*topLeft*/, const QModelIndex & /*bottomRight*/)
{
    setUnsavedData(true);
    updateMarketStats();
}

void ConfigView::tableSortingChanged(int logicalIndex, Qt::SortOrder /*order*/)
{
    // logicalIndex = -1 : no sorting, else one of
    setDragDropEnabled(logicalIndex < 0);
}

void ConfigView::resetTableSorting()
{
    ui->tableView->model()->sort(-1);
    ui->tableView->horizontalHeader()->setSortIndicator(-1, Qt::SortOrder::AscendingOrder);
}

void ConfigView::onCatshopMenuTriggered(QAction *action)
{
    unsigned itemId = action->data().toUInt();
    if (itemId > 0 && isSingleMode())
    {
        cats_[0]->buyCatshop(itemId);
    }
}

void ConfigView::onMoveItems(unsigned srcSlot, int dstSlot, bool copyItems, bool insertBefore)
{
    if (cats_.size() != 1)
    {
        assert(0);
        return;
    }

    if (hasUnsavedData_)
    {
        QMessageBox::StandardButton res = QMessageBox::warning(this
          , tr("Unsaved data")
          , tr("Setup window has unsaved data that would be lost after move operation.\nWould you like to save changes?")
          , QMessageBox::Save | QMessageBox::Discard | QMessageBox::Abort);
        if (res == QMessageBox::Save)
        {
            saveSettings();
        }
        else if (res == QMessageBox::Abort)
        {
            return;
        }
    }

    QSharedPointer<CatCtl> cat = cats_.at(0);
    QList<InventoryItem> inventory = cat->getInventoryItems();
    if (inventory.isEmpty())
    {
        return;
    }

    if (srcSlot > unsigned(inventory.size())
        || dstSlot > inventory.size()
        || inventory.at(srcSlot).isEmpty()
        || (dstSlot >= 0 && !insertBefore && inventory.at(dstSlot).isEmpty())
        )
    {
        // inv should be empty when not logged in
        qDebug() << "Move items: incorrect information";
        return;
    }

    // swap

    if (dstSlot >= 0 
        && !insertBefore
        && !inventory.at(dstSlot).isEmpty()
        && inventory.at(dstSlot).id != inventory.at(srcSlot).id)
    {
        cat->moveItems(srcSlot, dstSlot, inventory.at(srcSlot).count);
        return;
    }

    // move ()

    unsigned toMoveMax = 0;
    if (dstSlot >= 0 && !insertBefore)
    {
        assert(inventory.at(dstSlot).isEmpty() || (inventory.at(dstSlot).id == inventory.at(srcSlot).id));

        elements::Item item;
        if (g_elements_.getItem(inventory.at(srcSlot).id, item))
        {
            unsigned stackDst = item["stack"];
            if ( !inventory.at(dstSlot).isEmpty())
            {
                if (inventory.at(dstSlot).count >= stackDst)
                {
                    // cannot move
                    qDebug() << "Cannot move slot" << srcSlot << "to" << dstSlot << " : slot is full";
                    return;
                }
                toMoveMax = std::min(stackDst - inventory.at(dstSlot).count, inventory.at(srcSlot).count);
            }
            else
            {
                // move to empty
                toMoveMax = std::min(stackDst, inventory.at(srcSlot).count);
            }
        }
    }
    else if (dstSlot < 0)
    {
        // вставка в конец
        toMoveMax = inventory.at(srcSlot).count;
        dstSlot = inventory.size();
        insertBefore = true;
    }
    else // insertBefore && dstSlot > 0
    {
        toMoveMax = inventory.at(srcSlot).count;
    }

    if (copyItems && toMoveMax > 1)
    {
        if (moveItemsAsk_)
        {
            delete moveItemsAsk_;
        }
        moveItemsAsk_ = new MoveItems(this);
        moveItemsAsk_->setMaximum(toMoveMax);
        connect(moveItemsAsk_.data(), &QDialog::finished, this, &ConfigView::onMoveItemsAskDone);

        moveItemsSrcSlot_ = srcSlot;
        moveItemsDstSlot_ = dstSlot;
        moveItemsInsertBefore_ = insertBefore;

        QRect rect = moveItemsAsk_->rect();
        QRect screen = QApplication::desktop()->screenGeometry(this);

        QPoint p = QCursor::pos();
        QPoint onScreen = mapToGlobal(p);
        p.setX(p.x() + ((onScreen.x() + rect.width()) > screen.width() ? -(10 + rect.width()) : 10));
        p.setY(p.y() + ((onScreen.y() + rect.height()) > screen.height() ? -(10 + rect.height()) : 10));

        moveItemsAsk_->move(p);
        moveItemsAsk_->open();
        moveItemsAsk_->activateWindow();
    }
    else
    {
        doMoveItems(srcSlot, dstSlot, inventory.at(srcSlot).count, insertBefore);
    }
}

void ConfigView::ticker()
{
    updateCatshop();
}

void ConfigView::onGameEvent(const qlib::GameEvent & ev)
{
    BaseEvent *event = ev.data();

    switch (ev->type())
    {
        case BaseEvent::InventoryUpdate:
        {
            InventoryUpdateEvent *e = static_cast<InventoryUpdateEvent*>(event);
            if (isSingleMode())
            {
                QSharedPointer<CatCtl> ctl = cats_[0];

                if ((e->storageType() & MONEY_INFO) != 0)
                {
                    setMoney(ctl->getMoney(), ctl->getBanknotes());
                }
                if ((e->storageType() & GOLD_INFO) != 0)
                {
                    setGold(ctl->getGold());
                }
                if ((e->storageType() & EQUIPMENT_INFO) != 0)
                {
                    setCatshop(ctl->getCatshop());
                }
            }
            if ((e->storageType() & BAG_INFO) != 0)
            {
                setupFromCtls(true);
            }

            break;
        }
    }
}

void ConfigView::onMoveItemsAskDone(int result)
{
    if (result != QDialog::Accepted)
    {
        return;
    }

    doMoveItems(moveItemsSrcSlot_, moveItemsDstSlot_, moveItemsAsk_->value(), moveItemsInsertBefore_);
}

void ConfigView::saveSettings()
{
    Options opts;
    getOptions(opts);

    QString marketName;
    if (ui->tbShopTitle->isModified())
    {
        marketName = ui->tbShopTitle->text();
    }

    for (int iCat = 0; iCat < cats_.count(); iCat++)
    {
        cats_[iCat]->setOptions(opts);
        if (!marketName.isNull())     // null means different
        {
            cats_[iCat]->setMarketName(marketName);
        }
        cats_[iCat]->setShopItems(marketModel_.getData());
    }

    setUnsavedData(false);
}

//

void ConfigView::setupFromCtls(bool updateItemsOnly /*= false*/)
{
    struct MarketItemSetupEx : public MarketItemSetup
    {
        MarketItemSetupEx(const MarketItemSetup & r)
            : MarketItemSetup(r)
            , isVisited(true)   // true - новые предметы не должны быть слиты в текущем проходе
        {}

        bool isVisited;
    };
    QList<MarketItemSetupEx> setupList;
    Tristate<QString> marketName;
    int serverIndex = -1;
    Options options;

    for (int iCat = 0; iCat < cats_.count(); iCat++)
    {
        const QSharedPointer<CatCtl> & cat = cats_[iCat];

        // event
        connect( cat.data(),    &CatCtl::gameEvent
               , this,          &ConfigView::onGameEvent
               , Qt::UniqueConnection);

        // предусмотрим, что одинаковые итемы с одного кота не должны сливаться в один.
        // т.е. итемы нужно сливать по порядку появления

        // сбросим флаги перед каждым новым котом
        for (int i = 0; i < setupList.size(); i++)
        {
            setupList[i].isVisited = false;
        }

        // add items from cat
        QList<MarketItemSetup> catItems = cat->getShopItems();
        for (int iCatItems = 0; iCatItems < catItems.count(); iCatItems++)
        {
            // see if setup list already contains this item
            bool merged = false;
            for (int iSetupList = 0; iSetupList < setupList.count(); iSetupList++)
            {
                if (!setupList[iSetupList].isVisited
                    && setupList[iSetupList].isEqualItem(catItems[iCatItems]))
                {
                    setupList[iSetupList].isVisited = true;
                    setupList[iSetupList] << catItems[iCatItems];
                    merged = true;
                    break;
                }
            }

            if ( ! merged)
            {
                setupList.append(catItems[iCatItems]);
            }
        }

        // add server
        if (serverIndex == -1)
        {
            serverIndex = cat->getCurrentServerAsPwcatsIndex();
        }
        else if (serverIndex > -1
                && serverIndex != cat->getCurrentServerAsPwcatsIndex())
        {
            serverIndex = -2;
        }

        if (!updateItemsOnly)
        {
            // add name
            if (iCat == 0)
            {
                marketName = cat->getMarketName();    // override zero item
            }
            else
            {
                marketName << cat->getMarketName();
            }

            // add proxy setup
            if (iCat == 0)
            {
                options = cat->getOptions();        // override zero item
            }
            else
            {
                options << cat->getOptions();
            }
        }
    }

    serverIndex_ = serverIndex;
    if (serverIndex_ >= 0)
    {
        if (!priceLinkConnection_)
        {
            ui->tableView->setItemDelegateForColumn(MarketSetupModel::Name, new RenderLinkDelegate(this));
            priceLinkConnection_ = connect(ui->tableView, &QTableView::clicked
                , this, &ConfigView::onTableViewClicked);
        }
    }
    else
    {
        if (priceLinkConnection_)
        {
            ui->tableView->setItemDelegateForColumn(MarketSetupModel::Name, new QStyledItemDelegate(this));
            disconnect(priceLinkConnection_);
        }
    }

    if ( ! updateItemsOnly)
    {
        if (marketName.isTristate())
        {
            ui->tbShopTitle->setText(QString());
            ui->tbShopTitle->setPlaceholderText(tr("Different"));
        }
        else
        {
            ui->tbShopTitle->setText(marketName);
        }

        setOptions(options);
    }

    if (cats_.size() == 1)
    {
        marketModel_.setItemsMovable(true);
        connect(&marketModel_, &MarketSetupModel::moveItems, this, &ConfigView::onMoveItems, Qt::UniqueConnection);
    }
    else
    {
        marketModel_.setItemsMovable(false);
    }

    QList<MarketItemSetup> result;
    std::copy(setupList.begin(), setupList.end(), std::back_inserter(result));

    marketModel_.setData(result);
    setUnsavedData(false);
    updateMarketStats();
}

void ConfigView::setOptions(const Options & options)
{
    ui->cbDelayConnectStart->setCheckState(checkStateFrom(options.reconnectDelayOnStartup));

    ui->udConnectDelayFixed->setValue(options.reconnectDelayFixed.valueOpt(-1));
    ui->udConnectDelayRandom->setValue(options.reconnectDelayRandom.valueOpt(-1));

    ui->cbEnableAutoReopen->setCheckState(checkStateFrom(options.useReopen));
    ui->udAutoreopenModeSelector->setValue(options.reopenModeSelector.valueOpt(-1));
    ui->cbEnableAutoreopenPercentage->setCheckState(checkStateFrom(options.useReopenPercentage));
    ui->udAutoreopenPercentage->setValue(options.reopenPercentage.valueOpt(-1));
    ui->cbEnableAutoreopenValue->setCheckState(checkStateFrom(options.useReopenValue));
    ui->udAutoreopenValue->setValue(options.reopenValue.valueOpt(-1));
    ui->udAutoreopenDelayOpen->setValue(options.openDelayFixed.valueOpt(-1));
    ui->udAutoreopenDelayOpenRnd->setValue(options.openDelayRandom.valueOpt(-1));
    ui->udAutoreopenDelayClose->setValue(options.closeDelayFixed.valueOpt(-1));
    ui->udAutoreopenDelayCloseRnd->setValue(options.closeDelayRandom.valueOpt(-1));
    ui->cbEnableAutoReopenExternal->setCheckState(checkStateFrom(options.useReopenExternal));
    ui->udAutoReopenExternalInterval->setValue(options.reopenExternalInterval.valueOpt(-1));
    ui->cbSavePartialConfig->setCheckState(checkStateFrom(options.saveExternal));

    ui->cbOnlineRegistration->setCheckState(checkStateFrom(options.onlineReg));
    
    ui->cbLimitHistory->setCheckState(options.maxLogLines.isTristate() ? Qt::PartiallyChecked
                                        : (options.maxLogLines > 0 ? Qt::Checked : Qt::Unchecked));
    ui->udLimitHistory->setValue(options.maxLogLines.value());
    ui->cbSaveLog->setCheckState(checkStateFrom(options.saveLogToFile));
    ui->cbSplitLog->setCheckState(checkStateFrom(options.splitLogFilesPerDay));
    setEditTextFrom(options.logPath, ui->tbLogPath);

    ui->cbEnableProxy->setCheckState(checkStateFrom(options.useProxy));
    setEditTextFrom(options.proxyAddr, ui->leProxyHost);
    setEditTextFrom(options.proxyUser, ui->leProxyUser);
    setEditTextFrom(options.proxyPass, ui->leProxyPassword);

    if (options.proxyType.isTristate())
    {
        // uncheck both
        ui->rbProxyTypeSocks->setAutoExclusive(false);
        ui->rbProxyTypeHttp->setAutoExclusive(false);
        ui->rbProxyTypeSocks->setChecked(false);
        ui->rbProxyTypeHttp->setChecked(false);
        ui->rbProxyTypeSocks->setAutoExclusive(true);
        ui->rbProxyTypeHttp->setAutoExclusive(true);
    }
    else
    {
        if (options.proxyType == Options::ProxySocks)
        {
            ui->rbProxyTypeSocks->setChecked(true);
        }
        else
        {
            ui->rbProxyTypeHttp->setChecked(true);
        }
    }

    ui->cbLowConverter->setCheckState(checkStateFrom(options.lowConverter));
    ui->sbLowConverter->setValue(options.lowConverterValue.valueOpt(0));

    ui->cbHighConverter->setCheckState(checkStateFrom(options.highConverter));
    ui->sbHighConverter->setValue(options.highConverterValue.valueOpt(0));

    updateConverter();

    if (options.chatBufferSize.isTristate())
    {
        chatBufferActualValue_ = -1;
        setSlChatComment(-1);
    }
    else
    {
        int bufferSize = options.chatBufferSize.value();
        ui->slChatBufferSize->setValue(bufferSize);
        setSlChatComment(bufferSize);
    }

#if defined(ARC_TOKEN_AUTH)
    setEditTextFrom(options.arcHwid, ui->leArcId);
#endif
    ticker_.start();
}

void ConfigView::getOptions(Options & options) const
{
    checkStateTo(options.reconnectDelayOnStartup, ui->cbDelayConnectStart->checkState());

    options.reconnectDelayFixed.setValueOpt (ui->udConnectDelayFixed->value(), -1);
    options.reconnectDelayRandom.setValueOpt(ui->udConnectDelayRandom->value(), -1);

    checkStateTo(options.useReopen, ui->cbEnableAutoReopen->checkState());
    options.reopenModeSelector.setValueOpt(ui->udAutoreopenModeSelector->value(), -1);
    checkStateTo(options.useReopenPercentage, ui->cbEnableAutoreopenPercentage->checkState());
    options.reopenPercentage.setValueOpt(ui->udAutoreopenPercentage->value(), -1);
    checkStateTo(options.useReopenValue, ui->cbEnableAutoreopenValue->checkState());
    options.reopenValue.setValueOpt(ui->udAutoreopenValue->value(), -1);
    options.openDelayFixed.setValueOpt(ui->udAutoreopenDelayOpen->value(), -1);
    options.openDelayRandom.setValueOpt(ui->udAutoreopenDelayOpenRnd->value(), -1);
    options.closeDelayFixed.setValueOpt(ui->udAutoreopenDelayClose->value(), -1);
    options.closeDelayRandom.setValueOpt(ui->udAutoreopenDelayCloseRnd->value(), -1);
    checkStateTo(options.useReopenExternal, ui->cbEnableAutoReopenExternal->checkState());
    options.reopenExternalInterval.setValueOpt(ui->udAutoReopenExternalInterval->value(), -1);
    checkStateTo(options.saveExternal, ui->cbSavePartialConfig->checkState());

    checkStateTo(options.onlineReg,          ui->cbOnlineRegistration->checkState());

    if (ui->cbLimitHistory->checkState() == Qt::PartiallyChecked)
    {
        options.maxLogLines.setTristate();
    }
    else
    {
        options.maxLogLines = ui->cbLimitHistory->isChecked() ? ui->udLimitHistory->value() : 0;
    }
    checkStateTo(options.saveLogToFile, ui->cbSaveLog->checkState());
    checkStateTo(options.splitLogFilesPerDay, ui->cbSplitLog->checkState());
    setEditTextTo(options.logPath, ui->tbLogPath);
    
    checkStateTo(options.useProxy, ui->cbEnableProxy->checkState());
    setEditTextTo(options.proxyAddr, ui->leProxyHost);
    setEditTextTo(options.proxyUser, ui->leProxyUser);
    setEditTextTo(options.proxyPass, ui->leProxyPassword);

    if (ui->rbProxyTypeSocks->isChecked())
        options.proxyType = Options::ProxySocks;
    else if (ui->rbProxyTypeHttp->isChecked())
        options.proxyType = Options::ProxyHttp;
    else
        options.proxyType.setTristate();

    checkStateTo(options.lowConverter, ui->cbLowConverter->checkState());
    checkStateTo(options.highConverter, ui->cbHighConverter->checkState());

    options.lowConverterValue.setValueOpt(ui->sbLowConverter->value(), 0);
    options.highConverterValue.setValueOpt(ui->sbHighConverter->value(), 0);

    if (chatBufferActualValue_ < 0)
    {
        options.chatBufferSize.setTristate(true);
    }
    else
    {
        options.chatBufferSize = ui->slChatBufferSize->value();
    }

#if defined(ARC_TOKEN_AUTH)
    setEditTextTo(options.arcHwid, ui->leArcId);
#endif
}

void ConfigView::setMoney(unsigned money, unsigned banknotes)
{
    money_ = money;
    banknotes_ = banknotes;
    updateMoneyText();
    updateMarketStats();
}

void ConfigView::setGold(unsigned gold)
{
    gold_ = gold;
    updateMoneyText();
    updateAvailableCatshops();
}

void ConfigView::setCatshop(const InventoryItem & catshop)
{
    // presets

    cats_.at(0)->getCatshopLimits(catshop, catshopMaxSlots_, catshopMaxChars_);
    hasCatshop_ = catshopMaxSlots_ != 0;
    catShopEnding_ = catshop.timeLimit;
    if (catshopMaxSlots_ == 0)
    {
        catshopMaxSlots_ = CatshopMaxSlotsDefault;
    }
    if (catshopMaxChars_ == 0)
    {
        catshopMaxChars_ = CatshopMaxCharsDefault;
    }

    updateMarketTitleCount();
    updateCatshop();
    updateMarketStats();    // update limits, if any
}

QList<GShop::Item> ConfigView::getAvailableCatshops()
{
    QList<GShop::Item> result;

    std::shared_ptr<elements::ItemList> catshops = g_elements_.list(elements::VENDOR_SKINS);
    if ( ! catshops)
    {
        return result;
    }

    const GShop::GShop * gshop = GShop::GShop::instance();
    for (elements::ItemList::const_iterator it = catshops->begin(); it != catshops->end(); ++it)
    {
        unsigned itemId = it->second["ID"];
        // фактически это не совсем правильно, тк шоп может содержать предметы, не поставленные в продажу
        int gIndex = gshop->indexOf(itemId);
        if (gIndex >= 0)
        {
            result.append(gshop->items()[gIndex]);
        }
    }
    return result;
}

void ConfigView::updateMarketStats()
{
    QList<MarketItemSetup> items = marketModel_.getData();
    int buyLimit = money_;
    if (ui->cbHighConverter->isChecked())
    {
        unsigned value = ui->sbHighConverter->value();
        if (value > 1)
        {
            buyLimit = std::min(money_, (value - 1) * 10000000);
        }
    }

    unsigned maxSlots = 0;
    for each (auto cat in cats_)
    {
        unsigned s, unused;
        cat->getCatshopLimits(cat->getCatshop(), s, unused);
        maxSlots = std::max(maxSlots, s);
    }
    if (maxSlots == 0)
    {
        maxSlots = CatshopMaxSlotsDefault;
    }

    long long sellSum, buySum;
    unsigned sellCount, buyCount;
    getMarketSums( items.toVector().toStdVector()
                 , (hasCatshop_ ? InventoryMoneyLimitCatshop : InventoryMoneyLimit) - money_
                 , buyLimit
                 , maxSlots
                 , sellSum
                 , buySum
                 , sellCount
                 , buyCount);

    unsigned sellSet = 0, buySet = 0;
    for (auto item : items)
    {
        if (item.doSell)
        {
            ++sellSet;
        }
        if (item.doBuy)
        {
            ++buySet;
        }
    }
    static const QString yellowStyle("background-color: #FFC;");
    ui->lbToSell->setStyleSheet(sellSet > catshopMaxSlots_ ? yellowStyle : QString());
    ui->lbToBuy->setStyleSheet(buySet > catshopMaxSlots_ ? yellowStyle : QString());
    QString sellText, buyText;
    if (sellSet > sellCount)
    {
        sellText = QString("%1 (%2/%3 +%4)").arg(spacedNumber(sellSum)).arg(sellCount).arg(maxSlots).arg(sellSet - sellCount);
    }
    else
    {
        sellText = QString("%1 (%2/%3)").arg(spacedNumber(sellSum)).arg(sellCount).arg(maxSlots);
    }
    if (buySet > buyCount)
    {
        buyText = QString("%1 (%2/%3 +%4)").arg(spacedNumber(buySum)).arg(buyCount).arg(maxSlots).arg(buySet - buyCount);
    }
    else
    {
        buyText = QString("%1 (%2/%3)").arg(spacedNumber(buySum)).arg(buyCount).arg(maxSlots);
    }
    ui->lbToSell->setText(sellText);
    ui->lbToBuy->setText(buyText);

    // profit
    VMProtectBeginVirtualization(__FUNCTION__);
    if (Vmp::hasOption(Vmp::OptionHistory))
    {
        ui->wProfit->setVisible(cats_.size() == 1);
        if (cats_.size() == 1)
        {
            const HistoryDb *h = cats_.at(0)->tradeHistory();
            if (!h)
            {
                return;
            }

            const HistoryDb::TradeHistory & history = h->get();
            qint64 totalProfit = 0;
            for (auto it = history.constBegin(); it != history.constEnd(); ++it)
            {
                const HistoryDb::ItemHistory & item = it.value();
                int stock;
                qint64 value, profit;
                getIndicators(item, stock, value, profit);
                totalProfit += profit;
            }
            ui->lbProfit->setText(spacedNumber(totalProfit));
        }
    }
    VMProtectEnd();
}

void ConfigView::updateMarketTitleCount()
{
    unsigned count = ui->tbShopTitle->text().size();
    if (catshopMaxChars_ > 0)
    {
        ui->lbTitleLength->setText(QString("<span style=\"color:#%1;\">%2</span> / %3")
                                   .arg((count > catshopMaxChars_) ? "FF0000" : "000000")
                                   .arg(count)
                                   .arg(catshopMaxChars_)
                                   );
    }
    else
    {
        ui->lbTitleLength->setText(QString::number(count));
    }
}

void ConfigView::updateCatshop()
{
    if ( ! hasCatshop_)
    {
        ui->lbCatshop->setEnabled(false);
        ui->lbCatshopTimer->setText(tr("Catshop: None"));
        ui->wConvert->setEnabled(false);
        return;
    }

    // have catshop   v

    ui->lbCatshop->setEnabled(true);
    ui->wConvert->setEnabled(true);

    if (catShopEnding_ != 0)
    {
        int diff = catShopEnding_ - QDateTime::currentDateTimeUtc().toTime_t();
        if (diff < 0)
        {
            // коровка истекла
            hasCatshop_ = false;
            catshopMaxChars_ = CatshopMaxCharsDefault;
            updateCatshop();
            return;
        }

        int days = diff / (60 * 60 * 24);   diff -= days * (60 * 60 * 24);
        int hours = diff / (60 * 60);       diff -= hours * (60 * 60);
        int minutes = diff / 60;            diff -= minutes * 60;
        ui->lbCatshopTimer->setText(tr("Catshop: %1d %2:%3:%4")
            .arg(days)
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(diff, 2, 10, QChar('0'))
            );
    }
    else
    {
        ui->lbCatshopTimer->setText(tr("Catshop"));
    }
}

void ConfigView::updateConverter()
{
    if (ui->cbLowConverter->isChecked())
    {
        ui->sbHighConverter->setMinimum(2);
    }
    else
    {
        ui->sbHighConverter->setMinimum(1);
    }
// не нужно 
//     if (ui->cbHighConverter->isChecked())
//     {
//         ui->sbLowConverter->setMaximum(18);
//     }
//     else
//     {
//         ui->sbLowConverter->setMaximum(19);
//     }
}

void ConfigView::updateAvailableCatshops()
{
    catshopMenu_.clear();

    for (int i = 0; i < catshops_.size(); i++)
    {
        const GShop::Item & item = catshops_[i];

        QAction *action = catshopMenu_.addAction(QIcon(), QString("%1 (%2.%3)")
                                                        .arg(QString::fromStdWString(item.name))
                                                        .arg(item.saleOptions[0].price / 100)
                                                        .arg(item.saleOptions[0].price % 100, 2, 10, QChar('0'))
                                        );
        action->setData(item.shopId);
        if (gold_ < item.saleOptions[0].price)
        {
            action->setDisabled(true);
        }
    }
}

void ConfigView::updateMoneyText()
{
    if ( ! isSingleMode())
    {
        return;
    }

    QString text;
    if (gold_ > 0)
    {
        text += tr("<img src=\":/catView/coin_gold.png\"/> %1.%2&nbsp;&nbsp;&nbsp;").arg(gold_ / 100).arg(gold_ % 100, 2, 10, QChar('0'));
    }
    if (banknotes_ > 0)
    {
        text += tr("<img src=\":/catView/money.png\"/> %1 &nbsp; ").arg(banknotes_);
    }
    text += tr("<img src=\":/catView/coins.png\"/> %1").arg(spacedNumber(money_));

    ui->lbMoney->setText(text);
}

void ConfigView::setSlChatComment(int value)
{
    static const std::array<const char *, 6> comments {
        QT_TR_NOOP("Tiny"),
        QT_TR_NOOP("Small"),
        QT_TR_NOOP("Medium"),
        QT_TR_NOOP("Large"),
        QT_TR_NOOP("Huge"),
        QT_TR_NOOP("???"),
    };

    if (value < 0)
    {
        ui->lbChatBufferSize->setText(tr("Different"));
    }
    else
    {
        if (value >= (int)comments.size())
        {
            value = comments.size() - 1;
        }
        ui->lbChatBufferSize->setText(tr(comments[value]));
    }
}

void ConfigView::doMoveItems(unsigned srcSlot, unsigned dstSlot, unsigned count, bool insertBefore /*insert before dstSlot*/)
{
    assert(cats_.size() == 1);

    QSharedPointer<CatCtl> cat = cats_.at(0);
    QList<InventoryItem> inventory = cat->getInventoryItems();
    if (inventory.size() == 0
        || srcSlot > unsigned(inventory.size())
        || dstSlot > unsigned(inventory.size())
        || inventory.at(srcSlot).isEmpty()
        )
    {
        return;
    }

    if (insertBefore)
    {
        // бабочкой вокруг назначения найдем первый пустой слот
        // вниз - по массиву (вверх по инвентарю)
        unsigned slotUp = dstSlot;
        unsigned slotDown = dstSlot > 0 ? dstSlot - 1 : dstSlot;
        while ((slotDown  > 0 || slotUp < unsigned(inventory.size()))
                && !inventory.at(slotDown).isEmpty() 
                && (slotUp >= unsigned(inventory.size()) || !inventory.at(slotUp).isEmpty()))
        {
            if (slotDown > 0)
            {
                slotDown--;
            }
            if (slotUp < unsigned(inventory.size() - 1))
            {
                slotUp++;
            }
        }
        // сначала работает вставка так же и в пустой слот перед предметом
        if (inventory.at(slotDown).isEmpty())
        {
            // [] * *  D    [] D
            // передвигаем этот кусок вниз (то есть вверх по инвентарю) <-
            //  * * [] D    [] D

            for (unsigned slot = slotDown; slot != (dstSlot - 1); slot++)
            {
                cat->swapItems(slot + 1, slot);
            }
            // исходный тоже сдвинулся
            if (srcSlot > slotDown && srcSlot < dstSlot)
            {
                srcSlot--;
            }
            // вставляем в слот перед слотом назначения
            dstSlot--;
        }
        else if (slotUp < unsigned(inventory.size()) // вставка в конец выходит за границы
                 && inventory.at(slotUp).isEmpty())
        {
            //  D * * []   * D []
            // передвигаем этот кусок вверх (то есть вниз) ->
            // [] D * *    * [] D
            unsigned slot = slotUp;
            do
            {
                cat->swapItems(slot - 1, slot);
                slot--;
            } while (slot != dstSlot);

            // исходный тоже сдвинулся
            if (srcSlot < slotUp && srcSlot > dstSlot)
            {
                srcSlot++;
            }
            // вставляем в слот назначения
        }
        else
        {
            // ничего не получится
            qDebug() << "No room to insert item";
            return;
        }

    }
    cat->moveItems(srcSlot, dstSlot, count);
}

void ConfigView::setUnsavedData(bool isUnsaved)
{
    hasUnsavedData_ = isUnsaved;
    if (!isUnsaved)
    {
        setWindowTitle(windowTitle().remove('*'));
    }
    else if (!windowTitle().contains('*'))
    {
        setWindowTitle(windowTitle() + '*');
    }
}

void ConfigView::setDragDropEnabled(bool isEnabled)
{
    ui->tableView->setDragEnabled(isEnabled);
}
