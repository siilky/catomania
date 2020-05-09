
#include "stdafx.h"

#include "boost/signal.hpp"

#include "game/trade.h"
#include "netdata/fragments_client.h"

Trade::Trade()
{
}

void Trade::init()
{
    myId_      = INVALID_ID;
    activeId1_ = INVALID_ID;
    activeId2_ = INVALID_ID;
    activeId3_ = INVALID_ID;
    playerId_  = INVALID_ID;
}

void Trade::bind(std::shared_ptr<Connection> connection)
{
    if (connection_)
    {
        unbind();
    }

    connection_ = connection;

    HANDLE_S_GI(cookies_, connection_, Trade, SelfInfo);

    HANDLE_S(cookies_, connection, Trade, TradeRequest);
    HANDLE_S(cookies_, connection, Trade, TradeStartRe);
    HANDLE_S(cookies_, connection, Trade, TradeEnd);
    HANDLE_S(cookies_, connection, Trade, TradeAddGoodsRe);
    HANDLE_S(cookies_, connection, Trade, TradeRemoveGoodsRe);
    HANDLE_S(cookies_, connection, Trade, TradeSubmitRe);
    HANDLE_S(cookies_, connection, Trade, TradeConfirmRe);
}

void Trade::unbind()
{
    if (connection_)
    {
        connection_->unbindHandlers(cookies_);
        connection_.reset();
    }

    init();
}

void Trade::accept(DWORD tradeId)
{
    if (!connection_) return;

    clientdata::FragmentTradeResponse f;
    f.tradeId = tradeId;
    f.status = clientdata::FragmentTradeResponse::TradeConfirm;

    connection_->send(&f);
}

void Trade::reject(DWORD tradeId)
{
    if (!connection_) return;

    clientdata::FragmentTradeResponse f;
    f.tradeId = tradeId;
    f.status = clientdata::FragmentTradeResponse::TradeReject;

    connection_->send(&f);
}

void Trade::discard()
{
    if (!connection_) return;
    if (activeId1_ == INVALID_ID
        || activeId2_ == INVALID_ID
        || activeId3_ == INVALID_ID
        || playerId_ == INVALID_ID
        || !connection_)
    {
        Log("failed to discard trade: invalid ID(s)");
        return;
    }

    clientdata::FragmentTradeDiscard f;
    f.tradeId1 = activeId1_;
    f.myId = myId_;
    f.tradeId3 = activeId3_;

    connection_->send(&f);
}

void Trade::submit()
{
    clientdata::FragmentTradeSubmit f;
    f.tradeId1 = activeId1_;
    f.myId = myId_;
    f.tradeId3 = activeId3_;

    connection_->send(&f);
}

void Trade::confirm()
{
    clientdata::FragmentTradeConfirm f;
    f.tradeId1 = activeId1_;
    f.myId = myId_;
    f.tradeId3 = activeId3_;

    connection_->send(&f);
}

void Trade::addItem(DWORD itemId, int inventorySlot, unsigned count, unsigned money)
{
    if (!connection_) return;
    if (activeId1_ == INVALID_ID
        || activeId2_ == INVALID_ID
        || activeId3_ == INVALID_ID
        || playerId_ == INVALID_ID
        || !connection_)
    {
        Log("failed to add item: invalid ID(s)");
        return;
    }

    clientdata::FragmentTradeAddGoods f;

    f.tradeId1      = activeId1_;
    f.tradeId3      = activeId3_;
    f.myId          = myId_;
    f.itemId        = itemId;
    f.inventorySlot = inventorySlot;
    f.itemCount     = count;
    f.money         = money;
    f.u2            = 0;
    memset(f.u1, 0, sizeof(f.u1));

    connection_->send(&f);
}

void Trade::removeItem(DWORD itemId, int inventorySlot, unsigned count, unsigned money)
{
    if (!connection_) return;
    if (activeId1_ == INVALID_ID
        || activeId2_ == INVALID_ID
        || activeId3_ == INVALID_ID
        || playerId_ == INVALID_ID
        || !connection_)
    {
        Log("failed to remove item: invalid ID(s)");
        return;
    }

    clientdata::FragmentTradeRemoveGoods f;

    f.tradeId1      = activeId1_;
    f.tradeId3      = activeId3_;
    f.myId          = myId_;
    f.itemId        = itemId;
    f.inventorySlot = inventorySlot;
    f.itemCount     = count;
    f.money         = money;
    f.u2            = 0;
    memset(f.u1, 0, sizeof(f.u1));

    connection_->send(&f);
}

//

void Trade::onSelfInfo(const serverdata::FragmentGiSelfInfo *f)
{
    if (myId_ != INVALID_ID)
    {
        assert(myId_ == f->id);
    }

    myId_ = f->id;
}

void Trade::onTradeRequest(const serverdata::FragmentTradeRequest *f)
{
    Log("Trade request: from %08X TID %08X TID2 %08X", f->playerId, f->tradeId, f->tradeId2);
    assert(f->myId == myId_);
    assert((f->tradeId & 0x80000000) != 0);

    // f->tradeId кроме подтверждения нигде не нужен
    // f->tradeId2 обычно одинаков для разных запросов, отличается только TID1

    TradeRequestEvent e(f->playerId, f->tradeId & (~0x80000000));
    sEvent(&e);
}

