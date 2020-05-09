#include "stdafx.h"

#include "boost/signal.hpp"

#include "game/inventory.h"
#include "netdata/fragments_giclient.h"
#include "netdata/fragments_client.h"


Inventory::Inventory()
{
    init();
}

Inventory::~Inventory()
{
    unbind();
}

void Inventory::bind(std::shared_ptr<Connection> connection)
{
    connection_ = connection;

    HANDLE_S_GI(cookies_, connection_, Inventory, SelfInventoryDetail);
    HANDLE_S_GI(cookies_, connection_, Inventory, SelfInfo);
    HANDLE_S_GI(cookies_, connection_, Inventory, SelfItemInfo);
    HANDLE_S_GI(cookies_, connection_, Inventory, SelfItemEmptyInfo);

    HANDLE_S_GI(cookies_, connection_, Inventory, PlayerUseItem);
    HANDLE_S_GI(cookies_, connection_, Inventory, ExchangeInventoryItem);
    HANDLE_S_GI(cookies_, connection_, Inventory, ExchangeEquipmentItem);
    HANDLE_S_GI(cookies_, connection_, Inventory, MoveInventoryItem);
    HANDLE_S_GI(cookies_, connection_, Inventory, MoveEquipmentItem);
    HANDLE_S_GI(cookies_, connection_, Inventory, EquipItem);
    HANDLE_S_GI(cookies_, connection_, Inventory, PlayerDropItem);
    HANDLE_S_GI(cookies_, connection_, Inventory, PlayerPickupItem);
    HANDLE_S_GI(cookies_, connection_, Inventory, PlayerObtainItem);

    HANDLE_S_GI(cookies_, connection_, Inventory, ExchangeTrashboxItem);
    HANDLE_S_GI(cookies_, connection_, Inventory, MoveTrashboxItem);
    HANDLE_S_GI(cookies_, connection_, Inventory, ExchangeTrashboxInventory);
    HANDLE_S_GI(cookies_, connection_, Inventory, InventoryItemToTrash);
    HANDLE_S_GI(cookies_, connection_, Inventory, TrashItemToInventory);
    HANDLE_S_GI(cookies_, connection_, Inventory, ItemToMoney);
    HANDLE_S_GI(cookies_, connection_, Inventory, UnlockInventorySlot);

    HANDLE_S_GI(cookies_, connection_, Inventory, ProduceOnce);
    HANDLE_S_GI(cookies_, connection_, Inventory, TaskDeliverItem);

    HANDLE_S_GI(cookies_, connection_, Inventory, GetOwnMoney);
    HANDLE_S_GI(cookies_, connection_, Inventory, SpendMoney);

    HANDLE_S_GI(cookies_, connection_, Inventory, PlayerCash);

    HANDLE_S_GI(cookies_, connection_, Inventory, SelfOpenMarket);
    HANDLE_S_GI(cookies_, connection_, Inventory, PlayerPurchaseItem);
    HANDLE_S_GI(cookies_, connection_, Inventory, TradeAwayItem);

    init();
}

void Inventory::unbind()
{
    if (connection_.get())
    {
        connection_->unbindHandlers(cookies_);
        connection_.reset();
    }
}

void Inventory::init()
{
    bag_.clear();
    bagLocks_.clear();
    equip_.clear();
    trashbox_.clear();
    trashboxRes_.clear();
    market_.clear();
    while (!operationQueue_.empty())
    {
        operationQueue_.pop();
    }
    availability_ = NO_INFO;
    money_ = 0;
    maxMoney_ = 0;
    silver_ = 0;
    myId_ = INVALID_ID;
}

void Inventory::setElements(const elements::ItemListCollection & items)
{
    allItems_ = items;
}

void Inventory::setPropFactory(std::shared_ptr<ItemPropsFactory> propFactory)
{
    propFactory_ = propFactory;
}

const std::vector<InventoryItem> & Inventory::getBag() const
{
    setItemNames(bag_);
    return bag_;
}

const std::vector<InventoryItem> & Inventory::getEquip() const
{
    setItemNames(equip_);
    return equip_;
}

const std::vector<MarketItem> & Inventory::getMarket() const
{
    setItemNames(market_);
    return market_;
}

unsigned Inventory::getMoney(unsigned *maxMoney /*= NULL*/) const
{
    if (maxMoney != NULL)
    {
        *maxMoney = maxMoney_;
    }
    return money_;
}

