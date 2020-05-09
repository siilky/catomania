#ifndef inventory_h
#define inventory_h

#pragma warning(disable : 4996)     // unsafe function calls

#include <queue>
#include <cstdio>

#include "types.h"
#include "inventory_types.h"
#include "netdata/content.h"
#include "netdata/connection.h"
#include "netdata/fragments_giserver.h"
#include "data/elements.h"
#include "event.h"
#include "data/itemprops.h"
#include "data/items.h"
#include <corecrt_wstdio.h>

class Inventory
{
public:
    static const DWORD INVALID_ID = (DWORD)-1;
    enum
    {
        BanknoteId = 21652,
    };

    Inventory();
    ~Inventory();

    void bind(std::shared_ptr<Connection> connection);
    void unbind();
    void init();

    void setElements(const elements::ItemListCollection & items);
    void setPropFactory(std::shared_ptr<ItemPropsFactory> propFactory);

    // API

    InventoryStorageType getAvailability() const
    {
        return (InventoryStorageType)availability_;
    }

    const std::vector<InventoryItem> & getBag() const;
    const std::vector<InventoryItem> & getEquip() const;
    const std::vector<MarketItem>    & getMarket() const;

    unsigned getMoney(unsigned *maxMoney = NULL) const;
    unsigned getBanknotes() const;
    unsigned getSilver() const;

    // actions

    void equipItem(unsigned inventorySlot, unsigned equipSlot);

    void swapItems(unsigned slot1, unsigned slot2);
    void moveItems(unsigned srcSlot, unsigned dstSlot, unsigned count);

    // Signals

    boost::signal<void(BaseEvent*)>     sEvent;

private:
    void obtainItem(DWORD itemId, byte slot, WORD itemCount, unsigned itemsInSlot);
    void obtainItem(DWORD itemId, InventoryType type, byte slot, unsigned itemCount);
    void obtainItem(DWORD itemId, InventoryType type, byte slot, unsigned itemCount, unsigned totalInSlot);
    void loseItem(DWORD itemId, InventoryType type, byte slot, unsigned itemCount);

    void getItemInfo(InventoryType type, unsigned slot);

    std::vector<Inventory::InventoryItem> * getStorage(InventoryType type);
    Inventory::InventoryStorageType getStorageType(InventoryType type);

    void onSelfInfo(const serverdata::FragmentGiSelfInfo *f);
    void onSelfInventoryDetail(const serverdata::FragmentGiSelfInventoryDetail *f);
    void onSelfItemInfo(const serverdata::FragmentGiSelfItemInfo *f);
    void onSelfItemEmptyInfo(const serverdata::FragmentGiSelfItemEmptyInfo *f);

    void onPlayerUseItem(const serverdata::FragmentGiPlayerUseItem *f);
    void onExchangeInventoryItem(const serverdata::FragmentGiExchangeInventoryItem *f);
    void onExchangeEquipmentItem(const serverdata::FragmentGiExchangeEquipmentItem *f);
    void onMoveInventoryItem(const serverdata::FragmentGiMoveInventoryItem *f);
    void onMoveEquipmentItem(const serverdata::FragmentGiMoveEquipmentItem *f);
    void onEquipItem(const serverdata::FragmentGiEquipItem *f);
    void onUnlockInventorySlot(const serverdata::FragmentGiUnlockInventorySlot *f);

    void onExchangeTrashboxItem(const serverdata::FragmentGiExchangeTrashboxItem *f);
    void onMoveTrashboxItem(const serverdata::FragmentGiMoveTrashboxItem *f);
    void onExchangeTrashboxInventory(const serverdata::FragmentGiExchangeTrashboxInventory *f);
    void onInventoryItemToTrash(const serverdata::FragmentGiInventoryItemToTrash *f);
    void onTrashItemToInventory(const serverdata::FragmentGiTrashItemToInventory *f);

