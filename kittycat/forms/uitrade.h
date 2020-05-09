#ifndef uitrade_h_
#define uitrade_h_

#include <QList>
#include <QDialog>
#include <QTableWidgetItem>
#include <QTimer>

#include "qlib/game/gamebase.h"
#include "game/data/items.h"

namespace Ui
{
    class Trade;
}

class CatCtl;

class TradeView
    : public QDialog
{
    Q_OBJECT
public:
    TradeView(CatCtl * ctl, const QString & playerName, QWidget *parent = 0);
    ~TradeView();

private slots:
    void on_btnAddOne_clicked();
    void on_btnAddAll_clicked();
    void on_btnAddAllExOne_clicked();
    void on_btnRemoveOne_clicked();
    void on_btnRemoveAll_clicked();
    void on_btnSetMoney_clicked();
    void on_btnSetMax_clicked();
    void on_btnConfirm_clicked();
    void on_btnAccept_clicked();
    void on_twInventory_itemDoubleClicked(QTableWidgetItem * item);
    void on_twInventory_itemSelectionChanged();
    void on_twMyItems_itemDoubleClicked(QTableWidgetItem * item);
    void on_twMyItems_itemSelectionChanged();
    void on_twMyItems_itemChanged(QTableWidgetItem * item);

    void onGameEvent(const qlib::GameEvent &);
    void onErrorTimer();

private:
    struct InventoryItemEx
        : public InventoryItem
    {
        InventoryItemEx(unsigned slot)
            : inventorySlot(slot)
        {}
        InventoryItemEx(const InventoryItem & item, unsigned slot)
            : InventoryItem(item)
            , inventorySlot(slot)
        {}

        unsigned inventorySlot;
    };

    void disableRemoveButtons();
    void disableAddButtons();
    void enableRemoveButtons();
    void enableAddButtons();
    void disableButtons();
    void enableButtons();

    void addTableItem(QTableWidget *table, const InventoryItem &item, int row = -1);
    void enableTableItem(int row);
    void disableTableItem(int row);
    bool hasEnabledItems(QList<QTableWidgetItem *> selection);
    int  findItem(const QList<InventoryItemEx> & list, unsigned itemId, unsigned inventorySlot);

    void addItemToTrade(int row, int count = -1 /* all */);         // -2 is all except one
    void addSelectedItemsToTrade(int count);                        // -1 is all; -2 is all except one
    void removeItemFromTrade(int row, int count = -1 /* all */);
    void removeSelectedItemsFromTrade(int count);                   // -1 is all

    void itemAddedToTrade(unsigned inventorySlot, DWORD itemId, const barray & attrs, unsigned count);
    void itemRemovedFromTrade(unsigned inventorySlot, DWORD itemId, const barray & attrs, unsigned count);
    void itemAddedToPartnerTrade(unsigned inventorySlot, DWORD itemId, const barray & attrs, unsigned count);
    void itemRemovedFromPartnerTrade(unsigned inventorySlot, DWORD itemId, const barray & attrs, unsigned count);
    void moneyAddedToTrade(unsigned money);
    void moneyAddedToPartnerTrade(unsigned money);
    void moneyRemovedFromTrade(unsigned money);
    void moneyRemovedFromPartnerTrade(unsigned money);

    void showError(const QString & msg);
    void setConfirm(bool isOn);
    void setAccept(bool isOn);
    void setPartnerConfirm(bool isOn);
    void setPartnerAccept(bool isOn);
    void lockInput(bool isOn);
    void initLockStage();

    QScopedPointer<Ui::Trade>   ui_;
    QTimer                      errorTimer_;
    QIntValidator               moneyValidator_;

    CatCtl  *ctl_;
    QList<InventoryItemEx>  inventoryItems_;
    unsigned                money_;
    QList<InventoryItemEx>  myItems_;
    unsigned                myMoney_;
    QList<InventoryItemEx>  partnerItems_;
    unsigned                partnerMoney_;
    bool                    isMyAccepted_;
    bool                    isPartnerAccepted_;
};


#endif