unsigned Inventory::getBanknotes() const
{
    unsigned banknotes = 0;

    for (size_t i = 0; i < bag_.size(); ++i)
    {
        if ( ! bag_[i].isEmpty()
            && bag_[i].id == BanknoteId)
        {
            banknotes += bag_[i].count;
        }
    }
    return banknotes;
}

unsigned Inventory::getSilver() const
{
    return silver_;
}

void Inventory::equipItem(unsigned inventorySlot, unsigned equipSlot)
{
    if (!connection_
        || inventorySlot >= bag_.size() || equipSlot >= equip_.size())
    {
        assert(0);
        return;
    }

    lockSlot(inventorySlot);
    //todo lockSlot(equipSlot);

    clientdata::FragmentGiInventoryToEquipSwap *f = new clientdata::FragmentGiInventoryToEquipSwap();
    f->inventorySlot = byte(inventorySlot);
    f->equipSlot = byte(equipSlot);

    clientdata::FragmentGameinfoSet fgi(f);
    connection_->send(&fgi);
}

void Inventory::swapItems(unsigned slot1, unsigned slot2)
{
    if (!connection_
        || slot1 >= bag_.size() || slot2 >= bag_.size()
        /*|| bag_.at(slot1).isEmpty() || bag_.at(slot2).isEmpty() такой свап работает*/)
    {
        Log("Attempt to swap slots %i to %i", slot1, slot2);
        assert(0);
        return;
    }

    lockSlot(slot1);
    lockSlot(slot2);

    clientdata::FragmentGiInventoryItemSwap *f = new clientdata::FragmentGiInventoryItemSwap();
    f->slot1 = byte(slot1);
    f->slot2 = byte(slot2);

    clientdata::FragmentGameinfoSet fgi(f);
    connection_->send(&fgi);

    return;
}

void Inventory::moveItems(unsigned srcSlot, unsigned dstSlot, unsigned count)
{
    if (!connection_)
    {
        return;
    }

    if (isLockedSlot(srcSlot) || isLockedSlot(dstSlot))
    {
        operationQueue_.push(std::make_shared<MoveItemsOperation>(this, srcSlot, dstSlot, count));
        Log("Pushed move op. %i to %i of %i", srcSlot, dstSlot, count);
        return;
    }

    if (srcSlot >= bag_.size()
        || dstSlot >= bag_.size()
        || bag_.at(srcSlot).isEmpty())  // empty мы можем проверить только на разлоченном слоте чтобы исключить проверку до операций которые будут совершены (типа swap)
    {
        Log("Attempt to move slots %i to %i", srcSlot, dstSlot);
        assert(0);
        return;
    }

    // swap

    assert(!bag_.at(srcSlot).isEmpty());

    if ( ! bag_.at(dstSlot).isEmpty()
        && bag_.at(srcSlot).id != bag_.at(dstSlot).id)
    {
        swapItems(srcSlot, dstSlot);
        return;
    }

    // move

    unsigned moveCount = 0;
    elements::Item item;
    // в предыдущем условии мы убедились что если Dest слот не пуст, то id src и dst равны
    assert(bag_.at(dstSlot).isEmpty() || (bag_.at(dstSlot).id == bag_.at(srcSlot).id));

    if (allItems_.getItem(bag_.at(srcSlot).id, item))
    {
        unsigned stackDst = item["stack"];
        assert(stackDst >= bag_.at(srcSlot).count);

        if ( ! bag_.at(dstSlot).isEmpty())
        {
            // move to occupied slot
            assert(stackDst >= bag_.at(dstSlot).count);
            if (stackDst < bag_.at(dstSlot).count)
            {
                stackDst = bag_.at(dstSlot).count;
            }
            moveCount = stackDst == 0 ? count : std::min(count, stackDst - bag_.at(dstSlot).count);
        }
        else
        {
            // move to empty slot
            moveCount = stackDst == 0 ? count : std::min(count, stackDst);
        }
    }

    if (moveCount > 0)
    {
        lockSlot(srcSlot);
        lockSlot(dstSlot);

        clientdata::FragmentGiInventoryItemMove *f = new clientdata::FragmentGiInventoryItemMove();
        f->srcSlot = byte(srcSlot);
        f->dstSlot = byte(dstSlot);
        f->count = moveCount;

        clientdata::FragmentGameinfoSet fgi(f);
        connection_->send(&fgi);
    }
}