void Trade::onTradeStartRe(const serverdata::FragmentTradeStartRe *f)
{
    Log("Trade start: S %i PID %08X My %08X TID1 %08X TID2 %08X"
        , f->status, f->playerId, f->myId, f->tradeId1, f->tradeId2);
    assert(myId_ == f->myId);

    if (f->status != 0)
    {
        Log("Trade canceled");
        playerId_  = INVALID_ID;
        activeId1_ = INVALID_ID;
        activeId2_ = INVALID_ID;
        activeId3_ = INVALID_ID;
    }
    else
    {
        Log("Trade confirmed");
        playerId_  = f->playerId;
        activeId1_ = f->tradeId1;
        activeId2_ = f->tradeId2;
        activeId3_ = rand() * rand();
    }

    TradeStartEvent e(f->status == 0, f->playerId);
    sEvent(&e);
}

void Trade::onTradeEnd(const serverdata::FragmentTradeEnd *f)
{
    Log("Trade end: PID %08X TID %08X TID2 %08X", f->myId, f->tradeId1, f->tradeId2);

    assert(activeId1_ == f->tradeId1);
    assert(activeId2_ == f->tradeId2);
    assert(myId_ == f->myId);

    TradeEndEvent e(f->status == 1);
    sEvent(&e);

    activeId1_ = INVALID_ID;
    activeId2_ = INVALID_ID;
    activeId3_ = INVALID_ID;
    playerId_ = INVALID_ID;
}

void Trade::onTradeAddGoodsRe(const serverdata::FragmentTradeAddGoodsRe *f)
{
    Log("Trade add item: S %i PID %08X TID1 %08X TID2 %08X IID %08X count %i IS %i M %i"
        , f->status, f->sourceId, f->tradeId1, f->tradeId2
        , f->itemId, f->itemCount, f->inventorySlot, f->money);

    assert(myId_ == f->myId);
    assert(activeId1_ == f->tradeId1);
    assert(activeId2_ == f->tradeId2);

    bool isSelf = f->sourceId == myId_;
    if (!isSelf)
    {
        assert(f->sourceId == playerId_);
    }

    TradeAddItemEvent e(f->status, isSelf, f->itemId, f->attrs, f->inventorySlot, f->itemCount, f->money);
    sEvent(&e);
}

void Trade::onTradeRemoveGoodsRe(const serverdata::FragmentTradeRemoveGoodsRe *f)
{
    Log("Trade remove item: S %i PID %08X TID1 %08X TID2 %08X IID %08X count %i IS %i M %i"
        , f->status, f->sourceId, f->tradeId1, f->tradeId3
        , f->itemId, f->itemCount, f->inventorySlot, f->money);

    assert(myId_ == f->myId);
    assert(activeId1_ == f->tradeId1);
    //assert(0 == f->tradeId3);

    bool isSelf = f->sourceId == myId_;
    if (!isSelf)
    {
        assert(f->sourceId == playerId_);
    }

    TradeRemoveItemEvent e(f->status, isSelf, f->itemId, f->attrs, f->inventorySlot, f->itemCount, f->money);
    sEvent(&e);
}

void Trade::onTradeSubmitRe(const serverdata::FragmentTradeSubmitRe * f)
{
    Log("Trade submit: S %X PID %08X TID1 %08X My %08X u %08X"
        , f->status, f->playerId, f->tradeId1, f->myId, f->unk);

    assert(myId_ == f->myId);
    assert(activeId1_ == f->tradeId1);

    TradeConfirmEvent::ConfirmStage stage;

    bool isSelf = f->playerId == myId_;
    if (!isSelf)
    {
        assert(f->playerId == playerId_);
    }
    else
    {
        assert(f->playerId == myId_);
    }
    if (f->status == 0x4C || f->status == 0x4D)
    {
        stage = TradeConfirmEvent::Stage1;
    }
    else
    {
        Log("Submit code falinig: %i", f->status);
        stage = TradeConfirmEvent::None;
    }

     TradeConfirmEvent e(isSelf, stage);
     sEvent(&e);
}

void Trade::onTradeConfirmRe(const serverdata::FragmentTradeConfirmRe * f)
{
    Log("Trade confirm: S %X PID %08X TID1 %08X TID2 %08X My %08X"
        , f->status, f->playerId, f->tradeId1, f->tradeId2, f->myId);

    assert(myId_ == f->myId);
    assert(activeId1_ == f->tradeId1);

    bool isSelf = f->playerId == myId_;
    if (!isSelf)
    {
        assert(f->playerId == playerId_);
    }
    else
    {
        assert(f->playerId == myId_);
    }

    TradeConfirmEvent::ConfirmStage stage;
    if (f->status == 0x50 || f->status == 0x51 || f->status == 0x00)
    {
        stage = TradeConfirmEvent::Stage2;
    }
    else
    {
        Log("Confirm code falinig: %i", f->status);
        stage = TradeConfirmEvent::None;
    }

    TradeConfirmEvent e(isSelf, stage);
    sEvent(&e);
}

//

