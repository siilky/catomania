#ifndef uiconfig_h
#define uiconfig_h

#include <QDialog>
#include <QSortFilterProxyModel>
#include <QMenu>
#include <QMetaObject>
#include <QTimer>
#include <QPointer>

#include "MarketItemSetup.h"
#include "MarketSetupModel.h"
#include "qlib/game/gamebase.h"

namespace Ui
{
    class Config;
}
namespace GShop
{
    class Item;
}

class CatCtl;
class Options;
class InventoryItem;
class HistoryView;
class MoveItems;

class ConfigView
    : public QDialog
{
    Q_OBJECT
public:
    explicit ConfigView(const QList< QSharedPointer<CatCtl> > & ctls, JsonValue * config, QWidget *parent = 0);
    virtual ~ConfigView();

signals:
    void globalConfig();

public slots:
    virtual void done(int r);

    void updateMarketSetup();

protected:
    virtual void changeEvent(QEvent * e);
    virtual void resizeEvent(QResizeEvent *);
    virtual void moveEvent(QMoveEvent *);

private slots:
    void onModelDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight);
    void tableSortingChanged(int logicalIndex, Qt::SortOrder order);
    void resetTableSorting();

    void on_btnProfitDetails_toggled(bool checked);
    void onHistoryViewClosed();

    void on_slChatBufferSize_valueChanged(int value);
    void on_btnGlobalSettings_clicked();
    void on_btnOpenLogPath_clicked();
    void on_cbEnableAutoReopen_toggled(bool);
    void on_cbEnableAutoreopenPercentage_toggled(bool);
    void on_cbEnableAutoreopenValue_toggled(bool);
    void on_cbEnableAutoReopenExternal_toggled(bool);

    void on_tbShopTitle_textChanged(const QString & text);
    void on_cbLowConverter_toggled(bool);
    void on_sbLowConverter_valueChanged(int i);
    void on_cbHighConverter_toggled(bool);
    void on_sbHighConverter_valueChanged(int i);
    void on_tableView_customContextMenuRequested(const QPoint & pos);
    void onEquipAction();
    void onTableViewClicked(const QModelIndex & index);
    void onCatshopMenuTriggered(QAction *);
    void onMoveItems(unsigned srcSlot, int dstSlot, bool copyItems, bool insertBefore);

    void ticker();

    void onGameEvent(const qlib::GameEvent & ev);
    void onMoveItemsAskDone(int result);

private:
    enum 
    {
        CatshopMaxSlotsMax      = 18,
        CatshopMaxCharsMax      = 20,
    };

    bool isSingleMode() const
    {
        return cats_.size() == 1;
    }

    void saveSettings();
    void setupFromCtls(bool updateItemsOnly = false);
    void setOptions(const Options & opts);
    void getOptions(Options & opts) const;
    void setMoney(unsigned money, unsigned banknotes);
    void setGold(unsigned gold);
    void setCatshop(const InventoryItem & catshop);

    QList<GShop::Item> ConfigView::getAvailableCatshops();

    void updateMarketStats();
    void updateMarketTitleCount();
    void updateCatshop();
    void updateConverter();
    void updateAvailableCatshops();
    void updateMoneyText();
    void setSlChatComment(int value);

    void doMoveItems(unsigned srcSlot, unsigned dstStol, unsigned count, bool insertBefore);
    void setUnsavedData(bool isUnsaved);
    void setDragDropEnabled(bool isEnabled);

    JsonValue                   *config_;
    QList< QSharedPointer<CatCtl> > cats_;

    QScopedPointer<Ui::Config>  ui;
    QPointer<HistoryView>       historyView_;

    QPointer<MoveItems>     moveItemsAsk_;
    unsigned                moveItemsSrcSlot_;
    unsigned                moveItemsDstSlot_;
    bool                    moveItemsInsertBefore_;

    QTimer                  ticker_;
    QMenu                   catshopMenu_;
    QMenu                   itemMenu_;
    QAction                *equipAction_;
    QMetaObject::Connection priceLinkConnection_;

    MarketSetupModel        marketModel_;
    QSortFilterProxyModel  *proxyModel_;

    int                     serverIndex_;

    unsigned                money_;
    unsigned                banknotes_;
    unsigned                gold_;

    bool                    hasCatshop_;
    unsigned                catShopEnding_;
    unsigned                catshopMaxSlots_;
    unsigned                catshopMaxChars_;
    QList<GShop::Item>      catshops_;

    bool                    hasUnsavedData_;
    int                     chatBufferActualValue_;
};

#endif