//

void Inventory::obtainItem(DWORD itemId, InventoryType type, byte slot, unsigned itemCount)
{
    if (itemCount == 0)
    {
        Log("Attempt to add 0 items to slot %i", slot);
        return;
    }

    Log("Adding item id %i to storage %i slot %i (%i)", itemId, type, slot, itemCount);

    // Алгоритм добавления предметов:
    // проходимся по всем таким же итемам в инвентаре и добавляем всем до стека. Остаток складывается в 
    // слот, на который указыавет f->items[i].slot. Процесс останавливается когда все итемы сложены 
    // (в этом случае slot указывает на последний дополненный) либо все существующие слоты не будут
    // дополнены до стека (В этом случае остаток добавляется в слот на который указывает slot).


    std::vector<InventoryItem>  *storage = getStorage(type);
    if (storage == NULL)
    {
        return;
    }

    if (slot >= storage->size())
    {
        Log("Incorrect slot number %i (have %i)", slot, storage->size());
        return;
    }

    InventoryItem & slotItem = (*storage)[slot];

    try
    {
        elements::Item elItem;

        if (itemCount > 1
            && allItems_.getItem(itemId, elItem))
        {
            int stack = elItem["Stack"];
            if (stack > 1)
            {
                Log("Overcounting item %i to slot %i (%i total)", itemId, slot, itemCount);
                for (size_t i = 0; i < storage->size(); i++)
                {
                    InventoryItem & cItem = (*storage)[i];

                    if ( ! cItem.isEmpty() 
                        && cItem.id == itemId
                        && int(cItem.count) < stack)
                    {
                        WORD incement = WORD(std::min(itemCount, stack - cItem.count));
                        cItem.increase(incement);
                        itemCount -= incement;
                        Log("added partially to slot %i (%i); %i left", i, incement, itemCount);

                        if (itemCount == 0)
                        {
                            assert(slot == i);  // last slot in auto-placement
                            break;
                        }
                    }
                }
            }
            else
            {
                Log("No stack value for item id %i", itemId);
            }
        }
    }
    catch (elements::ItemValueNotFound * /*e*/)
    {
        Log("Item has no Stack field (id %i)", itemId);
    }

    if (itemCount == 0)
    {
        return;
    }

    if ( ! slotItem.isEmpty())
    {
        // adding items to the slot
        if (slotItem.id != itemId)
        {
            Log("Item id mismatch of slot %i:  %08X != %08X", slot, slotItem.id, itemId);
            slotItem.id = itemId;
        }

        slotItem.increase(itemCount);
    }
    else
    {
        // adding new
        slotItem = InventoryItem(itemId, itemCount);
    }
}

void Inventory::obtainItem(DWORD itemId, byte slot, WORD itemCount, unsigned totalInSlot)
{
    obtainItem(itemId, InventoryTypeBag, slot, itemCount, totalInSlot);
}

void Inventory::obtainItem(DWORD itemId, InventoryType type, byte slot, unsigned itemCount, unsigned totalInSlot)
{
    obtainItem(itemId, type, slot, itemCount);

    std::vector<InventoryItem>  *storage = getStorage(type);
    if (storage == NULL)
    {
        return;
    }

    InventoryItem & i = (*storage)[slot];

    if (i.count != totalInSlot)
    {
        Log("Resulting item count mismatch of slot %i:  %i != %i", slot, i.count, totalInSlot);
        i.count = totalInSlot;
    }
}

void Inventory::loseItem(DWORD itemId, InventoryType type, byte slot, unsigned itemCount)
{
    if (itemCount == 0)
    {
        Log("Attempt to remove 0 items to slot %i", slot);
        return;
    }

    Log("Removing item id %i from storage %i slot %i (%i)", itemId, type, slot, itemCount);

    std::vector<InventoryItem> *storage = getStorage(type);
    if (storage == NULL)
    {
        return;
    }

    if (slot >= storage->size())
    {
        Log("Incorrect slot number %i (have %i)", slot, storage->size());
        return;
    }

    if ((*storage)[slot].isEmpty())
    {
        Log("Attempt to remove item from empty slot %i", slot);
        return;
    }

    if ((*storage)[slot].count < itemCount || (*storage)[slot].id != itemId)
    {
        Log("Slot data mismatch: %i: %08X/%i != %08X/%i", slot, (*storage)[slot].id, (*storage)[slot].count, itemId, itemCount);
        return;
    }

    (*storage)[slot].decrease(itemCount);
}
    
