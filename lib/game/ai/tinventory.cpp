#include "stdafx.h"

#include "boost/signal.hpp"

#include "game/ai/task.h"
#include "game/ai/tinventory.h"
#include "game/inventory.h"

#define FTRACE() Log("*");
//#define FTRACE()


TUseItem::TUseItem(std::shared_ptr<Game> game, GetUsableItemFn getUsableItem)
    : game_(game)
    , getUsableItem_(getUsableItem)
    , itemId(Inventory::INVALID_ID)
    , inventoryType(InventoryTypeBag)
{
}

ITask::State TUseItem::onStarted()
{
    FTRACE();

    itemId = getUsableItem_(slot, inventoryType);

    if (itemId != Inventory::INVALID_ID)
    {
        if (cPlayerUseItem_.empty())
        {
            cPlayerUseItem_ = game_->getConnection()->bindServerHandler
                (std::function<void (const serverdata::FragmentGiPlayerUseItem *)>
                (std::bind(&TUseItem::onPlayerUseItem, this, _1)));
        }
    }
    else
    {
        Log("No usable items");
        return TASK_FAILED;
    }

    clientdata::FragmentGiUseItem *fUseItem = new clientdata::FragmentGiUseItem();
    fUseItem->type = inventoryType;
    fUseItem->itemId = itemId;
    fUseItem->slot = byte(slot);
    fUseItem->count = 1;
    fUseItem->unk2 = 0;

    clientdata::FragmentGameinfoSet fgi(fUseItem);

    if (game_->getConnection()->send(&fgi))
    {
        return TASK_RUNNING;
    }
    else
    {
        return TASK_FAILED;
    }
}

void TUseItem::onStopped()
{
    FTRACE();

    itemId = Inventory::INVALID_ID;
    game_->getConnection()->unbindHandler(cPlayerUseItem_);
}

void TUseItem::onPlayerUseItem(const serverdata::FragmentGiPlayerUseItem *f)
{
    //FTRACE();
    Log("%i/%i/%i", f->itemId, f->slot, f->type);

    if (state_ != TASK_RUNNING)
    {
        // sometimes server sends 'use item' twice
        return;
    }

    if (slot == f->slot 
        && inventoryType == f->type
        && itemId == f->itemId)
    {
        Log("Completed");
        state_ = TASK_COMPLETED;
    }
}