    void onPlayerDropItem(const serverdata::FragmentGiPlayerDropItem *f);
    void onPlayerPickupItem(const serverdata::FragmentGiPlayerPickupItem *f);
    void onPlayerObtainItem(const serverdata::FragmentGiPlayerObtainItem *f);
    void onItemToMoney(const serverdata::FragmentGiItemToMoney *f);

    void onProduceOnce(const serverdata::FragmentGiProduceOnce *f);
    void onTaskDeliverItem(const serverdata::FragmentGiTaskDeliverItem *f);

    void onGetOwnMoney(const serverdata::FragmentGiGetOwnMoney *f);
    void onSpendMoney(const serverdata::FragmentGiSpendMoney *f);

    void onPlayerCash(const serverdata::FragmentGiPlayerCash *f);

    void onPlayerPurchaseItem(const serverdata::FragmentGiPlayerPurchaseItem *f);
    void onTradeAwayItem(const serverdata::FragmentGiTradeAwayItem *f);
    void onSelfOpenMarket(const serverdata::FragmentGiSelfOpenMarket *f);
    void onCancelMarket(const serverdata::FragmentGiPlayerCancelMarket *f);

    template<class InventoryItem>
    void setItemNames(std::vector<InventoryItem> & containter) const
    {
        // apply names
        for (size_t i = 0; i < containter.size(); ++i)
        {
            if (containter[i].name.empty())
            {
                elements::Item item;
                if (allItems_.getItem(containter[i].id, item))
                {
                    containter[i].name = item["Name"];
                    containter[i].icon = item["file_icon"];
                }
                else
                {
                    containter[i].name = std::to_wstring(containter[i].id);
                }
            }
        }
    }

    void executeQueuedLockedOps();

    void lockSlot(unsigned slot)
    {
        assert(slot < bagLocks_.size());
        ++bagLocks_[slot];
    }

    void unlockSlot(unsigned slot)
    {
        assert(slot < bagLocks_.size());
        --bagLocks_[slot];
        if (bagLocks_.at(slot) < 0)
        {
            bagLocks_[slot] = 0;
        }
    }

    bool isLockedSlot(unsigned slot) const
    {
        assert(slot < bagLocks_.size());
        return bagLocks_[slot] > 0;
    }

    class Operation
    {
    public:
        virtual ~Operation() {}

        virtual bool isLocked() const = 0;
        virtual void execute() = 0;
    };

    class MoveItemsOperation : public Operation
    {
    public:
        MoveItemsOperation(Inventory * inventory, unsigned srcSlot, unsigned dstSlot, unsigned count)
            : inventory_(inventory)
            , srcSlot_(srcSlot)
            , dstSlot_(dstSlot)
            , count_(count)
        {
            assert(inventory_ != 0);
        }
        virtual ~MoveItemsOperation() {}

        bool isLocked() const
        {
            return inventory_->isLockedSlot(srcSlot_)
                || inventory_->isLockedSlot(dstSlot_);
        }

        void execute()
        {
            inventory_->moveItems(srcSlot_, dstSlot_, count_);
        }

    private:
        Inventory * inventory_;
        unsigned srcSlot_;
        unsigned dstSlot_;
        unsigned count_;
    };

    std::shared_ptr<Connection>   connection_;
    std::vector<Connection::Cookie> cookies_;

    elements::ItemListCollection    allItems_;
    std::shared_ptr<ItemPropsFactory> propFactory_;

    DWORD                               myId_;
    mutable std::vector<InventoryItem>  bag_;
    std::vector<int>                    bagLocks_;
    mutable std::vector<InventoryItem>  equip_;
    mutable std::vector<InventoryItem>  trashbox_;      // bank
    mutable std::vector<InventoryItem>  trashboxRes_;
    mutable std::vector<MarketItem>     market_;

    std::queue<std::shared_ptr<Operation>>    operationQueue_;

    unsigned                money_;
    unsigned                maxMoney_;

    unsigned                silver_;

    int                     availability_;
};


#endif