void Inventory::getItemInfo(InventoryType type, unsigned slot)
{
    if ( ! connection_) return;

    clientdata::FragmentGiGetSelfItemInfo *f = new clientdata::FragmentGiGetSelfItemInfo();
    f->inventoryType = type;
    f->slot = byte(slot);

    clientdata::FragmentGameinfoSet fgi(f);
    connection_->send(&fgi);
}

std::vector<InventoryItem> * Inventory::getStorage(InventoryType type)
{
    switch(type)
    {
        case InventoryTypeBag:
            return & bag_;
        case InventoryTypeEquip:
            return & equip_;
        case InventoryTypeTrashbox:
            return & trashbox_;
        case InventoryTypeTrashboxRes:
            return & trashboxRes_;
        default:
            Log("Error: incorrect storage type %i", type);
            return NULL;
    }
}

InventoryStorageType Inventory::getStorageType(InventoryType type)
{
    switch(type)
    {
        case InventoryTypeBag:
            return BAG_INFO;
        case InventoryTypeEquip:
            return EQUIPMENT_INFO;
        case InventoryTypeTrashbox:
            return TRASHBOX_INFO;
        case InventoryTypeTrashboxRes:
            return TRASHBOX_INFO;
        default:
            Log("Error: incorrect storage type %i", type);
            return NO_INFO;
    }
}

//

void Inventory::onSelfInfo(const serverdata::FragmentGiSelfInfo *f)
{
    if (myId_ != INVALID_ID)
    {
        assert(myId_ == f->id);
    }
    else
    {
        myId_ = f->id;
    }
}

void Inventory::onSelfInventoryDetail(const serverdata::FragmentGiSelfInventoryDetail *f)
{
    std::vector<InventoryItem> *storage = getStorage(f->type);
    if (storage == NULL)
    {
        return;
    }

    storage->clear();
    storage->resize(f->cells);

    InventoryStorageType sType = getStorageType(f->type);
    if (sType == BAG_INFO)
    {
        bagLocks_.resize(f->cells);
    }

    for (std::vector<struct serverdata::FragmentGiSelfInventoryDetail::Item>::const_iterator it = f->items.begin();
        it != f->items.end(); ++it)
    {
        InventoryItem & i = (*storage)[it->slot];
        i.count     = it->count;
        i.id        = it->id;
        i.timeLimit = it->timeLimit;
        if ( ! it->attrs.empty() && propFactory_)
        {
            i.props.reset(propFactory_->create(i.id, it->attrs));
        }
    }

    availability_ |= sType;

    InventoryUpdateEvent e(sType);
    sEvent(&e);
}

void Inventory::onSelfItemInfo(const serverdata::FragmentGiSelfItemInfo *f)
{
    std::vector<InventoryItem> *storage = getStorage(f->inventoryType);
    if (storage == NULL
        || f->slot > storage->size())
    {
        return;
    }

    InventoryItem & i = (*storage)[f->slot];
    assert(i.id == INVALID_ID || i.id == f->itemId);
    i.count     = f->count;
    i.id        = f->itemId;
    i.timeLimit = f->timeLimit;
    if ( ! f->attrs.empty() && propFactory_)
    {
        i.props.reset(propFactory_->create(i.id, f->attrs));
    }

    InventoryStorageType sType = getStorageType(f->inventoryType);
    InventoryUpdateEvent e(sType);
    sEvent(&e);
}

void Inventory::onSelfItemEmptyInfo(const serverdata::FragmentGiSelfItemEmptyInfo *f)
{
    std::vector<InventoryItem> *storage = getStorage(f->inventoryType);
    if (storage == NULL
        || f->slot > storage->size())
    {
        return;
    }

    InventoryItem & i = (*storage)[f->slot];
    assert(i.isEmpty());
    i.id = InventoryItem::INVALID_ID;
    i.count = 0;
}

void Inventory::onPlayerUseItem(const serverdata::FragmentGiPlayerUseItem *f)
{
    // Bag only item usage
    if (f->type != InventoryTypeBag)
    {
        // assert(0); player uses equip also like wings
        return;
    }

    loseItem(f->itemId, f->type, f->slot, f->count);

    InventoryUpdateEvent e(getStorageType(f->type));
    sEvent(&e);
}

void Inventory::onExchangeInventoryItem(const serverdata::FragmentGiExchangeInventoryItem *f)
{
    if (f->from >= bag_.size() || f->to >= bag_.size())
    {
        Log("Incorrect bag slots: from %i to %i (have %i)", f->from, f->to,  bag_.size());
        return;
    }

    std::swap(bag_[f->from], bag_[f->to]);

    InventoryUpdateEvent e(BAG_INFO);
    sEvent(&e);
}

void Inventory::onExchangeEquipmentItem(const serverdata::FragmentGiExchangeEquipmentItem *f)
{
    if (f->from >= equip_.size() || f->to >= equip_.size())
    {
        Log("Incorrect equip slots: from %i to %i (have %i)", f->from, f->to,  bag_.size());
        return;
    }

    std::swap(equip_[f->from], equip_[f->to]);

    InventoryUpdateEvent e(EQUIPMENT_INFO);
    sEvent(&e);
}

void Inventory::onMoveInventoryItem(const serverdata::FragmentGiMoveInventoryItem *f)
{
    if (f->amount == 0)
    {
        Log("Attempt to move 0 items from slot %i to slot %i", f->from, f->to);
        return;
    }

    if (f->from >= bag_.size() || f->to >= bag_.size())
    {
        Log("Incorrect slots: from %i to %i (have %i)", f->from, f->to,  bag_.size());
        return;
    }

    if (bag_[f->from].isEmpty())
    {
        Log("Attempt to move item from empty slot %i to %i", f->from, f->to);
        return;
    }

    if (bag_[f->to].isEmpty())
    {
        bag_[f->to] = bag_[f->from];
        bag_[f->to].count = f->amount;
    }
    else
    {
        if (bag_[f->from].id != bag_[f->to].id)
        {
            Log("Attempt to move item from different slots %i(%08X) to %i(%08X)", f->from, bag_[f->from].id, f->to, bag_[f->to].id);
            return;
        }

        bag_[f->to].increase(f->amount);
    }

    bag_[f->from].decrease(f->amount);

    InventoryUpdateEvent e(BAG_INFO);
    sEvent(&e);
}

void Inventory::onMoveEquipmentItem(const serverdata::FragmentGiMoveEquipmentItem *f)
{
    if (f->amount == 0)
    {
        Log("Attempt to move 0 items from bag slot %i to equip slot %i", f->bagSlot, f->equipSlot);
        return;
    }

    if (f->bagSlot >= bag_.size() || f->equipSlot >= equip_.size())
    {
        Log("Incorrect slots: from %i (have %i) to %i (have %i)", f->bagSlot, bag_.size(), f->equipSlot, equip_.size());
        return;
    }

    if (bag_[f->bagSlot].isEmpty())
    {
        Log("Attempt to move item from empty slot %i to %i", f->bagSlot, f->equipSlot);
        return;
    }

    if (equip_[f->equipSlot].isEmpty())
    {
        equip_[f->equipSlot] = bag_[f->bagSlot];
        equip_[f->equipSlot].count = f->amount;
    }
    else
    {
        if (bag_[f->bagSlot].id != equip_[f->equipSlot].id)
        {
            Log("Attempt to move item from different slots %i(%08X) to %i(%08X)", f->bagSlot, bag_[f->bagSlot].id, f->equipSlot, bag_[f->equipSlot].id);
            return;
        }

        equip_[f->equipSlot].increase(f->amount);
    }

    bag_[f->bagSlot].decrease(f->amount);

    InventoryUpdateEvent e(BAG_INFO | EQUIPMENT_INFO);
    sEvent(&e);
}

void Inventory::onEquipItem(const serverdata::FragmentGiEquipItem *f)
{
    if (f->bagSlot >= bag_.size() || f->equipSlot >= equip_.size())
    {
        Log("Incorrect slots: bag %i (have %i)  equip %i (have %i)", f->bagSlot, bag_.size(), f->equipSlot, equip_.size());
        return;
    }

    std::swap(bag_[f->bagSlot], equip_[f->equipSlot]);
    bag_[f->bagSlot].setCount(f->bagAmount);
    equip_[f->equipSlot].setCount(f->equipAmount);

    InventoryUpdateEvent e(BAG_INFO | EQUIPMENT_INFO);
    sEvent(&e);
}

void Inventory::onUnlockInventorySlot(const serverdata::FragmentGiUnlockInventorySlot * f)
{
    if (f->inventoryType != InventoryTypeBag)
    {
        return;
    }

    if (f->slot >= bag_.size())
    {
        Log("Incorrect slot: %i (have %i)", f->slot, bag_.size());
        return;
    }
    unlockSlot(f->slot);
    executeQueuedLockedOps();
}

void Inventory::onPlayerDropItem(const serverdata::FragmentGiPlayerDropItem *f)
{
    loseItem(f->itemId, f->type, f->slot, f->amount);
    DropItemEvent ev(getStorageType(f->type), f->slot, f->amount, f->itemId);
    sEvent(&ev);
    InventoryUpdateEvent e(getStorageType(f->type));
    sEvent(&e);
}

void Inventory::onPlayerPickupItem(const serverdata::FragmentGiPlayerPickupItem *f)
{
    obtainItem(f->itemId, f->type, f->slot, f->amount, f->totalInSlot);
    InventoryUpdateEvent e(getStorageType(f->type));
    sEvent(&e);
}

void Inventory::onPlayerObtainItem(const serverdata::FragmentGiPlayerObtainItem *f)
{
    obtainItem(f->itemId, f->type, f->slot, f->amount, f->totalInSlot);
    getItemInfo(f->type, f->slot);
    ObtainItemEvent ev(getStorageType(f->type), f->slot, f->amount, f->itemId);
    sEvent(&ev);
    InventoryUpdateEvent e(getStorageType(f->type));
    sEvent(&e);
}

void Inventory::onExchangeTrashboxItem(const serverdata::FragmentGiExchangeTrashboxItem *f)
{
    std::vector<InventoryItem>  *trashbox = getStorage(f->type);

    if (f->from >= trashbox->size() || f->to >= trashbox->size())
    {
        Log("Incorrect slots: trashbox %i to %i (have %i)", f->from, f->to, trashbox->size());
        return;
    }

    std::swap((*trashbox)[f->from], (*trashbox)[f->to]);

    InventoryUpdateEvent e(getStorageType(f->type));
    sEvent(&e);
}

void Inventory::onMoveTrashboxItem(const serverdata::FragmentGiMoveTrashboxItem *f)
{
    if (f->amount == 0)
    {
        Log("Attempt to move 0 items");
        return;
    }

    std::vector<InventoryItem>  *trashbox = getStorage(f->type);

    if (f->from >= trashbox->size() || f->to >= trashbox->size())
    {
        Log("Incorrect slots: trashbox %i to %i (have %i)", f->from, f->to, trashbox->size());
        return;
    }

    if ((*trashbox)[f->from].isEmpty())
    {
        Log("Attempt to move item from empty slot %i to %i", f->from, f->to);
        return;
    }

    if ((*trashbox)[f->to].isEmpty())
    {
        (*trashbox)[f->to] = (*trashbox)[f->from];
        (*trashbox)[f->to].count = f->amount;
    }
    else
    {
        if ((*trashbox)[f->to].id != (*trashbox)[f->from].id)
        {
            Log("Attempt to move item from different slots %i(%08X) to %i(%08X)", f->from, (*trashbox)[f->from].id, f->to, (*trashbox)[f->to].id);
            return;
        }

        (*trashbox)[f->to].increase(f->amount);
    }

    (*trashbox)[f->from].decrease(f->amount);

    InventoryUpdateEvent e(getStorageType(f->type));
    sEvent(&e);
}

void Inventory::onExchangeTrashboxInventory(const serverdata::FragmentGiExchangeTrashboxInventory *f)
{
    std::vector<InventoryItem>  *trashbox = getStorage(f->type);

    if (f->trashboxSlot >= trashbox->size() || f->bagSlot >= bag_.size())
    {
        Log("Incorrect slots: trashbox %i (have %i) to bag %i (have %i)", f->trashboxSlot, trashbox->size(), f->bagSlot, bag_.size());
        return;
    }

    std::swap((*trashbox)[f->trashboxSlot], bag_[f->bagSlot]);
    InventoryUpdateEvent e(getStorageType(f->type));
    sEvent(&e);
}

void Inventory::onInventoryItemToTrash(const serverdata::FragmentGiInventoryItemToTrash *f)
{
    if (f->amount == 0)
    {
        Log("Attempt to move 0 items");
        return;
    }

    std::vector<InventoryItem>  *trashbox = getStorage(f->type);

    if (f->trashboxSlot >= trashbox->size() || f->bagSlot >= bag_.size())
    {
        Log("Incorrect slots: bag %i (have %i) to trashbox %i (have %i)", f->bagSlot, bag_.size(), f->trashboxSlot, trashbox->size());
        return;
    }

    if (bag_[f->bagSlot].isEmpty())
    {
        Log("Attempt to move item from empty slot %i", f->bagSlot);
        return;
    }

    if ((*trashbox)[f->trashboxSlot].isEmpty())
    {
        (*trashbox)[f->trashboxSlot] = bag_[f->bagSlot];
        (*trashbox)[f->trashboxSlot].count = f->amount;
    }
    else
    {
        if ((*trashbox)[f->trashboxSlot].id != bag_[f->bagSlot].id)
        {
            Log("Attempt to move item from different slots %i(%08X) to %i(%08X)", f->bagSlot, bag_[f->bagSlot].id, f->trashboxSlot, (*trashbox)[f->trashboxSlot].id);
            return;
        }

        (*trashbox)[f->trashboxSlot].increase(f->amount);
    }

    bag_[f->bagSlot].decrease(f->amount);

    InventoryUpdateEvent e(getStorageType(f->type) | BAG_INFO);
    sEvent(&e);
}

void Inventory::onTrashItemToInventory(const serverdata::FragmentGiTrashItemToInventory *f)
{
    if (f->amount == 0)
    {
        Log("Attempt to move 0 items");
        return;
    }

    std::vector<InventoryItem>  *trashbox = getStorage(f->type);

    if (f->trashboxSlot >= trashbox->size() || f->bagSlot >= bag_.size())
    {
        Log("Incorrect slots: trashbox %i (have %i) to bag %i (have %i)", f->trashboxSlot, trashbox->size(), f->bagSlot, bag_.size());
        return;
    }

    if ((*trashbox)[f->trashboxSlot].isEmpty())
    {
        Log("Attempt to move item from empty slot %i", f->bagSlot);
        return;
    }

    if (bag_[f->bagSlot].isEmpty())
    {
        bag_[f->bagSlot] = (*trashbox)[f->trashboxSlot];
        bag_[f->bagSlot].count = f->amount;
    }
    else
    {
        if ((*trashbox)[f->trashboxSlot].id != bag_[f->bagSlot].id)
        {
            Log("Attempt to move item from different slots %i(%08X) to %i(%08X)", f->trashboxSlot, (*trashbox)[f->trashboxSlot].id, f->bagSlot, bag_[f->bagSlot].id);
            return;
        }

        bag_[f->bagSlot].increase(f->amount);
    }

    (*trashbox)[f->trashboxSlot].decrease(f->amount);

    InventoryUpdateEvent e(getStorageType(f->type) | BAG_INFO);
    sEvent(&e);
}

void Inventory::onItemToMoney(const serverdata::FragmentGiItemToMoney *f)
{
    loseItem(f->itemId, InventoryTypeBag, byte(f->slot), WORD(f->amount));

    if ((availability_ & MONEY_INFO) != 0)
    {
        money_ += f->money;
    }

    InventoryUpdateEvent e(BAG_INFO);
    sEvent(&e);
}

void Inventory::onProduceOnce(const serverdata::FragmentGiProduceOnce *f)
{
    obtainItem(f->itemId, f->type, f->slot, f->amount, f->totalInSlot);

    InventoryUpdateEvent e(getStorageType(f->type));
    sEvent(&e);
}

void Inventory::onTaskDeliverItem(const serverdata::FragmentGiTaskDeliverItem *f)
{
    obtainItem(f->itemId, f->type, f->slot, f->amount, f->totalInSlot);

    InventoryUpdateEvent e(getStorageType(f->type));
    sEvent(&e);
}

void Inventory::onGetOwnMoney(const serverdata::FragmentGiGetOwnMoney *f)
{
    money_ = f->money;
    maxMoney_ = f->maxMoney;

    availability_ |= MONEY_INFO;
    InventoryUpdateEvent e(MONEY_INFO);
    sEvent(&e);
}

void Inventory::onSpendMoney(const serverdata::FragmentGiSpendMoney *f)
{
    money_ -= f->money;

    InventoryUpdateEvent e(MONEY_INFO);
    sEvent(&e);
}

void Inventory::onPlayerCash(const serverdata::FragmentGiPlayerCash *f)
{
    silver_ = f->count;

    InventoryUpdateEvent e(GOLD_INFO);
    sEvent(&e);
}

void Inventory::onPlayerPurchaseItem(const serverdata::FragmentGiPlayerPurchaseItem *f)
{
    // market trade doesn't notifies money update when we buy an item into cat

    assert((availability_ & BAG_INFO) != 0);

    if ((availability_ & MONEY_INFO) != 0)
    {
        money_ -= f->moneyLost;
    }

    for (size_t i = 0; i < f->items.size(); ++i)
    {
        obtainItem(f->items[i].id, InventoryTypeBag, byte(f->items[i].slot), f->items[i].count);

        unsigned price = 0;

        // get price from market items
        // note: при покупке у нпс содержание не отличается, сложно отличить продажу в кота от покупки нпс

        size_t sIdx = f->items[i].tradeSlot;
        if (market_.size() > sIdx)
        {
            price = market_[sIdx].price;

            if (market_[sIdx].count >= f->items[i].count)
            {
                market_[sIdx].count -= f->items[i].count;
                if (market_[sIdx].count == 0)
                {
                    Log("Sold all items at market slot %i", sIdx);
                }
            }
            else
            {
                Log("Error sold item count exceeds market slot count at slot %i", sIdx);
                market_[sIdx].count = 0;
            }
        }
        else
        {
            Log("Error No market slot at %i", sIdx);
        }

        ItemPurchasedEvent e(f->items[i].id, f->items[i].count, price, f->items[i].tradeSlot);
        sEvent(&e);
    }

    InventoryUpdateEvent e(BAG_INFO | MARKET_INFO | MONEY_INFO);
    sEvent(&e);
}

void Inventory::onTradeAwayItem(const serverdata::FragmentGiTradeAwayItem *f)
{
    loseItem(f->itemId, InventoryTypeBag, byte(f->slot), WORD(f->count));

    // adjust market items
    unsigned price = 0;
    for (size_t i = 0; i < market_.size(); i++)
    {
        if (market_[i].id == f->itemId
            && market_[i].slot == f->slot)
        {
            price = market_[i].price;

            if (market_[i].count >= f->count)
            {
                market_[i].count -= f->count;
                if (market_[i].count == 0)
                {
                    Log("Trade all items at slot %i", i);
                }
            }
            else
            {
                Log("Error trade item count exceeds slot count at slot %i", i);
                market_[i].count = 0;
            }
        }
    }

    ItemSoldEvent es(f->itemId, f->count, price, f->buyerId);
    sEvent(&es);

    InventoryUpdateEvent iu(BAG_INFO | MARKET_INFO);
    sEvent(&iu);
}

void Inventory::onSelfOpenMarket(const serverdata::FragmentGiSelfOpenMarket *f)
{
    assert((availability_ & BAG_INFO) != 0);

    market_.clear();

    for (std::vector<struct serverdata::FragmentGiSelfOpenMarket::Item>::const_iterator it = f->items.begin();
        it != f->items.end(); ++it)
    {
        MarketItem i;
        i.count  = it->count;
        i.price  = it->price;
        i.id     = it->id;
        i.slot   = (signed short)it->slot;            // slot < 0 is buying item

        market_.push_back(i);
    }

    availability_ |= MARKET_INFO;

    InventoryUpdateEvent e(MARKET_INFO);
    sEvent(&e);
}

void Inventory::onCancelMarket(const serverdata::FragmentGiPlayerCancelMarket *f)
{
    assert(myId_ != INVALID_ID);

    if (f->id == myId_)
    {
        Log("Market closed");

        market_.clear();

        availability_ &= ~MARKET_INFO;
    }
}

void Inventory::executeQueuedLockedOps()
{
    if (operationQueue_.empty())
    {
        return;
    }
    if (!operationQueue_.front()->isLocked())
    {
        Log("Ran queued op");
        operationQueue_.front()->execute();
        operationQueue_.pop();
    }
}
